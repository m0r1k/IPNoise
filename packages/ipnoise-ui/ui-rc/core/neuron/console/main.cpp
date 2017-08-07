#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <pwd.h>

#include "utils.hpp"
#include <ipnoise-common/system.h>
#include <ipnoise-common/strings.h>

#include "core/neuron/console/main.hpp"

static QTimer *s_timer = NULL;

//
// some documentation about ansi codes available in
// doc/console/vt100.txt
// doc/console/escapecodes.txt
// http://ascii-table.com/ansi-escape-sequences.php
// http://www.unixwerk.eu/unix/ansicodes.html
// http://ascii-table.com/ansi-escape-sequences-vt-100.php

NeuronConsole::NeuronConsole()
    :   SUPER_CLASS()
{
    do_init();
};

NeuronConsole::~NeuronConsole()
{
};

void NeuronConsole::do_init()
{
    initType(PROP_NEURON_TYPE("core.neuron.console"));
//    initStatus(PROP_STRING("stop"));

    // register API
//    registerApiMethod(
//        "setStatus",
//        NeuronConsole::apiSetStatus
//    );

    m_fd_master             = -1;
    m_fd_slave              = -1;
    m_shell_pid             = -1;

    // m_attrib_position    = 0;
    // m_attrib_texcoord    = 1;

    m_viewer        = NULL;
    m_local_echo    = 0;
}

int32_t NeuronConsole::do_autorun()
{
    int32_t err = 0;

    initConsole();
    h_reset();

    // init timer
    if (!s_timer){
        s_timer = new QTimer(0);
        connect(
            s_timer,    SIGNAL(timeout()),
            this,       SLOT(timerBlinkingSlot())
        );
        s_timer->start(1000);
    }

    return err;
}

void NeuronConsole::timerBlinkingSlot()
{
    m_blinking = !m_blinking;
    getViewer()->repaint();
}

void NeuronConsole::initShaders(
    const QGLContext *a_context)
{
    string code;

    m_program = new QGLShaderProgram(a_context);

    // vertex shader
    m_shader_vertex = new QGLShader(QGLShader::Vertex);
    code            = "// Vertex Shader\n"
        "#version 330\n"
        "in vec2 position;\n"
        "in vec2 texcoord;\n"
        "out vec2 vert_tex;\n"
        "void main()\n"
        "{\n"
            "vert_tex = texcoord;\n"
            "gl_Position = vec4(position*2 - 1., 0, 1);\n"
        "}\n";

    m_shader_vertex->compileSourceCode(code.c_str());
    m_program->addShader(m_shader_vertex);

    // fragment shader
    m_shader_fragment   = new QGLShader(QGLShader::Fragment);
    code                = "// Fragment Shader\n"
        "#version 330\n"
        "uniform sampler2D frame;\n"
        "in vec2 vert_tex;\n"
        "void main()\n"
        "{\n"
            "gl_FragColor = texture(frame, vert_tex);\n"
        "}\n";

    m_shader_fragment->compileSourceCode(code.c_str());
    m_program->addShader(m_shader_fragment);
    m_program->link();
}

void NeuronConsole::getAllProps(
    PropMapSptr a_props)
{
//    SAVE_PROP("status", getStatus);

    SUPER_CLASS::getAllProps(a_props);
}

int32_t NeuronConsole::parseBSON(
    mongo::BSONObj  a_obj)
{
    int32_t err = 0;
//    string  status;

    // status
//    if (a_obj.hasField("status")){
//        status = a_obj.getStringField("status");
//        initStatus(PROP_STRING(
//            status
//        ));
//    }

    err = SUPER_CLASS::parseBSON(a_obj);
    return err;
}

void NeuronConsole::add(
    const char &a_char)
{
    PropMapSptr col;

    col = getCurColInfo();
    col->add("val",     PROP_STRING(a_char));
    col->add("modes",   PROP_MAP(*(m_modes.get())));
    cursorForward(
        1,  // number of columns
        1   // create new line
    );
}

void NeuronConsole::add(
    const string &a_string)
{
    uint32_t i;

    for (i = 0; i < a_string.size(); i++){
        add(a_string.at(i));
    }
}

void NeuronConsole::processEsc(
    ConsoleParseState   &a_console_parse_state,
    string              &a_esc)
{
    ConsoleHandlersIt   it;
    int32_t             found   = 0;
    string              esc     = a_esc;

    // remove 1b (ESC)
    esc = esc.replace(
        0, 1, "ESC"
    );

    // search and execute handler
    for (it = m_esc_handlers.begin();
        it != m_esc_handlers.end();
        it++)
    {
        string format = it->first;

        QRegExp rgx(format.c_str());
        int32_t pos = 0;

        pos = rgx.indexIn(esc.c_str(), pos);
        PDEBUG(150, "MORIK test"
            " esc: '%s',"
            " pos: '%d'\n",
            esc.c_str(),
            pos
        );
        if (-1 < pos){
            it->second(this, esc, rgx);
            found = 1;
            break;
        }
    }
    if (found){
        a_console_parse_state   = CONSOLE_PARSE_STATE_REGULAR;
        a_esc                   = "";
    } else {
        // PWARN("unsupported esc seq: '%s'\n",
        //    esc.c_str()
        // );
    }
}

PropMapSptr NeuronConsole::getCurColInfo()
{
    PropVectorSptr  row;
    PropMapSptr     col;

    // get cur row
    row = dynamic_pointer_cast<PropVector>(
        m_buffer->get(
            m_cursor_row
        )
    );
    col = dynamic_pointer_cast<PropMap>(
        row->get(
            m_cursor_col
        )
    );

    return col;
}

void NeuronConsole::backspace()
{
    PropMapSptr col;

    col = getCurColInfo();
    col->add("val", "");

    // move cursor
    m_cursor_col--;
    if (0 > m_cursor_col){
        if (0 < m_cursor_row){
            m_cursor_row--;
            m_cursor_col = m_cols - 1;
        } else {
            m_cursor_col = 0;
        }
    }

    col = getCurColInfo();
    col->add("val", "");
}

PropMapSptr NeuronConsole::createEmptyModes()
{
    PropMapSptr modes = PROP_MAP();
    return modes;
}

PropMapSptr NeuronConsole::createEmptyCol()
{
    PropMapSptr col = PROP_MAP();

    col->add("val",   PROP_STRING());
    col->add("modes", PROP_MAP());

    return col;
}

PropVectorSptr NeuronConsole::createEmptyRow()
{
    PropVectorSptr  vector  = PROP_VECTOR();
    uint32_t        cur_col = 0;

    for (cur_col = 0;
        cur_col < m_cols;
        cur_col++)
    {
        PropMapSptr col = createEmptyCol();
        vector->push_back(col);
    }

    return vector;
}

void NeuronConsole::newLine()
{
    // add new line
    if (m_cursor_row < (m_rows - 1)){
        m_cursor_row++;
    } else {
        // overflow remove first line from buffer
        m_buffer->erase(
            m_buffer->begin()
        );
        // add new line
        PropVectorSptr row = createEmptyRow();
        m_buffer->push_back(row);

        //row = dynamic_pointer_cast<PropVector>(
        //    m_buffer->get(m_cursor_row)
        //);
        //(*(row.get()))[m_cursor_col] = PROP_STRING();
    }

    m_cursor_col = 0;
}

