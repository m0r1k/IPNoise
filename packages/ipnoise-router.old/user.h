#ifndef USER_H
#define USER_H

#include <stdint.h>
#include <vector>
#include <string>
#include <map>

#include <event.h>
#include <evhttp.h>

#include "chat.h"

#include "uiSession.h"
#include "netSession.h"

#include "routeClass.h"

using namespace std;

// sessions
// <sess_id, uiSessionClass>
typedef map <string, uiSessionClass *>  UiSessions;
// <sess_id, netSessionClass>
typedef map <string, uiSessionClass *>  NetSessions;

// <huid, *UserClass>
typedef map <string, void*>             Items;

// <chat_id, ChatClass>
typedef map <int, ChatClass *>          History;

class UserClass
{
    public:
        UserClass(string huid, string login);
        ~UserClass();

        uiSessionClass *openSession();

        string getXMLItems();
        string getXMLGroups();

        int EventUpdateContactList(struct evbuffer *out = NULL,
            uiSessionClass *session = NULL);
        int historyAdd(string dst_huid,
            history_item &hitem,
            int src_chatid = 0, int dst_chatid = 0);
        int send2AllUiSessions(string &xml);
        int save(string root_dir);

        int                 setAvatar(string avatar_icon);
        string              getAvatar();
        int                 setStatus(string status_icon);
        string              getStatus();
        string              getDescr(void);
        void                setDescr(string);
        string              getHuid(void);
        void                setHuid(string);
        string              getLogin(void);
        void                setLogin(string);
        void                addItem(UserClass *item);
        void                delItem(string &huid);
        string              getGroup();
        void                setGroup(string);
        void                delChatMsg(int chat_id, int msg_id);
        void                addHistory(ChatClass *chat);
        ChatClass           *getHistory(int chat_id);
        UserClass           *getItem(string huid);
        ChatClass           *getLastChat();
        History             *getAllChats();
        uiSessionClass      *getSessById(string sessid);
        ChatClass           *getChat(int chat_id);
        void                dump();
        void                watchdog();
        string              generateFreeHuid();
        string              getXml();
        static UserClass    *getUser(string huid);
        static UserClass    *searchUser(string);
        static void         putUser(string huid);
        static void         closeSession(uiSessionClass *sess);
        static UserClass    *loadFromFile(char *filename,
            UserClass *user = NULL);

        void icmpReply(__u8 *buf, uint32_t len){
            routes.icmpReply(buf, len);
        };
 
        void checkRoutes(){
            routes.checkRoutes();
        };

        int32_t addRoute(const char *dev, const char *lladdr);

        private:
            UiSessions      ui_sessions;
            NetSessions     net_sessions;

            History         history;
            Items           items;
            routeClass      routes;

            string          login;
            string          huid;
            string          group_id;
            string          status_icon; 
            string          avatar_icon;
            string          descr;
            int             dirty;
            int             deleted; //!< item was deleted

            // Родитель item'а
            UserClass       *parent;            
};

// huid, UserClass
typedef map<string, UserClass *>        Users;

#endif

