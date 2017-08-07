#include <stdlib.h>

#include "user.h"
#include "base64.h"
#include "shttp.h"
#include "parts.h"

// all users
Users USERS;

uiSessionClass *UserClass::openSession()
{
    uiSessionClass *session       = new uiSessionClass();
    session->user                 = (void *)this;
    ui_sessions[session->sess_id] = session;
    return session;
}

string UserClass::getXMLItems()
{
    string ret = "";
    Items::iterator item_i;

    ret += "<items>\n";

    for (item_i = items.begin(); item_i != items.end(); item_i++){
        UserClass *user = (UserClass *)item_i->second;
        string    huid  = user->huid;
        if (user->deleted){
            // user was deleted
            continue;
        }
        ret += "<item name='"+user->login+"'"
                    " huid='"+user->huid+"'"
                    " status_icon='"+user->status_icon+"'"
                    " avatar_icon='"+user->avatar_icon+"'/>\n";

    }
    ret += "</items>\n";

    return ret;
}

string UserClass::getXMLGroups()
{
    string ret = "";
    Items::iterator item_i;

    // group_id, items
    map <string, vector<UserClass *> >           items_by_groups;
    map <string, vector<UserClass *> >::iterator items_by_groups_i;

    for (item_i = items.begin(); item_i != items.end(); item_i++){
        UserClass *user = (UserClass *)item_i->second;
        if (user->deleted){
            // item was deleted
            continue;
        }
        if (user->group_id == ""){
            user->group_id = "0";
        }
        items_by_groups[user->group_id].push_back(user);
    }

    ret += "<groups>\n";

    for (items_by_groups_i  = items_by_groups.begin();
         items_by_groups_i != items_by_groups.end();
         items_by_groups_i++)
    {
        string name = "";
        string icon = "";

        if (items_by_groups_i->first == "2210"){
            name = "Сервисы";
            icon = "images/icon2o.gif";
        } else if (items_by_groups_i->first == "2211"){
            name = "Каналы";
            icon = "images/icon2o.gif";
        } else if (items_by_groups_i->first == "2212"){
            name = "Заметки";
            icon = "images/icon1o.gif";
        } else {
            name = "Все пользователи";
            icon = "images/icon3o.gif";
        }

        ret += "<group";
        ret += " id='"+items_by_groups_i->first+"'";
        ret += " name='"+name+"'";
        ret += " icon='"+icon+"'";
        ret += ">\n";

        vector<UserClass *>::iterator item_tmp;

        for (item_tmp  = items_by_groups_i->second.begin();
             item_tmp != items_by_groups_i->second.end();
             item_tmp++)
        {
            UserClass *item = *item_tmp;
            ret += "<item huid='"+item->huid+"'/>\n";
        }
        ret += "</group>\n";
    }

    ret += "</groups>\n";

    return ret;
}

string UserClass::getAvatar()
{
    return avatar_icon;
}

int UserClass::setAvatar(string _avatar_icon)
{
    avatar_icon = _avatar_icon;
}

string UserClass::getStatus()
{
    return status_icon;
}

int UserClass::setStatus(string _status_icon)
{
    Items::iterator item_i;
    Items::iterator item_i2;

    if (_status_icon.size() && status_icon != _status_icon){
        PDEBUG(5, "Set status icon to: '%s'\n", _status_icon.c_str());
        status_icon = _status_icon;
        for (item_i = items.begin();
            item_i != items.end();
            item_i++)
        {
            UserClass *item = (UserClass *)item_i->second;
            if (item->deleted){
                // item was deleted
                continue;
            }
            // search logined user
            PDEBUG(5, "DDD: %s\n", item->huid.c_str());
            UserClass *user = getUser(item->huid);
            if (user != NULL){
                item_i2 = user->items.find(huid);
                if (item_i2 != user->items.end()){
                    UserClass *self = (UserClass *)item_i2->second;
                    if (!self->deleted){
                        self->status_icon = _status_icon;
                        user->EventUpdateContactList();
                    }
                }
            }
        }
    }
}