void NeuronConsole::parseInput(
    const char          &a_char,
    ConsoleParseState   &a_parse_state,
    string              &a_esc)
{
    string cur_char;
    string esc = a_esc;

    // remove 1b (ESC)
    esc = esc.replace(
        0, 1, "ESC"
    );

    // cur char
    if (0x1b == a_char){
        cur_char = "ESC";
    } else {
        cur_char = a_char;
    }

    PWARN("NeuronConsole::parseInput:\n"
        "  state: '%d'\n"
        "  parse char: '%d' (%s)\n",
        a_parse_state,
        a_char,
        cur_char.c_str()
    );

    switch (a_char){
        case 0x1b:
            if (a_esc.size()){
                PWARN("unsupported esc seq: '%s'\n",
                    esc.c_str()
                );
                processEsc(
                    a_parse_state,
                    a_esc
                );
            }
            a_parse_state   = CONSOLE_PARSE_STATE_ESC;
            a_esc           = a_char;
            break;

        case '\n':
            if (CONSOLE_PARSE_STATE_REGULAR
                == a_parse_state)
            {
                if ("B" == m_slave_character_g0){
                    m_cursor_col = 0;
                } else {
                    newLine();
                }
            } else if (CONSOLE_PARSE_STATE_ESC
                == a_parse_state)
            {
                a_esc += a_char;
                processEsc(
                    a_parse_state,
                    a_esc
                );
            }
            break;

        case '\r':
            if (CONSOLE_PARSE_STATE_REGULAR
                == a_parse_state)
            {
                m_cursor_col = 0;
            } else if (CONSOLE_PARSE_STATE_ESC
                == a_parse_state)
            {
                a_esc += a_char;
                processEsc(
                    a_parse_state,
                    a_esc
                );
            }
            break;

        case 0x07:
            // bell
            if (CONSOLE_PARSE_STATE_REGULAR
                == a_parse_state)
            {
//                if (m_cursor_row < (m_rows - 1)){
//                    m_cursor_row++;
//                }
                m_cursor_col = 0;
            } else if (CONSOLE_PARSE_STATE_ESC
                == a_parse_state)
            {
                a_esc += a_char;
                processEsc(
                    a_parse_state,
                    a_esc
                );
            }
            break;

        case 0x08:
            // backspace
            if (CONSOLE_PARSE_STATE_REGULAR
                == a_parse_state)
            {
                backspace();
            } else if (CONSOLE_PARSE_STATE_ESC
                == a_parse_state)
            {
                a_esc += a_char;
                processEsc(
                    a_parse_state,
                    a_esc
                );
            }
            break;

        default:
            if (CONSOLE_PARSE_STATE_ESC
                == a_parse_state)
            {
                a_esc += a_char;
                processEsc(
                    a_parse_state,
                    a_esc
                );
            } else {
                add(a_char);
            }
            break;
    }
}

void NeuronConsole::parseInput(
    const string        &a_string,
    ConsoleParseState   &a_parse_state,
    string              &a_esc)
{
    uint32_t i;

    for (i = 0; i < a_string.size(); i++){
        parseInput(
            a_string.at(i),
            a_parse_state,
            a_esc
        );
    }

    getViewer()->repaint();
}

void NeuronConsole::readSlot(
    int a_fd)
{
    int32_t res;
    char    buffer[65535] = { 0x00 };
    string  data;

    do {
        res = ::read(a_fd, buffer, sizeof(buffer));
    } while (0 > res && EINTR == errno);

    if (0 >= res){
        goto out;
    }

    data.assign(buffer, res);

    parseInput(
        data,
        m_slave_parse_state,
        m_slave_esc
    );

out:
    return;
}

void NeuronConsole::exceptionSlot(
    int a_fd)
{
    PWARN("UiViewer::exceptionSlot, fd: '%d'\n", a_fd);
}

// esc handlers

void NeuronConsole::setCursorPosition(
    const uint32_t   &a_row,
    const uint32_t   &a_col)
{
    PWARN("move cursor to row: '%d', col: '%d'\n",
        a_row,
        a_col
    );

//    if (    (0 <= a_col) && (a_col < m_cols)
//        &&  (0 <= a_row) && (a_row < m_rows))
//    {
        m_cursor_col = a_col;
        m_cursor_row = a_row;
//    }
}

uint32_t NeuronConsole::getCursorRow() const
{
    return m_cursor_row;
}

uint32_t NeuronConsole::getCursorCol() const
{
    return m_cursor_col;
}

void NeuronConsole::escHandlerCursorPositionReport(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    char    buffer[512] = { 0x00 };
    int32_t len         = 0;

    len = snprintf(buffer, sizeof(buffer),
        "%c[%d;%dR",
        0x1b,
        a_neuron->getCursorRow() + 1,
        a_neuron->getCursorCol() + 1
    );
    ::write(a_neuron->m_fd_master, buffer, len);
}

void NeuronConsole::escHandlerCursorPosition(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    string      params_str = a_rgx.cap(1).toStdString();
    Splitter    params(params_str, ";");

    uint32_t    row = 0;
    uint32_t    col = 0;

    PWARN("escHandlerCursorPosition:\n"
        "  params_str:       '%s'\n"
        "  params.size():    '%d'\n",
        params_str.c_str(),
        (int32_t)params.size()
    );

    if (2 == params.size()){
        row = atoi(params[0].c_str()) - 1;
        if (0 > row){
            row = 0;
        }
        col = atoi(params[1].c_str()) - 1;
        if (0 > col){
            col = 0;
        }
    }

    a_neuron->setCursorPosition(row, col);
}

void NeuronConsole::cursorUp(
    const int32_t &a_val)
{
    PWARN("cursorUp cur row: '%d', col: '%d'\n"
        "  move on '%d' symbols\n",
        m_cursor_row,
        m_cursor_col,
        a_val
    );

    m_cursor_row -= a_val;
    if (0 > m_cursor_row){
        m_cursor_row = 0;
    }
}

void NeuronConsole::escHandlerCursorUp(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    string  cols_str    = a_rgx.cap(1).toStdString();
    int32_t cols        = 1;

    if (cols_str.size()){
        cols = atoi(cols_str.c_str());
    }

    a_neuron->cursorUp(cols);
}

void NeuronConsole::cursorDown(
    const int32_t &a_val)
{
    PWARN("cursorDown cur row: '%d', col: '%d'\n"
        "  move on '%d' symbols\n",
        m_cursor_row,
        m_cursor_col,
        a_val
    );

    m_cursor_row += a_val;
    if (m_rows <= m_cursor_row){
        m_cursor_row = m_rows - 1;
    }
}

void NeuronConsole::escHandlerCursorDown(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    string  cols_str = a_rgx.cap(1).toStdString();
    int32_t cols     = 1;

    if (cols_str.size()){
        cols = atoi(cols_str.c_str());
    }

    a_neuron->cursorDown(cols);
}

void NeuronConsole::cursorForward(
    const int32_t &a_val,
    const int32_t &a_can_move_down)
{
    uint32_t cursor_col = m_cursor_col;
    PWARN("cursorForward cur row: '%d', col: '%d'\n"
        "  move on '%d' symbols,"
        "  can_move_down: '%d'\n",
        m_cursor_row,
        m_cursor_col,
        a_val,
        a_can_move_down
    );
    cursor_col += a_val;
    if (cursor_col >= m_cols){
        if (a_can_move_down){
            newLine();
            cursorForward(
                cursor_col - m_cols,
                a_can_move_down
            );
        } else {
            m_cursor_col = m_cols - 1;
        }
    } else {
        m_cursor_col = cursor_col;
    }
}

void NeuronConsole::escHandlerCursorForward(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    string  cols_str = a_rgx.cap(1).toStdString();
    int32_t cols     = 1;

    if (cols_str.size()){
        cols = atoi(cols_str.c_str());
    }

    a_neuron->cursorForward(cols);
}

