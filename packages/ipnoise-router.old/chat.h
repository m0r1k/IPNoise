#ifndef ITEM_H
#define ITEM_H

#include <time.h>

#include <string>
#include <map>
#include <vector>

using namespace std;

class history_item
{
    public:
        string  msg;
        string  msg_type;
        time_t  time;
        int     id;
};

typedef vector<history_item>            HistoryItems;
typedef vector<history_item>::iterator  HistoryItem_i;

class ChatClass
{
    public:
        ChatClass();
        ~ChatClass();

        enum States {
            CHAT_STATE_CLOSED = 0,
            CHAT_STATE_ACTIVE
        };
 
        int     getId();
        void    setId(int);
        States  getState();
        void    setState(States);
        time_t  getCreateDate();
        void    setCreateDate(time_t);
 
        history_item createHistItem();

        int     historyAdd(history_item &);
        void    historyDel(int msg_id);
        int     historySize();
        HistoryItems historyGet();
        int     save(string root_dir);
        void    dump();

        static ChatClass *loadFromFile(char *filename);

    private:
        HistoryItems    history;
        int             next_msg_id;
        int             chat_id;
        time_t          create_date;
        States          state;
};

#endif