string UserClass::getXml()
{
    string xml = "";
    xml += "   <item ";
    xml += "     huid=\""+huid+"\"";
    xml += "     name=\""+login+"\"";
    xml += "     descr=\""+descr+"\"";
    xml += "     status_icon=\""+status_icon+"\"";  
    xml += "     avatar_icon=\""+avatar_icon+"\"";
    xml += "   />\n";
    return xml;
}

int UserClass::send2AllUiSessions(string &xml)
{
    UiSessions::iterator sess_i;
    struct evbuffer *evb = NULL;

    string encoded = base64_encode(
        reinterpret_cast<const unsigned char*>(xml.c_str()),
        xml.length()
    );

    // get all ui_sessions
    for (sess_i  = ui_sessions.begin();
         sess_i != ui_sessions.end();
         sess_i++)
    {
        uiSessionClass *cur_session = sess_i->second;
        if (cur_session != NULL){
            evb = evbuffer_new();
            evbuffer_add_printf(evb, "%s\n", encoded.c_str());
            cur_session->sendEvbuff(evb);
            evbuffer_free(evb);
        }
    }
}

int UserClass::EventUpdateContactList(struct evbuffer *out, uiSessionClass *session)
{
    string answer = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    if (session != NULL){
        answer += "<ipnoise sess_id=\""+session->sess_id+"\">          \n";
    } else {
        answer += "<ipnoise>                                           \n";
    }
    answer += "  <events>                                              \n";
    answer += "      <event type='update-contact-list'>                \n";
    answer += getXMLGroups();
    answer += getXMLItems();
    answer += "      </event>                                          \n";
    answer += " </events>                                              \n";
    answer += "</ipnoise>                                              \n";

    if (out != NULL){
        add2out(out, answer);
    }
    send2AllUiSessions(answer);
}

int UserClass::historyAdd(string dst_huid, history_item &hitem,
    int src_chatid, int dst_chatid)
{
    string answer;
    char buf[128];

    answer = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    answer += "<ipnoise>                                        \n";
    answer += "  <events>                                       \n";
    answer += "      <event type='update-history' huid='"
        +dst_huid+"'>\n";
    answer += "          <msg";
    snprintf(buf, sizeof(buf) - 1, "%d", hitem.id);
    answer += "              id='"+string(buf)+"'";
    answer += "              msg_type='"+hitem.msg_type+"'";
    snprintf(buf, sizeof(buf) -1, "%d", hitem.time);
    answer += "              time='"+string(buf)+"'";
    snprintf(buf, sizeof(buf) - 1, "%d", src_chatid);
    answer += "              src_chatid='"+string(buf)+"'";
    snprintf(buf, sizeof(buf) - 1, "%d", dst_chatid);
    answer += "              dst_chatid='"+string(buf)+"'";
    answer += "><![CDATA["+hitem.msg+"]]></msg>\n";
    answer += "      </event>                                   \n";
    answer += " </events>                                       \n";
    answer += "</ipnoise>                                       \n";

    PDEBUG(5, "%s:\n%s\n", hitem.msg_type.c_str(), answer.c_str());

    send2AllUiSessions(answer);

    return 0;
}