void NeuronConsole::cursorBackward(
    const int32_t   &a_val,
    const int32_t   &a_can_move_up)
{
    PropMapSptr col;

    PWARN("cursorBackward cur row: '%d', col: '%d'\n"
        "  move on '%d' symbols\n"
        "  can_move_up: '%d'\n",
        m_cursor_row,
        m_cursor_col,
        a_val,
        a_can_move_up
    );

    // move cursor
    m_cursor_col -= a_val;
    if (0 > m_cursor_col){
        if (    0 < m_cursor_row
            &&  a_can_move_up)
        {
            m_cursor_row--;
            m_cursor_col = m_cols - 1;
        } else {
            m_cursor_col = 0;
        }
    }
}

void NeuronConsole::escHandlerCursorBackward(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    string  cols_str = a_rgx.cap(1).toStdString();
    int32_t cols     = 1;

    if (cols_str.size()){
        cols = atoi(cols_str.c_str());
    }

    a_neuron->cursorBackward(cols);
}

void NeuronConsole::escHandlerSaveCursorPosition(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    PWARN("TODO esc save cursor position\n");
}

void NeuronConsole::escHandlerRestoreCursorPosition(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    PWARN("TODO esc restore cursor position\n");
}

void NeuronConsole::showCursor(
    const int32_t &a_val)
{
    m_show_cursor = a_val;
}

void NeuronConsole::localEcho(
    const int32_t &a_val)
{
    m_local_echo = a_val;
    PWARN("MORIK local_echo: '%d'\n", m_local_echo);
}

void NeuronConsole::escHandlerSetResetMode(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    uint32_t    i;
    string      options = a_rgx.cap(1).toStdString();
    string      on_off  = a_rgx.cap(2).toStdString();

    Splitter res(options, ";");
    for (i = 0; i < res.size(); i++){
        string  option_str = res[i];
        int32_t option     = atoi(option_str.c_str());

        PWARN("MORIK escHandlerSetResetMode\n"
            "  option:  '%d'\n"
            "  on_off:  '%s'\n"
            "  options: '%s'\n",
            option,
            on_off.c_str(),
            options.c_str()
        );
        switch (option){
            case 12:
                if ("h" == on_off){
                    // Local echo off
                    a_neuron->localEcho(0);
                } else if ("l" == on_off){
                    // Local echo on
                    a_neuron->localEcho(1);
                } else {
                    PWARN("TODO process value: '%s'"
                        " for esc: '%s'\n",
                        on_off.c_str(),
                        a_esc.c_str()
                    );
                }

                break;

            case 25:
                // show hide cursor
                if ("h" == on_off){
                    a_neuron->showCursor(0);
                } else if ("l" == on_off){
                    a_neuron->showCursor(1);
                } else {
                    PWARN("TODO process value: '%s'"
                        " for esc: '%s'\n",
                        on_off.c_str(),
                        a_esc.c_str()
                    );
                }
                break;

            default:
                PWARN("TODO process esc: '%s'\n",
                    a_esc.c_str()
                );
                break;
        }
    }
}

void NeuronConsole::escHandlerEraseDisplay(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    uint32_t    i;
    string      mode_str = a_rgx.cap(1).toStdString();

    PWARN("MORIK mode_str: '%s'\n",
        mode_str.c_str()
    );

    if (mode_str.size()){
        Splitter res(mode_str, ";");
        for (i = 0; i < res.size(); i++){
            string  mode_str = res[i];
            int32_t mode     = atoi(mode_str.c_str());

            PWARN("MORIK reset mode: '%d'\n", mode);

            switch (mode){
                case 2:
                    a_neuron->clear();
                    break;

                case 3:
                    a_neuron->h_reset();
                    break;

                default:
                    PWARN("TODO esc erase display handler"
                        " for mode: '%d'\n",
                        mode
                    );
                    break;
            };
        }
    }
}

void NeuronConsole::eraseLineFromCursor()
{
    uint32_t        col = 0;
    PropVectorSptr  row;

    row = dynamic_pointer_cast<PropVector>(
        m_buffer->get(m_cursor_row)
    );

    for (col = m_cursor_col;
        col < m_cols;
        col++)
    {
        (*(row.get()))[col] = createEmptyCol();
    }
}

void NeuronConsole::escHandlerEraseLineFromCursor(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    a_neuron->eraseLineFromCursor();
}

void NeuronConsole::clearModes()
{
    m_modes = createEmptyModes();
}

void NeuronConsole::addMode(
    const int32_t &a_val)
{
    PWARN("add graphics mode: '%d'\n",
        a_val
    );
    m_modes->add(
        PROP_INT(a_val),
        PropSptr()
    );
}

void NeuronConsole::escHandlerSetGraphicsMode(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    uint32_t        i;
    string          mode_str = a_rgx.cap(1).toStdString();
    PropMapSptr     col      = a_neuron->getCurColInfo();

   a_neuron->clearModes();

    if (mode_str.size()){
        Splitter res(mode_str, ";");
        for (i = 0; i < res.size(); i++){
            string  mode_str = res[i];
            int32_t mode     = atoi(mode_str.c_str());
//            if (!mode){
//                a_neuron->clearModes();
//            }
            a_neuron->addMode(mode);
        }
 //   } else {
 //       a_neuron->clearModes();
    }
}

void NeuronConsole::escHandlerSetMode(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    QString mode_str = a_rgx.cap(1);

    PWARN("TODO set mode: '%s'\n",
        mode_str.toStdString().c_str()
    );
}

/*
void NeuronConsole::escHandlerResetMode(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    QString mode_str = a_rgx.cap(1);

    PWARN("TODO reset mode: '%s'\n",
        mode_str.toStdString().c_str()
    );
}
*/

void NeuronConsole::s_reset()
{
    PWARN("MORIK RESET\n");

    m_old_keyboard_event_code   = 0;
    m_old_keyboard_event_text   = "";

    // console
    m_cols                  = atoi(CONSOLE_COLS);
    m_rows                  = atoi(CONSOLE_ROWS);
    m_font_size             = 10;
    m_font                  = QFont(
        "Liberation Mono",
        m_font_size
    );

    m_step_x                = m_font_size * 0.9f;
    m_step_y                = m_font_size * 2.0f;
    m_width                 = m_step_x * (m_cols + 1);
    m_height                = m_step_y * (m_rows + 1);
    m_cursor_row            = 0;
    m_cursor_col            = 0;
    m_master_parse_state    = CONSOLE_PARSE_STATE_REGULAR;
    m_master_esc            = "";
    m_slave_parse_state     = CONSOLE_PARSE_STATE_REGULAR;
    m_slave_esc             = "";

    m_blinking              = 0;
    m_show_cursor           = 1;

    // IO characters
    m_slave_character_g0   = "";
    m_master_character_g1  = "";

    // clear modes
    clearModes();

    // init/clear console buffer
    clear();

    if (0 && m_local_echo){
        // local echo off
        int32_t len         = 0;
        char    buffer[512] = { 0x00 };
        len = snprintf(buffer, sizeof(buffer),
            "%c[12l",
            0x1b
        );
        ::write(m_fd_master, buffer, len);
        m_local_echo = 0;
    }
}

void NeuronConsole::h_reset()
{
    PWARN("MORIK HARD RESET\n");

    s_reset();
}

void NeuronConsole::escHandlerSetKeyboardStrings(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    QString mode_str = a_rgx.cap(1);

    if ("!" == mode_str){
        a_neuron->s_reset();
    } else {
        PWARN("TODO set keyboard strings: '%s'\n",
            mode_str.toStdString().c_str()
        );
    }
}

