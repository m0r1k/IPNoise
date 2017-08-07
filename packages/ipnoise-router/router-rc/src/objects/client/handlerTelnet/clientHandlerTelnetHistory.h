#include <string>
#include <map>

using namespace std;

class History;
class HistoryString;

#define CLIENT_HISTORY_FILE "/etc/ipnoise/profile/client.history"

typedef map<int, HistoryString>             ClientHistoryCommands;
typedef ClientHistoryCommands::iterator     ClientHistoryCommandsIt;

#ifndef CLIENT_HANDLER_TELNET_HISTORY_H
#define CLIENT_HANDLER_TELNET_HISTORY_H

#include "objects/client/clientHandler.h"
#include "objects/client/handlerTelnet/clientHandlerTelnetObject.h"

class HistoryString
{
    public:
        HistoryString(){};
        virtual ~HistoryString(){};

        string getValue(){
            return m_value;
        }
        void setValue(const string &a_value){
            m_value = a_value;
        }

        string getSaveValue(){
            return m_save_value;
        }
        void setSaveValue(const string &a_save_value){
            m_save_value = a_save_value;
        }

    protected:
        string  m_value;
        string  m_save_value;
};

class History
{
    public:
        History(ClientHandlerTelnetObject *_handler);
        ~History();

        void            normalize_line_pos();
        void            addItem(const HistoryString &a_command);
        void            createNewItem();
        void            clearLine();
        void            down();
        void            up();
        void            setCurItem(const HistoryString &a_value);
        void            setCurItemValue(const string &a_value);
        void            setCurItemSaveValue(const string &a_value);
        HistoryString   getCurItem();
        string          getCurItemValue();
        string          getCurItemSaveValue();
        HistoryString   getPrevItem();
        string          getPrevItemValue();
        HistoryString   getTopItem();
        string          getTopItemValue();
        void            upCurItemColPos();
        void            downCurItemColPos();
        string          getCurItemCol();
        int             getCurItemColPos();
        void            addCurItemColPos(const char symbol);
        void            delCurItemColPos();

        int             save(const string &fname = CLIENT_HISTORY_FILE);

    protected:
        int             load(const string &fname = CLIENT_HISTORY_FILE);

    private:
        ClientHistoryCommands       commands;
        int                         history_pos;
        int                         cmd_line_pos;
        ClientHandlerTelnetObject   *handler;
};

#endif