int UserClass::save(string root_dir)
{
    FILE        *out    = NULL;
    ChatClass   *chat   = NULL;
    int res, err;

    Items::iterator     items_i;
    History::iterator   hist_i;

    string items_dir    = "";

    // create huid directory
    string huid_dir = root_dir + "/" + huid.c_str();
    if (deleted){
        // item was deleted
        string cmd = "rm -rf '"+huid_dir+"'";
        // XXX don't use system command
        my_system(cmd.c_str());
        return 0;
    }
    mkdir((char *)huid_dir.c_str(), NEW_DIRS_MODE);

    string descr_encoded = base64_encode(
        reinterpret_cast<const unsigned char*>(descr.c_str()),
        descr.length()
    );

    // create file
    string filename = huid_dir + string("/user.dat");
    string fname    = huid_dir + string("/user.dat.new");

    err = 0;
    do {
        // try to open
        out = fopen(fname.c_str(), "w+");
        if (out == NULL){
            PERROR("Cannot open file for write: '%s'\n",
                fname.c_str());
            err = 1;
            break;
        }       
        res = fprintf(out,
            "HUID                   %s\n"
            "LOGIN                  %s\n"
            "GROUP_ID               %s\n"
            "DESCR                  %s\n"
            "AVATAR_ICON            %s\n", 
            huid.c_str(),
            login.c_str(),
            group_id.c_str(),
            descr_encoded.c_str(),
            avatar_icon.c_str()
        );
        if (res <= 0){
            PERROR("fprintf failed()\n");
            err = 1;
        }
        res = fclose(out);
        if (res){
            PERROR("fclose failed()\n");
            err = 1;
        }
        if (err){
            break;
        }
        rename(fname.c_str(), filename.c_str());
    } while (0);

    PDEBUG(5, "SAVE USER FILE: ERR: '%d'\n", err);
    unlink(fname.c_str());
    if (err){
        goto ret;
    }

    // save history
    for (hist_i = history.begin();
        hist_i != history.end();
        hist_i++)
    {
        chat = (ChatClass *)hist_i->second;
        PDEBUG(7, "\t\tCHAT_ID: '%d' State: '%d'\n",
            chat->getId(),
            chat->getState()
        );

        // save history {
        res = chat->save(huid_dir);
        if (res){
            PERROR("Cannot save chat()\n");
            err = 1;
            goto ret;
        }
        // save history }
    }

    if (items.size()){
        // create items directory
        items_dir = huid_dir + "/items/";
        mkdir((char *)items_dir.c_str(), NEW_DIRS_MODE);

        // save items
        for (items_i = items.begin();
            items_i != items.end();
            items_i++)
        {
            UserClass *item = (UserClass*)items_i->second;
            if (item == this){
                // skip link on him self
                continue;
            }
            PDEBUG(7, "\tITEM: '%s'\n", item->getHuid().c_str());

            // save information about user {
            res = item->save(items_dir);
            if (res){
                PERROR("Cannot save item()\n");
                err = 1;
                goto ret;
            }
            // save information about user }
        }
    }

ret:
    return err;
}

string UserClass::getLogin()
{
    return login;
}

void UserClass::setLogin(string new_login)
{
    login = new_login;
    dirty = 1;
}

string UserClass::getDescr()
{
    return huid;
}

void UserClass::setDescr(string new_descr)
{
    descr  = new_descr;
    dirty = 1;
}

string UserClass::getHuid()
{
    return huid;
}

void UserClass::setHuid(string new_huid)
{
    huid  = new_huid;
    dirty = 1;
}

string UserClass::getGroup()
{
    return group_id;
}

void UserClass::setGroup(string _group_id)
{
    group_id = _group_id;
    dirty    = 1;
}

void UserClass::addItem(UserClass *item)
{
    items[item->huid] = item;
    item->parent      = this;
    dirty = 1;
    EventUpdateContactList();
}

void UserClass::delItem(string &huid)
{
    Items::iterator item_i;
    UserClass *item = NULL;

    item_i = items.find(huid);
    if (item_i != items.end()){
        item = (UserClass *)item_i->second;
        item->deleted = 1;
        dirty = 1;
        EventUpdateContactList();
    }
}

ChatClass *UserClass::getHistory(int chat_id)
{
    ChatClass *chat = NULL;
    History::iterator hist_i;

    hist_i = history.find(chat_id);
    if (hist_i != history.end()){
        chat = (ChatClass *)hist_i->second;
    }
    return chat;
}

History *UserClass::getAllChats()
{
    return &history;
}

UserClass *UserClass::getItem(string huid)
{
    UserClass *item = NULL;
    Items::iterator item_i;

    item_i = items.find(huid);
    if (item_i != items.end()){
        item = (UserClass *)item_i->second;
        if (item->deleted){
            // item was deleted
            item = NULL;
        }
    }

    return item;
}

ChatClass *UserClass::getChat(int chat_id)
{
    ChatClass *chat = NULL;
    History::iterator history_i;

    history_i = history.find(chat_id);
    if (history_i != history.end()){
        chat = (ChatClass *)history_i->second;
    }
    return chat;
}