void NeuronConsole::escHandlerResetHard(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    a_neuron->h_reset();
}

void NeuronConsole::escHandlerShowPrompt(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    PWARN("TODO esc show prompt handler\n");
}

void NeuronConsole::tabAdd(
    const int32_t &a_val)
{
    int32_t i;

    for (i = 0; i < a_val; i++){
        add("    ");
    }
}

void NeuronConsole::escHandlerTabAdd(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    PWARN("TODO esc horizontal tab set\n");
    a_neuron->tabAdd();
}

void NeuronConsole::escHandlerTabClear(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    QString mode_str = a_rgx.cap(1);
    int32_t mode     = mode_str.toInt();

    switch (mode){
        case 3:
            // Clear all tabs
            break;
        default:
            PWARN("TODO esc tab clear, mode: '%d'\n", mode);
            break;
    }
}

void NeuronConsole::setSlaveCharacter(
    const string &a_val)
{
    m_slave_character_g0 = a_val;
}

void NeuronConsole::setMasterCharacter(
    const string &a_val)
{
    m_master_character_g1 = a_val;
}

void NeuronConsole::escHandlerCharacterSet(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    string  go_g1 = a_rgx.cap(1).toStdString();
    string  type  = a_rgx.cap(2).toStdString();

    PWARN("TODO esc character set,"
        " g0_g1: '%s',"
        " character type: '%s'\n",
        go_g1.c_str(),
        type.c_str()
    );

    if ("(" == go_g1){
        a_neuron->setSlaveCharacter(type);
    } else if (")" == go_g1){
        a_neuron->setMasterCharacter(type);
    } else {
        PWARN("unsupported g0_g1 type: '%s',"
            " esc: '%s'\n",
            go_g1.c_str(),
            a_esc.c_str()
        );
    }
}

void NeuronConsole::escHandlerKeypadMode(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx)
{
    string  mode = a_rgx.cap(1).toStdString();

    PWARN("TODO esc keypad mode: '%s'\n",
        mode.c_str()
    );
}

void NeuronConsole::clear()
{
    uint32_t row;

    PropVectorSptr empty_buffer = PROP_VECTOR();

    for (row = 0; row < m_rows; row++){
        PropVectorSptr row_vector = createEmptyRow();
        empty_buffer->push_back(row_vector);
    }

    m_buffer        = empty_buffer;
    m_cursor_row    = 0;
    m_cursor_col    = 0;
}

int32_t NeuronConsole::initConsole()
{
    int32_t res, err = -1;

    m_fd_master = -1;
    m_fd_slave  = -1;

    // register handlers
    REG_HANDLER(
        "ESC\\[([\\d;]*)[Hf]",
        NeuronConsole::escHandlerCursorPosition
    );
    REG_HANDLER(
        "ESC\\[6n",
        NeuronConsole::escHandlerCursorPositionReport
    );
    REG_HANDLER(
        "ESC\\[(\\d*)A",
        NeuronConsole::escHandlerCursorUp
    );
    REG_HANDLER(
        "ESC\\[(\\d*)B",
        NeuronConsole::escHandlerCursorDown
    );
    REG_HANDLER(
        "ESC\\[(\\d*)C",
        NeuronConsole::escHandlerCursorForward
    );
    REG_HANDLER(
        "ESC\\[(\\d*)D",
        NeuronConsole::escHandlerCursorBackward
    );

    REG_HANDLER(
        "ESC\\[s",
        NeuronConsole::escHandlerSaveCursorPosition
    );
    REG_HANDLER(
        "ESC\\[u",
        NeuronConsole::escHandlerRestoreCursorPosition
    );

    REG_HANDLER(
        "ESC\\[([^J]+)J",
        NeuronConsole::escHandlerEraseDisplay
    );

    REG_HANDLER(
        "ESC\\[\\d*K",
        NeuronConsole::escHandlerEraseLineFromCursor
    );

    REG_HANDLER(
        "ESC\\[\\?*([\\d;]+)([hl])",
        NeuronConsole::escHandlerSetResetMode
    );

    REG_HANDLER(
        "ESC\\[([\\d;]*)m",
        NeuronConsole::escHandlerSetGraphicsMode
    );
    REG_HANDLER(
        "ESC\\[=(\\d*)h",
        NeuronConsole::escHandlerSetMode
    );
//    REG_HANDLER(
//        "ESC\\[=(\\d+)",
//        NeuronConsole::escHandlerResetMode
//    );

    REG_HANDLER(
        "ESC\\[([^p]*)p",
        NeuronConsole::escHandlerSetKeyboardStrings
    );

    REG_HANDLER(
        "ESCc",
        NeuronConsole::escHandlerResetHard
    );

    REG_HANDLER(
        "ESC\\]0;",
        NeuronConsole::escHandlerShowPrompt
    );

    REG_HANDLER(
        "ESCH",
        NeuronConsole::escHandlerTabAdd
    );

    REG_HANDLER(
        "ESC\\[(\\d*)g",
        NeuronConsole::escHandlerTabClear
    );

    REG_HANDLER(
        "ESC([\\(\\)])([\\d\\w])",
        NeuronConsole::escHandlerCharacterSet
    );

    REG_HANDLER(
        "ESC([>=]$)",
        NeuronConsole::escHandlerKeypadMode
    );

    m_fd_master = posix_openpt(O_RDWR);
    if (0 > m_fd_master){
        PERROR("cannot open posix pseudo terminal()\n");
        goto fail;
    }

    res = grantpt(m_fd_master);
    if (res){
        PERROR("grantpt() failed\n");
        err = res;
        goto fail;
    }

    res = unlockpt(m_fd_master);
    if (res){
        PERROR("unlockpt() failed\n");
        err = res;
        goto fail;
    }

    // Open the slave side ot the PTY
    m_fd_slave = ::open(
        ptsname(m_fd_master),
        O_RDWR
    );

    // Create the child process
    m_shell_pid = fork();
    if (m_shell_pid){
        // parent

        // Close the slave side of the PTY
        ::close(m_fd_slave);

        // read
        m_notifier_r = QSocketNotifierSptr(
            new QSocketNotifier(
                m_fd_master,
                QSocketNotifier::Read
            )
        );
        CHECKED_CONNECT(
            m_notifier_r.get(), SIGNAL(activated(int)),
            this,               SLOT(readSlot(int))
        );

        // exception
        m_notifier_x = QSocketNotifierSptr(
            new QSocketNotifier(
                m_fd_master,
                QSocketNotifier::Exception
            )
        );
        CHECKED_CONNECT(
            m_notifier_x.get(), SIGNAL(activated(int)),
            this,               SLOT(exceptionSlot(int))
        );

/*
        while (1){
            // Wait for data from standard input and master side of PTY
            FD_ZERO(&fd_in);
            FD_SET(0,   &fd_in);
            FD_SET(fdm, &fd_in);

            rc = select(fd_master + 1, &fd_in, NULL, NULL, NULL);
            switch(rc){
                case -1:
                    fprintf(stderr, "Error %d on select()\n", errno);
                    exit(1);

                default:
                {
                    // If data on standard input
                    if (FD_ISSET(0, &fd_in)){
                      rc = read(0, input, sizeof(input));
                      if (rc > 0){
                            // Send data on the master side of PTY
                            write(fd_master, input, rc);
                        } else {
                            if (rc < 0){
                                fprintf(stderr, "Error %d on read standard input\n", errno);
                                exit(1);
                            }
                        }
                    }
                }
                break;
            }

            // If data on master side of PTY
            if (FD_ISSET(fd_master, &fd_in)){
                rc = read(fd_master, input, sizeof(input));
                if (rc > 0){
                    //ipnoise_hexdump(input, rc);
                    // Send data on standard output
                    write(1, input, rc);
                } else {
                    if (rc < 0){
                        fprintf(stderr, "Error %d on read master PTY\n", errno);
                        exit(1);
                    }
                }
            }
        }
*/
    } else {
        // child

        struct termios slave_orig_term_settings;    // Saved terminal settings
        struct termios new_term_settings;           // Current terminal settings

        // Close the master side of the PTY
        ::close(m_fd_master);

        // Save the defaults parameters of the slave side of the PTY
        res = tcgetattr(
            m_fd_slave,
            &slave_orig_term_settings
        );

        // Set RAW mode on slave side of PTY
        new_term_settings = slave_orig_term_settings;
        cfmakeraw(&new_term_settings);
        tcsetattr(
            m_fd_slave,
            TCSANOW,
            &new_term_settings
        );

        // The slave side of the PTY becomes the standard input and outputs of the child process
        ::close(0); // Close standard input (current terminal)
        ::close(1); // Close standard output (current terminal)
        ::close(2); // Close standard error (current terminal)

        ::dup(m_fd_slave); // PTY becomes standard input (0)
        ::dup(m_fd_slave); // PTY becomes standard output (1)
        ::dup(m_fd_slave); // PTY becomes standard error (2)

        // Now the original file descriptor is useless
        ::close(m_fd_slave);

        // Make the current process a new session leader
        setsid();

        // As the child is a session leader, set the controlling terminal to be the slave side of the PTY
        // (Mandatory for programs like the shell to make them manage correctly their outputs)
        ioctl(0, TIOCSCTTY, 1);

        // Execution of the program
        do {
            int             i;
            char **         child_args    = NULL;
            char **         child_envs    = NULL;
            struct passwd * pw            = getpwuid(getuid());
            const char    * homedir       = pw->pw_dir;
            char            buffer[65535] = { 0x00 };

            // template for args
            int32_t     argc    = 1;
            const char  *argv[] = {
                "/bin/bash",
                NULL
            };

            // prepare HOME env
            snprintf(buffer, sizeof(buffer),
                "HOME=%s",
                homedir
            );

            // template for envs
            int32_t     envc    = 4;
            const char  *env[]  = {
                "LINES=" CONSOLE_ROWS, // TODO remove hardcode
                "COLUMNS=" CONSOLE_COLS,
                "TERM=xterm",
                buffer,
                NULL
            };

            // prepare cmd line
            child_args = (char **)malloc(sizeof(*argv));
            for (i = 0; i < argc; i++){
                child_args[i] = strdup(argv[i]);
            }
            child_args[i] = NULL;

            // prepare env
            child_envs = (char **)malloc(sizeof(*env));
            for (i = 0; i < envc; i++){
                child_envs[i] = strdup(env[i]);
            }
            child_envs[i] = NULL;

            res = execvpe(
                child_args[0],
                child_args,
                child_envs
            );
            sleep (1);
        } while (1);
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 <= err){
        err = -1;
    }
    goto out;
}

