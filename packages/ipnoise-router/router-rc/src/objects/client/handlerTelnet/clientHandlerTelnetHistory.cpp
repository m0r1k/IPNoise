#include "objects/client/handlerTelnet/clientHandlerTelnetHistory.h"

History::History(ClientHandlerTelnetObject *_handler)
{
    handler         = _handler;
    history_pos     = 0;
    cmd_line_pos    = 0;

    // create empty line
    commands[cmd_line_pos] = HistoryString();

    // load history commands
    load();
}

History::~History()
{

}

int History::save(const string &fname)
{
    int     err         = -1;
    FILE    *f          = NULL;
    string  tmp_fname   = fname + ".tmp";

    f = fopen(tmp_fname.c_str(), "w");
    if (f == NULL){
        PERROR("Cannot open file for write: '%s\n", fname.c_str());
        goto fail;
    }

    for (int i = 0; i < (int)commands.size(); i++){
        string cmd = commands[i].getSaveValue();
        if (cmd.empty()){
            continue;
        }
        cmd += "\n";
        fwrite(cmd.c_str(), cmd.size(), 1, f);
    }

    // close file
    fclose(f);

    {
        string cmd = "mv \"" + tmp_fname + "\" \"" + fname + "\"";
        system(cmd.c_str());
    }

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int History::load(const string &fname)
{
    int     err = -1;
    FILE    *f  = NULL;
    ssize_t res = 0;

    // clear current comands
    commands.clear();
    history_pos = 0;

    // trying to load history from file
    f = fopen(fname.c_str(), "r");
    if (!f){
        PERROR("Cannot open: '%s' for read\n", fname.c_str());
        goto fail;
    }

    do {
        char    *lineptr    = NULL;
        size_t  n           = 0;

        res = getline(&lineptr, &n, f);
        if (res <= (ssize_t)0){
            break;
        }

        if (lineptr){
            // don't add 'new line' symbol
            lineptr[strlen(lineptr) - 1] = '\0';
            // create command
            HistoryString value;
            value.setValue(lineptr);
            value.setSaveValue(lineptr);
            setCurItem(value);
            // create next item
            createNewItem();
            free(lineptr);
        }
    } while (1);

    // close file
    fclose(f);

    // create empty line
    createNewItem();

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

void History::normalize_line_pos()
{
    string value = getCurItemValue();
    if (cmd_line_pos > int(value.size())){
        cmd_line_pos = value.size();
    }
    if (cmd_line_pos < 0){
        cmd_line_pos = 0;
    }
}

void History::addItem(const HistoryString &a_command)
{
    int old_pos = history_pos;

    // goto top
    history_pos = commands.size() - 1;
    if (history_pos < 0){
        history_pos = 0;
    }
    setCurItem(a_command);

    // create new item
    history_pos = old_pos;
    createNewItem();
}

void History::createNewItem()
{
    string cur_value  = getCurItemValue();
    string prev_value = getPrevItemValue();
    string top_value  = getTopItemValue();

    // goto top
    history_pos = commands.size() - 1;
    if (history_pos < 0){
        history_pos = 0;
    }

    // it is top
    if (cur_value == prev_value){
        // already exist, just clear command line
        setCurItem(HistoryString());
    } else {
        history_pos++;
        setCurItem(HistoryString());
    }

    normalize_line_pos();
    return;
};

void History::clearLine()
{
    int     i       = 0;
    int     steps   = 0;
    string  value   = getCurItemValue();
    for (i = cmd_line_pos; i < int(value.size()); i++){
        // clear between cur pos and end of line
        handler->write(" ");
        steps++;
    }
    for (i = 0; i < (cmd_line_pos + steps); i++){
        handler->write("\x08 \x08");
    }
}

void History::down()
{
    clearLine();

    history_pos++;
    if (history_pos > (int(commands.size()) - 1)){
        history_pos = commands.size() - 1;
    }

    string value;
    if (history_pos != (int)commands.size() - 1){
        setCurItemValue(getCurItemSaveValue());
    }

    value = getCurItemValue();
    cmd_line_pos = value.size();

    handler->write(value);
}

void History::up()
{
    clearLine();

    history_pos--;
    if (history_pos < 0){
        history_pos = 0;
    }

    string value;
    if (history_pos != (int)commands.size() - 1){
        setCurItemValue(getCurItemSaveValue());
    }

    value = getCurItemSaveValue();
    cmd_line_pos = value.size();

    handler->write(value);
}

void History::setCurItem(const HistoryString &a_value)
{
    commands[history_pos] = a_value;
    cmd_line_pos = getCurItemValue().size();
    normalize_line_pos();
}

void History::setCurItemValue(const string &a_value)
{
    HistoryString cur_item = getCurItem();
    cur_item.setValue(a_value);
    setCurItem(cur_item);
}

void History::setCurItemSaveValue(const string &a_value)
{
    HistoryString cur_item = getCurItem();
    cur_item.setSaveValue(a_value);
    setCurItem(cur_item);
}

HistoryString History::getCurItem()
{
    HistoryString ret;

    if (not commands.empty() && history_pos >= 0){
        // only if commands count > 0
        ret = commands[history_pos];
    }

    return ret;
}

string History::getCurItemValue()
{
    HistoryString cur_item = getCurItem();
    return cur_item.getValue();
}

string History::getCurItemSaveValue()
{
    HistoryString cur_item = getCurItem();
    return cur_item.getSaveValue();
}

HistoryString History::getPrevItem()
{
    HistoryString ret;

    if (commands.size() >= 2){
        // only if commands count > 1
        ret = commands[commands.size() - 2];
    }

    return ret;
}

string History::getPrevItemValue()
{
    HistoryString prev_item = getPrevItem();
    return prev_item.getValue();
}

HistoryString History::getTopItem()
{
    HistoryString ret;

    if (commands.size()){
        // only if commands count > 1
        ret = commands[commands.size() - 1];
    }

    return ret;
}

string History::getTopItemValue()
{
    HistoryString top_item = getTopItem();
    return top_item.getValue();
}

void History::upCurItemColPos()
{
    string value = getCurItemValue();
    if (cmd_line_pos < int(value.size())){
        handler->write(getCurItemCol());
        cmd_line_pos++;
    }
}

void History::downCurItemColPos()
{
    HistoryString value = getCurItem();
    if (cmd_line_pos > 0){
        cmd_line_pos--;
        handler->write("\x08");
    }
}

string History::getCurItemCol()
{
    string ret;

    string value = getCurItemValue();
    ret += value.at(cmd_line_pos);

    return ret;
}

int History::getCurItemColPos()
{
    return cmd_line_pos;
}

void History::addCurItemColPos(const char symbol)
{
    int i;
    string  value       = getCurItemValue();
    string  before_pos  = "";
    string  after_pos   = "";

    before_pos = value.substr(0, cmd_line_pos);
    if (cmd_line_pos < int(value.size())){
        after_pos = value.substr(cmd_line_pos);
    }

    value = before_pos;
    value += symbol;
    value += after_pos;

    // PWARN("MORIK value: '%s', cmd_line_pos: '%d',"
    //    " before_pos: '%s', after_pos: '%s'"
    //    " value: '%s'\n",
    //    value.c_str(),
    //    cmd_line_pos,
    //    before_pos.c_str(),
    //    after_pos.c_str(),
    //    value.c_str()
    // );

    // update current history line
    {
        int cur_cmd_line_pos = cmd_line_pos;
        setCurItemValue(value);
        cmd_line_pos = cur_cmd_line_pos;
    }

    // rewrite line from cursor to end
    for (i = cmd_line_pos; i < int(value.size()); i++){
        string val;
        val += value.at(i);
        handler->write(val);
    }
    // move cursor back
    for (i = cmd_line_pos; i < (int(value.size()) - 1); i++){
        handler->write("\x08");
    }

    // move cursor pos
    cmd_line_pos++;
}

void History::delCurItemColPos()
{
    int i;
    string  value       = getCurItemValue();
    string  before_pos  = "";
    string  after_pos   = "";

    if (!value.size()){
        goto out;
    }

    normalize_line_pos();
    after_pos = value.substr(cmd_line_pos);

    if (cmd_line_pos > 0){
        before_pos = value.substr(0, cmd_line_pos - 1);
        cmd_line_pos--;
        handler->write("\x08");
    }

    value = before_pos;
    value += after_pos;

    // update current history line
    {
        int cur_cmd_line_pos = cmd_line_pos;
        setCurItemValue(value);
        cmd_line_pos = cur_cmd_line_pos;
    }

    // rewrite line from cursor to end
    for (i = cmd_line_pos; i < int(value.size()); i++){
        string val;
        val += value.at(i);
        handler->write(val);
    }
    handler->write(" ");
    // move cursor back
    for (i = cmd_line_pos; i < int(value.size()); i++){
        handler->write("\x08");
    }
    handler->write("\x08");

out:
    return;
}