ChatClass *UserClass::getLastChat()
{
    ChatClass *chat = NULL;
    History::iterator history_i;

    int last_chat_id            = 0;
    int last_inactive_chat_id   = 0;
    int last_active_chat_id     = 0;

    for (history_i  = history.begin();
         history_i != history.end();
         history_i++)
    {
        chat = history_i->second;
        if (!chat->historySize()){
            continue;
        }
        if (chat->getState() >= ChatClass::CHAT_STATE_ACTIVE){
            if (chat->getId() > last_active_chat_id){
                last_active_chat_id = chat->getId();
            }
        } else {
             if (chat->getId() > last_chat_id){
                last_inactive_chat_id = chat->getId();
            }
        }
        if (last_active_chat_id){
            last_chat_id = last_active_chat_id;
        } else {
            last_chat_id = last_inactive_chat_id;
        }
    }
    history_i = history.find(last_chat_id);
    if (history_i != history.end()){
        chat = history_i->second;
    } else {
        chat = NULL;
    }

    return chat;
}

void UserClass::delChatMsg(int chat_id, int msg_id)
{
    ChatClass *chat = getHistory(chat_id);
    if (chat == NULL){
        return;
    }
    chat->historyDel(msg_id);
}

void UserClass::addHistory(ChatClass *chat)
{
    history[chat->getId()] = chat;
    PDEBUG(7, "store chat: ID: %d\n", chat->getId());
    dirty = 1;
}

void UserClass::dump()
{
    Items::iterator     items_i;
    History::iterator   hist_i;

    for (items_i = items.begin();
        items_i != items.end();
        items_i++)
    {
        UserClass *item = (UserClass *)items_i->second;
        if (item->deleted){
            // item was deleted
            continue;
        }
        PDEBUG(7, "\tITEM: '%s' (0x%x)\n", item->huid.c_str(), item);
        for (hist_i = item->history.begin();
            hist_i != item->history.end();
            hist_i++)
        {
            ChatClass *chat = hist_i->second;
            PDEBUG(7, "\t\tCHAT_ID: '%d' State: '%d' (0x%x)\n",
                chat->getId(),
                chat->getState(),
                hist_i->second
            );
            chat->dump();
        }
    }
}

// generate internal huid
string UserClass::generateFreeHuid()
{
    int i;
    char buf[256];
    string huid     = "";
    Items::iterator item_i;

    unsigned short oct[7] = { 1, 0, 0, 0, 0, 0, 0 };

    do {
        // Start from: 2211:0:0:0:0:0:0:1
        snprintf(buf, sizeof(buf) - 1,
            "2211:%d:%d:%d:%d:%d:%d:%d",
            oct[6], oct[5], oct[4], oct[3],
            oct[2], oct[1], oct[0]
        );
        item_i = items.find(buf);
        if (item_i == items.end()){
            // we have found free huid
            huid = buf;
            break;
        }
        // search next free
        oct[0]++;
        for (i = 0; i < 6; i++){
            if (oct[i] > 65535){
                oct[i] = 0;
                oct[i+1]++;
            }
        }
        if (i == 7){
            break;
        }
    } while (1);

    PDEBUG(5, "Was generated free internal HUID: '%s'\n",
        huid.c_str());
    return huid;
}

void UserClass::watchdog()
{
    Items::iterator         items_i; 
    UiSessions::iterator    sess_i;

    for (sess_i  = ui_sessions.begin();
         sess_i != ui_sessions.end();
         sess_i++)
    {
        uiSessionClass *session = sess_i->second;
        if (session != NULL){
            session->watchdog();
        }
    }

    // check all user's routes
    for (items_i = items.begin();
        items_i != items.end();
        items_i++)
    {
        UserClass *item = (UserClass *)items_i->second;
        if (item->deleted){
            // item was deleted
            continue;
        }
        if (item == this){
            // it is we self, skip
            continue;
        }
        if (item->huid.substr(0, 4) == "2211"){
            // it is local huid
            continue;
        }

        PDEBUG(5, "MORIK: user: '%s', check routes for item: '%s'\n",
            login.c_str(),
            item->login.c_str()
        );
        item->checkRoutes();
    }
}