void NeuronConsole::keyPressEvent(
    QKeyEvent *a_ev)
{
    int32_t res;
    int32_t code = a_ev->key();
    string  text = a_ev->text().toStdString();

    Qt::KeyboardModifiers modifiers = a_ev->modifiers();

    PDEBUG(20, "QKeyEvent: code: 0x'%x',"
        " text: '%s',"
        " m_local_echo: '%d'\n",
        code,
        text.c_str(),
        m_local_echo
    );

    PWARN("NeuronConsole::keyPressEvent code: 0x'%x',"
        " text: '%s',"
        " m_local_echo: '%d'\n",
        code,
        text.c_str(),
        m_local_echo
    );

    {
        res = -1;
        int32_t write_to_buffer = 0;
        int32_t write_to_pty    = 0;
        string  send_signal     = "";

        if (   (Qt::Key_Enter   == code
            ||  Qt::Key_Return  == code))
        {
            text                    = '\n';
            write_to_buffer         = 1;
            write_to_pty            = 1;
        } else if (Qt::Key_Up == code){
            // up
            text += '\x1b';
            text += '[';
            text += '1';
            text += 'A';
            write_to_buffer         = 1;
            write_to_pty            = 1;
        } else if (Qt::Key_Down == code){
            // down
            text += '\x1b';
            text += '[';
            text += '1';
            text += 'B';
            write_to_buffer         = 1;
            write_to_pty            = 1;
        } else if (Qt::Key_Right == code){
            // forward
            text += '\x1b';
            text += '[';
            text += '1';
            text += 'C';
            write_to_buffer         = 1;
            write_to_pty            = 1;
        } else if (Qt::Key_Left == code){
            // backward
            text += '\x1b';
            text += '[';
            text += '1';
            text += 'D';
            write_to_buffer         = 1;
            write_to_pty            = 1;
        } else if ( 0x43 == code
            &&      Qt::ControlModifier == modifiers)
        {
            // ctrl+c (end of text)
            text            = '\x03';
            write_to_pty    = 1;

            if (0x43 == m_old_keyboard_event_code){
                send_signal = "SIGKILL";
            } else {
                send_signal = "SIGINT";
            }
        } else if ( "a" == text
            &&      Qt::ControlModifier == modifiers)
        {
            // ctrl+a
            text            = "\n";
            write_to_pty    = 1;
        } else if (Qt::Key_Backspace == code){
            // backspace
            text                    = '\x08';
            write_to_pty            = 1;
            write_to_buffer         = 1;
        } else if (Qt::Key_Tab == code){
            // tab
            text                    = '\x09';
            write_to_pty            = 1;
        } else if (text.size()){
            write_to_buffer         = 1;
            write_to_pty            = 1;
        }

        if (    write_to_buffer
            &&  !m_local_echo)
        {
            // process donot have local echo,
            // so emulate it, add char to buffer
            parseInput(
                text,
                m_master_parse_state,
                m_master_esc
            );
        }
        if (write_to_pty){
            do {
                res = ::write(
                    m_fd_master,
                    text.c_str(),
                    text.size()
                );
            } while (0 > res && EAGAIN == errno);

            if (0 < res){
                fsync(m_fd_master);
            }

            PWARN("wrote '%s' to pty, res: '%d'\n",
                text.c_str(),
                res
            );
        }

        if (send_signal.size()){
            string      my_stdout;
            string      my_stderr;
            char        buffer[512] = { 0x00 };
            string      signal      = "-"+send_signal;

            snprintf(buffer, sizeof(buffer),
                "%d",
                m_shell_pid
            );

            const char  *argv[] = {
                "/usr/bin/pkill",
                signal.c_str(),
                "-P",
                buffer,
                NULL
            };
            my_system(argv, my_stdout, my_stderr);
            if (my_stderr.size()){
                PERROR("pkill failed (%s)\n",
                    my_stderr.c_str()
                );
            }
        }
    }

    m_old_keyboard_event_code = code;
    m_old_keyboard_event_text = text;
    m_old_keyboard_modifiers  = modifiers;

    return;
}

