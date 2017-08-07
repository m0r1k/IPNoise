#include "class.hpp"
DEFINE_CLASS(NeuronConsole);

#ifndef NEURON_CONSOLE_HPP
#define NEURON_CONSOLE_HPP

#include <stdio.h>

#include <map>
#include <string>
#include <memory>
#include <QRegExp>
#include <QTimer>

#include "prop/map.hpp"
#include "prop/int.hpp"
#include "prop/string.hpp"
#include "prop.hpp"
#include "prop/neuron/type.hpp"
#include "http_answer.hpp"

#include "core/neuron/neuron/main.hpp"

#include <QSocketNotifier>
DEFINE_CLASS(QSocketNotifier);

using namespace std;

#define CONSOLE_ROWS "24"
#define CONSOLE_COLS "100"

#define REG_HANDLER(rgx, func)  \
    m_esc_handlers[rgx] = func;

typedef void (*ConsoleHandler)(
    NeuronConsole   *a_neuron,
    const string    &a_esc,
    const QRegExp   &a_rgx
);
typedef map<string, ConsoleHandler>     ConsoleHandlers;
typedef ConsoleHandlers::iterator       ConsoleHandlersIt;

#undef  SUPER_CLASS
#define SUPER_CLASS  Neuron

class NeuronConsole
    :   public   SUPER_CLASS
{
    Q_OBJECT

    public:
        NeuronConsole();
        virtual ~NeuronConsole();

        enum ConsoleParseState {
            CONSOLE_PARSE_STATE_REGULAR = 0,
            CONSOLE_PARSE_STATE_ESC
        };

        DEFINE_PROP(Title,  PropStringSptr);
//        DEFINE_PROP(Status, PropStringSptr);

        // neuron
        virtual void    getAllProps(
            PropMapSptr a_props = PROP_MAP()
        );
        virtual int32_t parseBSON(mongo::BSONObj);
        virtual int32_t apiObjectUpdate(
            HttpAnswerSptr,
            const PropMapSptr
        );
        virtual int32_t do_autorun();

        virtual void renderOpenGl(
            const int32_t   &a_with_name,
            const int32_t   &a_selected
        );
        virtual void keyPressEvent(
            QKeyEvent   *a_ev
        );

        // generic
        void    add(const char   &);
        void    add(const string &);

        void    processEsc(
            ConsoleParseState   &a_console_parse_state,
            string              &a_esc
        );
        void    parseInput(
            const string        &a_string,
            ConsoleParseState   &a_console_parse_state,
            string              &a_console_esc
        );

        void            clearModes();
        void            addMode(const int32_t &a_val);
        PropMapSptr     createEmptyModes();
        PropMapSptr     createEmptyCol();
        PropVectorSptr  createEmptyRow();

        void            backspace();
        void            newLine();
        void            addCursor();
        void            parseInput(
            const char          &a_char,
            ConsoleParseState   &a_console_parse_state,
            string              &a_console_esc
        );

        void clear();


        // api
//        static void apiSetStatus(
//            Neuron              *a_neuron,
//            HttpAnswerSptr      a_answer,
//            const PropMapSptr   a_req_props
//        );

        // module
        static int32_t     init();
        static void        destroy();
        static NeuronSptr  object_create();

    protected slots:
        // console
        void    readSlot(int a_fd);
        void    exceptionSlot(int a_fd);
        void    timerBlinkingSlot();

    protected:
        void            initShaders(
            const QGLContext *a_context
        );
        int32_t         initConsole();

        // esc handlers
        static void     escHandlerCursorPositionReport(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );
        static void     escHandlerCursorPosition(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );
        static void     escHandlerCursorUp(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );
        static void     escHandlerCursorDown(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );
        static void     escHandlerCursorForward(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );
        static void     escHandlerCursorBackward(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerSaveCursorPosition(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );
        static void     escHandlerRestoreCursorPosition(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerEraseDisplay(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );
        static void     escHandlerEraseLineFromCursor(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerSetResetMode(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerSetGraphicsMode(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );
        static void     escHandlerSetMode(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );
//        static void     escHandlerResetMode(
//            NeuronConsole   *a_neuron,
//            const string    &a_esc,
//            const QRegExp   &a_rgx
//        );

        static void     escHandlerSetKeyboardStrings(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerResetHard(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerShowPrompt(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerTabAdd(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerTabClear(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerCharacterSet(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        static void     escHandlerKeypadMode(
            NeuronConsole   *a_neuron,
            const string    &a_esc,
            const QRegExp   &a_rgx
        );

        void    s_reset();
        void    h_reset();
        void    setSlaveCharacter(const string &a_val);
        void    setMasterCharacter(const string &a_val);
        void    eraseLineFromCursor();
        void    tabAdd(         const int32_t &a_val = 1);
        void    showCursor(     const int32_t &a_val = 1);
        void    localEcho(      const int32_t &a_val = 1);
        void    cursorUp(       const int32_t &a_val = 1);
        void    cursorDown(     const int32_t &a_val = 1);
        void    cursorForward(
            const int32_t &a_val                = 1,
            const int32_t &a_can_move_down      = 0
        );
        void    cursorBackward(
            const int32_t &a_val                = 1,
            const int32_t &a_can_move_up        = 0
        );

        void    setCursorPosition(
            const uint32_t   &a_row,
            const uint32_t   &a_col
        );

        uint32_t    getCursorRow()  const;
        uint32_t    getCursorCol()  const;
        PropMapSptr getCurColInfo();

        // text
        void displayText(
            const Vec       &a_pos,
            const string    &a_text,
//            const int32_t   &a_text_width,
//            const int32_t   &a_text_height,
            bool            a_render
        );
        void displayConsole(
            const Vec       &a_pos,
//            const int32_t   &a_text_width,
//            const int32_t   &a_text_height,
            bool            a_render
        );

    private:
        void                    do_init();

        // console
        QSocketNotifierSptr     m_notifier_r;
        QSocketNotifierSptr     m_notifier_w;
        QSocketNotifierSptr     m_notifier_x;
        PropVectorSptr          m_buffer;
        uint32_t                m_cols;
        uint32_t                m_rows;
        int32_t                 m_font_size;
        QFont                   m_font;
        uint32_t                m_step_x;
        uint32_t                m_step_y;
        int32_t                 m_width;
        int32_t                 m_height;
        uint32_t                m_cursor_col;
        uint32_t                m_cursor_row;

        ConsoleParseState       m_master_parse_state;
        string                  m_master_esc;

        ConsoleParseState       m_slave_parse_state;
        string                  m_slave_esc;

        int32_t                 m_fd_master;
        int32_t                 m_fd_slave;
        int32_t                 m_shell_pid;
        ConsoleHandlers         m_esc_handlers;

        GLuint                  m_text_texture;

        QGLShader               *m_shader_vertex;
        QGLShader               *m_shader_fragment;
        QGLShaderProgram        *m_program;
        GLuint                  m_attrib_position;
        GLuint                  m_attrib_texcoord;

        string                  m_old_keyboard_event_text;
        int32_t                 m_old_keyboard_event_code;
        Qt::KeyboardModifiers   m_old_keyboard_modifiers;

        PropMapSptr             m_modes;
        int32_t                 m_blinking;
        int32_t                 m_show_cursor;
        int32_t                 m_local_echo;

        // IO characters
        string                  m_slave_character_g0;
        string                  m_master_character_g1;
};

#endif