uiSessionClass *UserClass::getSessById(string sessid)
{
    UiSessions::iterator sess_i;
    uiSessionClass *sess = NULL;

    // search current session
    sess_i = ui_sessions.find(sessid);
    if (sess_i != ui_sessions.end()){
        sess = sess_i->second;
    }
    return sess;
}

/*
 * маршрут до item'а контакт листа
 * src_huid пользователь которому принадлежит item
 */
int32_t UserClass::addRoute(const char *dev, const char *lladdr)
{
    int32_t err = -1;

    do {
        if (parent == NULL){
            PERROR("parent == NULL, maybe it's not item?"
                "huid: '%s', login: '%s'\n",
                huid.c_str(), login.c_str()
            );
            break;
        }

        if (dev == NULL || lladdr == NULL){
            PERROR("All arguments must be defined:"
                " (dev, lladdr)"
                " dev: '%s', lladdr: '%s'"
                "\n"
            );
            break;
        }
       
        PDEBUG(5, "--- MORIK --- add route dev: '%s', lladdr: '%s', src: '%s', dst: '%s', parent_login: '%s'\n",
            dev,
            lladdr,
            parent->getHuid().c_str(),
            huid.c_str(),
            parent->login.c_str()
        );

        // TODO check duplicates before add
        routeRecordClass route(
            dev,                        // hw interface
            lladdr,                     // hw addr
            parent->getHuid().c_str(),  // src huid
            huid.c_str()                // dst huid
        );
        routes.addRoute(route);
        
        err = 0;
    
    } while(0);

    return err;
}

UserClass::UserClass(string _huid, string _login)
{
    huid        = _huid;
    login       = _login;
    status_icon = (const char *)"images/iconarr.gif";
    avatar_icon = "";
    group_id    = "0";
    descr       = "";
    dirty       = 0;
    deleted     = 0;
    parent      = NULL;

//    routeRecordClass rr1(
//        (char*)"udp0",                                      // hw interface
//        (char*)"85.119.78.13:2210",                         // hw addr
//        (char*)huid.c_str(),                                // src huid
//        (char*)"2210:0:7309:227d:3030:fb23:816a:cc5e"       // dst huid
//    );
//    routes.addRoute(rr1);

    //struct timeval tv;
    //gettimeofday(&tv, NULL);
    //rr2.addPingRes(&tv);
    //dump();
}

UserClass::~UserClass()
{
    Users::iterator user_i = USERS.find(huid);
    if (user_i != USERS.end()){
        USERS.erase(user_i);
    }
}

// Static functions

// close session
void UserClass::closeSession(uiSessionClass *sess)
{
    UserClass *user = NULL;

    if (sess == NULL){
        PERROR("Attempt to close NULL session\n");
        return;
    }
    // search session owner
    user = (UserClass *)sess->user;
    if (user == NULL){
        PERROR("Attempt to close session with NULL owner (user)\n");
        return;
    }

    PDEBUG(5, "Close session: '%s' for HUID: '%s'\n",
        sess->sess_id.c_str(),
        user->huid.c_str()
    );

    UiSessions::iterator sess_i;
    sess_i = user->ui_sessions.find(sess->sess_id);
    if (sess_i != user->ui_sessions.end()){
        // remove session from user list
        user->ui_sessions.erase(sess_i);
    }
    delete sess;
    sess = NULL;

    if (!user->ui_sessions.size()){
        // no more ui_sessions, set status offline
        user->setStatus((char*)"images/iconarr.gif");
    }
}

// get user by HUID
UserClass *UserClass::getUser(string huid)
{
    UserClass *user = NULL;
    Users::iterator user_i = USERS.find(huid);
    if (user_i != USERS.end()){
        user = user_i->second;
    }
    return user;
}