void NeuronConsole::displayText(
    const Vec       &a_pos,
    const string    &a_text,
    bool            a_render)
{
    if (!m_text_texture){
        glGenTextures(1, &m_text_texture);
    }

    glActiveTexture(GL_TEXTURE0);
//    gltPrintMultiError("glActiveTexture");

    glBindTexture(GL_TEXTURE_2D, m_text_texture);
//    gltPrintMultiError("glBindTexture");

    int32_t tex_width, tex_height;

    glGetTexLevelParameteriv(
        GL_TEXTURE_2D,
        0,
        GL_TEXTURE_WIDTH,
        &tex_width
    );
//    gltPrintMultiError(
//        "glGetTexLevelParameteriv GL_TEXTURE_WIDTH"
//    );
    glGetTexLevelParameteriv(
        GL_TEXTURE_2D,
        0,
        GL_TEXTURE_HEIGHT,
        &tex_height
    );
//    gltPrintMultiError(
//        "glGetTexLevelParameter GL_TEXTURE_HEIGHT"
//    );

    if (    tex_width  != m_width
        ||  tex_height != m_height)
    {
        a_render    = true;
        tex_width   = m_width;
        tex_height  = m_height;
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB8,
            tex_width,
            tex_height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            NULL
        );
//        gltPrintMultiError("glTexImage2D");
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR
        );
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_LINEAR
        );
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE
        );
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE
        );
    }

    if (a_render){
        QImage textimg(
            tex_width,
            tex_height,
            QImage::Format_RGB888
        );
        {
            QPainter painter(&textimg);
            painter.fillRect(
                0, 0,
                tex_width, tex_height,
                QColor(0, 0, 0)
            );
            painter.setBrush(QColor(255, 255, 255));
            painter.setPen(QColor(255, 255, 255));
            painter.setFont(QFont("Sans", 10));
            painter.drawText(5, 20, a_text.c_str());
        }

        glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
        glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
        glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
        glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
        glPixelStorei(GL_UNPACK_SKIP_IMAGES,  0);
        glPixelStorei(GL_UNPACK_ALIGNMENT,    4);

        glTexSubImage2D(
            GL_TEXTURE_2D,          // target
            0,                      // level
            0,                      // x-offset
            0,                      // y-offset
            tex_width,              // width
            tex_height,             // height
            GL_RGB,                 // format
            GL_UNSIGNED_BYTE,       // type
            textimg.constBits()     // data
        );
    }

    Frame             m_frame;
    m_frame.setPosition(a_pos);

    glPushMatrix();
    glMultMatrixd(m_frame.matrix());

//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

    //glViewport(0, 0, tex_width, tex_height);

//    glDisable(GL_DEPTH_TEST);
//    glDepthMask(0);

    if (1){
//            glColorMask(1,1,1,1);
//            glDrawArrays(
//                GL_QUADS,   // mode
//                0,          // first
//                4           // count
//            );

        glEnable(GL_TEXTURE_2D);

        // Display the quad
        //glNormal3f(0.0, 0.0, 1.0);
        glColor3f(1.0f, 1.0f, 1.0f);
        //glColorMask(1,1,1,1);

        // Nice texture coordinate interpolation
        //glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

        glBegin(GL_QUADS);

        glTexCoord2f(0.0f,   0.0f);
        glVertex2f(-1.0f,    0.0f);

        glTexCoord2f(0.0f,   1.0f);
        glVertex2f(-1.0f,    0.4f);

        glTexCoord2f(1.0f,  1.0f);
        glVertex2f(1.0f,    0.4f);

        glTexCoord2f(1.0f,  0.0f);
        glVertex2f(1.0f,    0.0f);

        glEnd();

    }


//    glDisable(GL_BLEND);

    glPopMatrix();

    // glDisableVertexAttribArray(frame2d.attrib_position);
    // glDisableVertexAttribArray(frame2d.attrib_texcoord);

    //m_program->disableAttributeArray(
    //    m_attrib_position
    //);
    //m_program->disableAttributeArray(
    //    m_attrib_texcoord
    //);

    // glUseProgram(0);

    //m_program->release();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void NeuronConsole::displayConsole(
    const Vec       &a_pos,
//    const int32_t   &a_text_width,
//    const int32_t   &a_text_height,
    bool            a_render)
{
    // gen texture
    if (!m_text_texture){
        glGenTextures(1, &m_text_texture);
    }

    glActiveTexture(GL_TEXTURE0);
//    gltPrintMultiError("glActiveTexture");

    glBindTexture(GL_TEXTURE_2D, m_text_texture);
//    gltPrintMultiError("glBindTexture");

    int tex_width, tex_height;
    glGetTexLevelParameteriv(
        GL_TEXTURE_2D,
        0,
        GL_TEXTURE_WIDTH,
        &tex_width
    );
//    gltPrintMultiError(
//        "glGetTexLevelParameteriv GL_TEXTURE_WIDTH"
//    );
    glGetTexLevelParameteriv(
        GL_TEXTURE_2D,
        0,
        GL_TEXTURE_HEIGHT,
        &tex_height
    );
//    gltPrintMultiError(
//        "glGetTexLevelParameter GL_TEXTURE_HEIGHT"
//    );

    if (    tex_width  != m_width
        ||  tex_height != m_height)
    {
        a_render    = true;
        tex_width   = m_width;
        tex_height  = m_height;
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGB8,
            tex_width,
            tex_height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            NULL
        );
//        gltPrintMultiError("glTexImage2D");
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MIN_FILTER,
            GL_LINEAR
        );
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_MAG_FILTER,
            GL_LINEAR
        );
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_S,
            GL_CLAMP_TO_EDGE
        );
        glTexParameteri(
            GL_TEXTURE_2D,
            GL_TEXTURE_WRAP_T,
            GL_CLAMP_TO_EDGE
        );
    }

    if (a_render){
        QImage textimg(
            tex_width,
            tex_height,
            QImage::Format_RGB888
        );
        QPainter painter(&textimg);

        painter.fillRect(
            0, 0,
            tex_width, tex_height,
            QColor(0, 0, 0)
        );

        uint32_t    cur_row, cur_col;
//        double      x_step = tex_width  / (m_cols + 1);
//        double      y_step = tex_height / (m_rows + 1);

        for (cur_row = 0;
            cur_row < m_rows;
            cur_row++)
        {
            PropVectorSptr row;
            row = dynamic_pointer_cast<PropVector>(
                m_buffer->get(cur_row)
            );
            for (cur_col = 0;
                cur_col < m_cols;
                cur_col++)
            {
                PropMapSptr     col;
                PropMapSptr     modes;
                PropMapIt       modes_it;
                PropStringSptr  prop_val;
                string          val;

                QColor          pen_color;
                QColor          brush_color;
                QColor          background_color;
                Qt::BGMode      background_mode;

                QColor          d_pen_color;
                QColor          d_brush_color;
                QColor          d_background_color;
                Qt::BGMode      d_background_mode;

                // default colors
                d_pen_color        = QColor(255, 255, 255);
                d_brush_color      = QColor(255, 255, 255);
                d_background_color = QColor(0, 0, 0);
                d_background_mode  = Qt::TransparentMode;

                // get col
                col = dynamic_pointer_cast<PropMap>(
                    row->get(cur_col)
                );

                // copy modes
                modes = PROP_MAP(
                    *dynamic_pointer_cast<PropMap>(
                        col->get("modes")
                    ).get()
                );

                // get val
                prop_val = dynamic_pointer_cast<PropString>(
                    col->get("val")
                );
                if (prop_val){
                    val = prop_val->toString();
                }

                // setup defaults
                pen_color        = d_pen_color;
                brush_color      = d_brush_color;
                background_color = d_background_color;
                background_mode  = d_background_mode;

                if (    m_show_cursor
                    &&  cur_row == m_cursor_row
                    &&  cur_col == m_cursor_col)
                {
                    if (val.size()){
                        // switch reverse video on
                        modes->add(
                            PROP_INT(7),
                            PropSptr()
                        );
                    } else {
                        val = "_";
                    }
                }

                for (modes_it = modes->begin();
                    modes_it != modes->end();
                    modes_it++)
                {
                    PropIntSptr prop_mode;
                    int32_t     mode = 0;

                    uint8_t     red   = 0;
                    uint8_t     green = 0;
                    uint8_t     blue  = 0;

                    prop_mode = dynamic_pointer_cast<PropInt>(
                        modes_it->first
                    );
                    mode = prop_mode->getVal();
                    switch (mode){
                        // *** Text attributes ***

                        case 0:
                            // All attributes off
                            m_font.setBold(0);
                            pen_color        = d_pen_color;
                            brush_color      = d_brush_color;
                            background_color = d_background_color;
                            background_mode  = d_background_mode;
                            break;

                        case 1:
                            // Bold on
                            m_font.setBold(1);
                            break;

                        case 4:
                            // Underscore
                            // (on monochrome display
                            //  adapter only)
                            break;

                        case 5:
                            // Blink on
                            if (!m_blinking){
                                // not ready for blink
                                break;
                            }
                            // no 'break' here
                            // because next mode is 7 :)

                        case 7:
                            // Reverse video on
                            // pen
                            red   = ~char(pen_color.red());
                            green = ~char(pen_color.green());
                            blue  = ~char(pen_color.blue());

                            pen_color = QColor(
                                red, green, blue
                            );

                            // brush
                            red   = ~char(brush_color.red());
                            green = ~char(brush_color.green());
                            blue  = ~char(brush_color.blue());
                            brush_color = QColor(
                                red, green, blue
                            );

                            // background
                            red   = ~char(background_color.red());
                            green = ~char(background_color.green());
                            blue  = ~char(background_color.blue());
                            background_color = QColor(
                                red, green, blue
                            );

                            // background mode
                            if (Qt::TransparentMode == background_mode){
                                background_mode = Qt::OpaqueMode;
                            } else {
                                background_mode = Qt::TransparentMode;
                            }

                            break;

                        case 8:
                            // Concealed on
                            break;

                        // *** Foreground colors ***

                        case 30:
                            // Black
                            pen_color = QColor(
                                0, 0, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 31:
                            // Red
                            pen_color = QColor(
                                255, 0, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 32:
                            // Green
                            pen_color = QColor(
                                0, 255, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 33:
                            // Yellow
                            pen_color = QColor(
                                255, 255, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 34:
                            // Blue
                            pen_color = QColor(
                                0, 0, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 35:
                            // Magenta
                            pen_color = QColor(
                                255, 0, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 36:
                            // Cyan
                            pen_color = QColor(
                                0, 255, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 37:
                            // White
                            pen_color = QColor(
                                255, 255, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 39:
                            // Default
                            pen_color = d_pen_color;
                            break;


                        // *** Foreground high colors ***

                        case 90:
                            // Black
                            pen_color = QColor(
                                0, 0, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 91:
                            // Red
                            pen_color = QColor(
                                255, 0, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 92:
                            // Green
                            pen_color = QColor(
                                0, 255, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 93:
                            // Yellow
                            pen_color = QColor(
                                255, 255, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 94:
                            // Blue
                            pen_color = QColor(
                                0, 0, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 95:
                            // Magenta
                            pen_color = QColor(
                                255, 0, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 96:
                            // Cyan
                            pen_color = QColor(
                                0, 255, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 97:
                            // White
                            pen_color = QColor(
                                255, 255, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 99:
                            // Default
                            pen_color = d_pen_color;
                            break;


                        // *** Background colors ***
                        case 40:
                            // Black
                            background_color = QColor(
                                0, 0, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 41:
                            // Red
                            background_color = QColor(
                                255, 0, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 42:
                            // Green
                            background_color = QColor(
                                0, 255, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 43:
                            // Yellow
                            background_color = QColor(
                                255, 255, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 44:
                            // Blue
                            background_color = QColor(
                                0, 0, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 45:
                            // Magenta
                            background_color = QColor(
                                255, 0, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 46:
                            // Cyan
                            background_color = QColor(
                                0, 255, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 47:
                            //  White
                            background_color = QColor(
                                255, 255, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 49:
                            // Default
                            background_color = d_background_color;
                            background_mode = Qt::OpaqueMode;
                            break;


                        // *** Background high colors ***
                        case 100:
                            // Black
                            background_color = QColor(
                                0, 0, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 101:
                            // Red
                            background_color = QColor(
                                255, 0, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 102:
                            // Green
                            background_color = QColor(
                                0, 255, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 103:
                            // Yellow
                            background_color = QColor(
                                255, 255, 0
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 104:
                            // Blue
                            background_color = QColor(
                                0, 0, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 105:
                            // Magenta
                            background_color = QColor(
                                255, 0, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 106:
                            // Cyan
                            background_color = QColor(
                                0, 255, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 107:
                            //  White
                            background_color = QColor(
                                255, 255, 255
                            );
                            background_mode = Qt::OpaqueMode;
                            break;

                        case 109:
                            // Default
                            background_color = d_background_color;
                            background_mode = Qt::OpaqueMode;
                            break;


                        default:
                            PWARN("TODO add mode: '%d'\n",
                                mode
                            );
                            background_mode = Qt::OpaqueMode;
                            break;
                    }
                }

                brush_color = pen_color; // TODO remove me

/*
                if (    cur_row == m_cursor_row
                    &&  cur_col == m_cursor_col)
                {
                    if (!val.size()){
                        val = "_";
                        pen_color   = QColor(0, 0, 0);
                        brush_color = QColor(255, 255, 255);
                        background_color = QColor(255, 255, 255);
                        background_mode  = Qt::OpaqueMode;
                    } else {
                        pen_color   = QColor(0, 0, 0);
                        brush_color = QColor(255, 255, 255);
                        background_color = QColor(255, 255, 255);
                        background_mode = Qt::OpaqueMode;
                    }
                }
*/

                // setup paint instruments
                painter.setPen(pen_color);
                painter.setBrush(brush_color);
                painter.setBackground(background_color);
                painter.setBackgroundMode(background_mode);
                painter.setFont(m_font);

                if (val.size()){
                    painter.drawText(
                        m_step_x + m_step_x * cur_col,
                        m_step_y + m_step_y * cur_row,
                        val.c_str()
                    );
                }
            }
        }

        glPixelStorei(GL_UNPACK_SWAP_BYTES,   GL_FALSE);
        glPixelStorei(GL_UNPACK_LSB_FIRST,    GL_FALSE);
        glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
        glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
        glPixelStorei(GL_UNPACK_SKIP_ROWS,    0);
        glPixelStorei(GL_UNPACK_SKIP_PIXELS,  0);
        glPixelStorei(GL_UNPACK_SKIP_IMAGES,  0);
        glPixelStorei(GL_UNPACK_ALIGNMENT,    4);

        glTexSubImage2D(
            GL_TEXTURE_2D,          // target
            0,                      // level
            0,                      // x-offset
            0,                      // y-offset
            tex_width,              // width
            tex_height,             // height
            GL_RGB,                 // format
            GL_UNSIGNED_BYTE,       // type
            textimg.constBits()     // data
        );
    }

/*
    static GLfloat const pos[] = {
        0,      0.5f,
        0.5f,   0.5f,
        0.5f,   0,
        0,      0
    };
    static GLfloat const tex[] = {
        0,      0,
        0.5f,   0,
        0.5f,   0.5f,
        0,      0.5f
    };
*/

    //setShaders();
//    m_program->bind();

//    glEnableVertexAttribArray(m_attrib_position);
//    glEnableVertexAttribArray(m_attrib_texcoord);

/*
    // vertex
    m_program->setAttributeArray(
        m_attrib_position,
        pos,
        2
    );
    m_program->enableAttributeArray(
        m_attrib_position
    );

    // textures
    m_program->setAttributeArray(
        m_attrib_texcoord,
        tex,
        2
    );
    m_program->enableAttributeArray(
        m_attrib_texcoord
    );
*/

/*
    glVertexAttribPointer(
        m_attrib_position,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        pos
    );

//    gltPrintMultiError("glVertexAttribPointer(attrib_position, ...)");
    glVertexAttribPointer(
        m_attrib_texcoord,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        tex
    );
//    gltPrintMultiError("glVertexAttribPointer(attrib_texcoord, ...)");
*/

//    m_program->setUniformValue(0, 0);
//    gltPrintMultiError("glUniform1i(frame2d.uniform_sampler_frame)");

    Frame             m_frame;
    m_frame.setPosition(a_pos);

    glPushMatrix();
    glMultMatrixd(m_frame.matrix());

//    glEnable(GL_BLEND);
//    glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

    //glViewport(0, 0, tex_width, tex_height);

//    glDisable(GL_DEPTH_TEST);
//    glDepthMask(0);

    if (1){
//            glColorMask(1,1,1,1);
//            glDrawArrays(
//                GL_QUADS,   // mode
//                0,          // first
//                4           // count
//            );

        glEnable(GL_TEXTURE_2D);

        // Display the quad
        //glNormal3f(0.0, 0.0, 1.0);
        glColor3f(1.0f, 1.0f, 1.0f);
        //glColorMask(1,1,1,1);

        // Nice texture coordinate interpolation
        //glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

        glBegin(GL_QUADS);

        glTexCoord2f(0.0f,   0.0f);
        glVertex2f(0.0f,    0.0f);

        glTexCoord2f(0.0f,   1.0f);
        glVertex2f(0.0f,    0.4f);

        glTexCoord2f(1.0f,  1.0f);
        glVertex2f(1.0f,    0.4f);

        glTexCoord2f(1.0f,  0.0f);
        glVertex2f(1.0f,    0.0f);

        glEnd();

    }


//    glDisable(GL_BLEND);

    glPopMatrix();

    // glDisableVertexAttribArray(frame2d.attrib_position);
    // glDisableVertexAttribArray(frame2d.attrib_texcoord);

    //m_program->disableAttributeArray(
    //    m_attrib_position
    //);
    //m_program->disableAttributeArray(
    //    m_attrib_texcoord
    //);

    // glUseProgram(0);

    //m_program->release();

    glBindTexture(GL_TEXTURE_2D, 0);
}

void NeuronConsole::renderOpenGl(
    const int32_t   &a_with_name,
    const int32_t   &a_selected)
{
    QString             msg;
    uint32_t            cur_row, cur_col;
//    const QGLContext    *context = NULL;
//    context = getViewerContext();

    if (a_with_name){
        // add name
        glPushName(a_with_name);
    }

    if (0){
        for (cur_row = 0;
            cur_row < m_rows;
            cur_row++)
        {
            PropVectorSptr row;
            row = dynamic_pointer_cast<PropVector>(
                m_buffer->get(cur_row)
            );
            for (cur_col = 0;
                cur_col < m_cols;
                cur_col++)
            {
                PropStringSptr col;
                col = dynamic_pointer_cast<PropString>(
                    row->get(cur_col)
                );
                string msg = col->getVal();
                if (msg.size()){
                    displayText(
                        Vec(
                            -0.5f + 0.005f * cur_col,
                            -0.5f + 0.005f * cur_row,
                            0.0f
                        ),
                        msg,
                        true    // render
                    );
                }
            }
        }
    }

    if (0){
        for (cur_row = 0;
            cur_row < m_rows;
            cur_row++)
        {
            string          msg;
            PropVectorSptr  row;
            row = dynamic_pointer_cast<PropVector>(
                m_buffer->get(cur_row)
            );
            for (cur_col = 0;
                cur_col < m_cols;
                cur_col++)
            {
                PropStringSptr  prop_col;
                string          val;
                prop_col = dynamic_pointer_cast<PropString>(
                    row->get(cur_col)
                );
                val = prop_col->getVal();
                if (!val.size()){
                    val = " ";
                }
                msg += val;
            }
            displayText(
                Vec(
                    -1.0f,
                    -1.0f + 0.02f * cur_row,
                    0.0f
                ),
                msg,
                true
            );
        }
    }

    if (1){
        displayConsole(
            Vec(
                -1.0f,
                -1.0f + 0.02f * cur_row,
                0.0f
            ),
//            1400,
//            400,
            true
        );
    }

    if (a_with_name){
        // remove name
        glPopName();
    }
}

// ---------------- api -------------------

int32_t NeuronConsole::apiObjectUpdate(
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_props)
{
    int32_t     res, err = -1;
//    PropSptr    prop_status;

    // search status
//    prop_status = a_props->get("status");
//    if (prop_status){
//        setStatus(PROP_STRING(
//            prop_status->toString()
//        ));
//    }

    // process upper props
    res = SUPER_CLASS::apiObjectUpdate(
        a_answer,
        a_props
    );
    if (res){
        PERROR("Cannot update props for object with ID: '%s'\n",
            getId()->toString().c_str()
        );
        err = res;
        goto fail;
    }

    // all ok
    err = 0;

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}


/*
void NeuronConsole::apiSetStatus(
    Neuron              *a_neuron,
    HttpAnswerSptr      a_answer,
    const PropMapSptr   a_req_props)
{
    NeuronConsole     *neuron = NULL;
    PropSptr        prop_status;
    string          status;
    PropMapSptr     params;
    PropMapSptr     answer_params;

    neuron = dynamic_cast<NeuronConsole *>(a_neuron);

    // TODO check perms here

    // prepare answer params
    answer_params = a_answer->getCreateAnswerParams();

    {
        PropSptr tmp = a_req_props->get("params");
        params = dynamic_pointer_cast<PropMap>(tmp);
    }

    if (params){
        prop_status = params->get("status");
    }
    if (prop_status){
        status = prop_status->toString();
    }
    if (    "play"  == status
        ||  "pause" == status
        ||  "stop"  == status)
    {
        neuron->setStatus(PROP_STRING(status));
        neuron->save();
    } else if (status.size()){
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                "\"descr\": \"unsupported value"
                    " for param 'status'\""
            "}"
        );
        goto out;
    } else {
        a_answer->setAnswerStatusCode(200);
        a_answer->setAnswerStatusString("OK");
        a_answer->setAnswerBody(
            "{"
                "\"status\": \"failed\","
                "\"descr\": \"missing or empty param 'status'\""
            "}"
        );
        goto out;
    }

    a_answer->setAnswerStatusCode(200);
    a_answer->setAnswerStatusString("OK");
    a_answer->setAnswerApiStatus("success");
    a_answer->setAnswerApiDescr(
        "status was updated"
    );

out:
    return;
}
*/

// ---------------- module ----------------

int32_t NeuronConsole::init()
{
    int32_t err = 0;
    return err;
}

void NeuronConsole::destroy()
{
}

NeuronSptr NeuronConsole::object_create()
{
    NeuronSptr neuron(new NeuronConsole);
    return neuron;
}

static ModuleInfo info = {
    .type           = "core.neuron.console",
    .init           = NeuronConsole::init,
    .destroy        = NeuronConsole::destroy,
    .object_create  = NeuronConsole::object_create,
    .flags          = 0
};

REGISTER_MODULE(info);