// Search user by session
UserClass *UserClass::searchUser(string sess_id)
{
    UserClass *user     = NULL;
    Users::iterator     user_i;
    UiSessions::iterator  sess_i;

    for (user_i = USERS.begin(); user_i != USERS.end(); user_i++){
        UserClass *cur_user = user_i->second;
        sess_i = cur_user->ui_sessions.find(sess_id);
        if (sess_i != cur_user->ui_sessions.end()){
            uiSessionClass *cur_session = sess_i->second;
            // session found
            user = cur_user;
            break;
        }
    }
    return user;
}

UserClass *UserClass::loadFromFile(char *filename, UserClass *user)
{
    FILE        *in         = NULL;
    char        *line       = NULL;
    char        *line_ptr   = NULL;
    char        *key        = NULL;
    char        *value      = NULL;
    struct parts ps;
    int i, state;
    size_t tmp, n;

    string huid         = "";
    string login        = "unknown";
    string group_id     = "0";
    string descr        = "";
    string avatar_icon  = "";

    string k,v;
    map <string, string>            data;
    map <string, string>::iterator  data_i;

    filename = realpath(filename, NULL);
    if (filename == NULL){
        PDEBUG(7, "realpath() failed for: '%s'\n", filename);
        goto ret;
    }

    PDEBUG(10, "Trying to load UserClass from: '%s'\n", filename);

    in = fopen(filename, "rb");
    if (in == NULL){
        PERROR("Cannot open file for read: '%s'\n", filename);
        goto ret;
    }

    do {
        line = NULL;
        tmp  = getdelim(&line, &n, '\n', in);

        if (tmp == (size_t)-1 || line == NULL){
            // важно, проверять на ошибку нужно
            // именно приводя к типу (size_t)-1
            break;
        }

        if (line[0] == '#'){
            // comment line
            free(line);
            continue;
        }

        state       = 0;
        line_ptr    = line;
        key         = NULL;
        value       = NULL;

        do {
            if (state == 2
                && (*line_ptr == '\n'
                    || *line_ptr == '\0'))
            {
                *line_ptr = '\0';
                break;
            }
            if (state == 1 && *line_ptr != ' '){
                value = line_ptr;
                state = 2;
            }
            if (state == 0 && *line_ptr == ' '){
                state = 1;
                key = line;
                *line_ptr = '\0';
            }
            line_ptr++;
        } while (*line_ptr != '\0');

        if (key != NULL){
            if (value != NULL){
                k = key;
                v = value;
                data[k]  = v;
            } else {
                k = key;
                v = "";
                data[k]  = v;
            }
        }

        free(line);
    } while (in);
    fclose (in);

    data_i = data.find("HUID");
    if (data_i != data.end()){
        huid = data_i->second;
    } else {
        PWARN("Cannot read 'HUID' attribute from: '%s'\n", filename);
    }

    data_i = data.find("LOGIN");
    if (data_i != data.end()){
        login = data_i->second;
    } else {
        PWARN("Cannot read 'LOGIN' attribute from: '%s'\n", filename);
    }

    data_i = data.find("GROUP_ID");
    if (data_i != data.end()){
        group_id = data_i->second;
    } else {
        PWARN("Cannot read 'GROUP_ID' attribute from: '%s'\n", filename);
    }

    data_i = data.find("DESCR");
    if (data_i != data.end()){
        descr = data_i->second;
    } else {
        PWARN("Cannot read 'DESCR' attribute from: '%s'\n", filename);
    }

    data_i = data.find("AVATAR_ICON");
    if (data_i != data.end()){
        avatar_icon = data_i->second;
    } else {
        PWARN("Cannot read 'AVATAR_ICON' attribute from: '%s'\n", filename);
    }

    if (!huid.size()){
        PERROR("Cannot open file for read: '%s'\n", filename);
        goto ret;
    }
    if (user == NULL){
        user = new UserClass(huid, login);
    }

    user->huid          = huid;
    user->login         = login;
    user->group_id      = group_id;
    user->descr         = base64_decode(descr);
    user->avatar_icon   = avatar_icon;

ret:
    if (filename != NULL){
        free(filename);
        filename = NULL;
    }
    return user;
}

void UserClass::putUser(string huid)
{
    Users::iterator user_i = USERS.find(huid);
    if (user_i != USERS.end()){
        delete user_i->second;
        USERS.erase(user_i);
    }
}


