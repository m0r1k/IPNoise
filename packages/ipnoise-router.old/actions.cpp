#include <sys/queue.h>
#include <event.h>
#include <evhttp.h>

#include <iconv.h>

#include "shttp.h"
#include "cookie.h"

#include <fstream>

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <sys/types.h>
#include <dirent.h>

#include "user.h"
#include "chat.h"
#include "base64.h"

#include <time.h>

#include "net/icmp.h"

#include <linux/filter.h>
#include <netinet/ip6.h>
#include <netinet/icmp6.h>

// all users
extern Users USERS;

using namespace std;

char *file_buffer        = NULL;

extern string genHuid(string msg, string key);
extern struct myEnviroment myenv;

#define FILE_BUFFER_SIZE    512*1024 // 512K
#define TMP_BUF_SIZE        512

enum dir_type
{
    DATA_DIR = 0,
    USER_DIR,
    ITEMS_DIR,
    ITEM_DIR,
    CHATS_DIR,
    CHAT_DIR
};

UserClass *user = NULL;
UserClass *item = NULL;
ChatClass *chat = NULL;

int dumpUsers (void);
extern struct timeval tv;
extern struct event   ev_timer;

int uptime = 0;

u_short
in_cksum(const u_short *addr, register int len, u_short csum)
{
    register int nleft = len;
    const u_short *w = addr;
    register u_short answer;
    register int sum = csum;

    /*
     *  Our algorithm is simple, using a 32 bit accumulator (sum),
     *  we add sequential 16 bit words to it, and at the end, fold
     *  back all the carry bits from the top 16 bits into the lower
     *  16 bits.
     */
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

    /* mop up an odd byte, if necessary */
    if (nleft == 1)
        sum += htons(*(u_char *)w << 8);

    /*
     * add back carry outs from top 16 bits to low 16 bits
     */
    sum = (sum >> 16) + (sum & 0xffff); /* add hi 16 to low 16 */
    sum += (sum >> 16);         /* add carry */
    answer = ~sum;              /* truncate to 16 bits */
    return (answer);
}

void icmp_cb(int fd, short event, void *arg)
{
    Users::iterator user_i; 
    __u8 *buf = NULL;
    struct msghdr msg;
    struct msghdr *msg_ptr = &msg;

    int res;

    PDEBUG(7, "ICMP CALLBACK\n");
    memset(&msg, 0, sizeof(msg));

    char addrbuf[128];
    char ans_data[4096];
    struct iovec iov;

    u_char *packet;
    int packlen;

    struct cmsghdr *c;
    struct icmp6_hdr *icmph;
    int hops = -1;

    packlen = 65535;
    packet = (u_char *)malloc(packlen);

    iov.iov_base = (char *)packet;
    iov.iov_len  = packlen;

    msg.msg_name        = addrbuf;
    msg.msg_namelen     = sizeof(addrbuf);
    msg.msg_iov         = &iov;
    msg.msg_iovlen      = 1;
    msg.msg_control     = ans_data;
    msg.msg_controllen  = sizeof(ans_data);

    res = recvmsg(fd, &msg, 0);
    buf = (__u8*)msg.msg_iov->iov_base;

    for (c = CMSG_FIRSTHDR(msg_ptr); c; c = CMSG_NXTHDR(msg_ptr, c)) {
        if (c->cmsg_level != SOL_IPV6)
            continue;
        switch(c->cmsg_type) {
        case IPV6_HOPLIMIT:
            if (c->cmsg_len < CMSG_LEN(sizeof(int)))
                continue;
            hops = *(int*)CMSG_DATA(c);
        }
    }

    /* Now the ICMP part */
    icmph = (struct icmp6_hdr *) buf;
    if (res < 8) {
        PDEBUG(5, "ping: packet too short (%d bytes)\n", res);
        goto ret;
    }

    if (icmph->icmp6_type != ICMP6_ECHO_REPLY) {
        goto ret;
    }

    if (icmph->icmp6_id != IPNOISE_ICMP6_REPLY_ID){
        goto ret;
    }

    // receive answer
    //hexdump((unsigned char*)buf, res); // MORIK
    //PDEBUG(5, "ICMP IDENT: '0x%x'\n", icmph->icmp6_id);

    for (user_i = USERS.begin();
        user_i != USERS.end();
        user_i++)
    {
        UserClass *user = user_i->second;
        user->icmpReply(buf, res);
    }

ret:
    free (packet);
    return;
}

void watchdog_cb(int fd, short event, void *arg)
{
    Users::iterator user_i;
    PDEBUG(10, "watchdog_cb()\n");

    uptime++;

    if (uptime%10 == 0){
        for (user_i = USERS.begin();
            user_i != USERS.end();
            user_i++)
        {
            UserClass *user = user_i->second;
            user->watchdog();
        }
    };

    if (uptime%60 == 0){
        PDEBUG(7, "Saving all to disk..\n");
        save_all();
        PDEBUG(7, "Create backups..\n");
        my_system("/etc/ipnoise/every_min.sh");
    }

    tv.tv_usec = 0;
    tv.tv_sec  = WATCHDOG_TIME_SEC;
    evtimer_add(&ev_timer, &tv);
};

int init_database(string path = "", dir_type type = DATA_DIR)
{
    int err                     = 0;
    DIR             *dir        = NULL;
    struct dirent   *entry      = NULL;
    FILE            *in         = NULL;
    string          user_file   = "";
    string          h           = "";
    struct          stat st;

    PDEBUG(10, "Trying to open: '%s'\n", path.c_str());

    dir = opendir(path.c_str());
    if (!dir){
        PERROR("Cannot open: '%s'\n", path.c_str());
        err = 1;
        goto ret;
    };

    while ((entry = readdir(dir)) != NULL){
        string a = path + "/" + entry->d_name;
        stat((char *)a.c_str(), &st);
        switch (type){
            case (CHAT_DIR):
                if (chat == NULL){
                    PERROR("Internal error, chat not exist\n");
                    break;
                }
                if (    !strncmp(entry->d_name, ".",  1)
                    ||  !strncmp(entry->d_name, "..", 2))
                {
                    continue;
                }
                break;
            case (CHATS_DIR):
                if (item == NULL){
                    PERROR("Internal error, item not exist\n");
                    break;
                }
                if (!S_ISDIR(st.st_mode)){
                    continue;
                }
                if (    !strncmp(entry->d_name, ".",  1)
                    ||  !strncmp(entry->d_name, "..", 2))
                {
                    continue;
                }
                PDEBUG(7, "\t\tFound chat:   '%s'\n", entry->d_name);
                user_file = path + "/" + string(entry->d_name)
                    + "/history.dat";
                chat = ChatClass::loadFromFile(
                    (char *)user_file.c_str()
                );
                if (chat != NULL){
                    chat->setId(atoi(entry->d_name));
                    chat->setCreateDate(atoi(entry->d_name));
                    item->addHistory(chat);
                    PDEBUG(7, "\t\tCreated chat: '%s'"
                        " (loaded: %d records)\n",
                        entry->d_name, chat->historySize());
                }
                init_database(path+"/"+entry->d_name, CHAT_DIR);
                break;
            case (ITEM_DIR):
                if (item == NULL){
                    PERROR("Internal error, item not exist\n");
                    break;
                }
                if (!S_ISDIR(st.st_mode)){
                    continue;
                }
                if (    !strncmp(entry->d_name, ".",  1)
                    ||  !strncmp(entry->d_name, "..", 2))
                {
                    continue;
                }
                if (!strncmp(entry->d_name, "chats",  5)){
                    init_database(path+"/"+entry->d_name, CHATS_DIR);
                }
                chat = NULL;
                break;

            case (ITEMS_DIR):
                if (user == NULL){
                    PERROR("Internal error, user not exist\n");
                    break;
                }
                if (!S_ISDIR(st.st_mode)){
                    continue;
                }
                if (    !strncmp(entry->d_name, ".",  1)
                    ||  !strncmp(entry->d_name, "..", 2))
                {
                    continue;
                }
                PDEBUG(7, "\tFound item:   '%s'\n", entry->d_name);
                user_file = path + "/" + entry->d_name + "/user.dat";
                h = user->getHuid();
                item = UserClass::loadFromFile(
                    (char *)user_file.c_str()
                );
                if (item != NULL){
                    user->addItem(item);
                    PDEBUG(7, "\tCreated item: '%s' (0x%x)\n",
                        entry->d_name, item);
                }
                init_database(path+"/"+entry->d_name, ITEM_DIR);
                break;

            case (USER_DIR):
                if (user == NULL){
                    PERROR("Internal error, user not exist\n");
                    break;
                }
                if (!S_ISDIR(st.st_mode)){
                    continue;
                }
                if (    !strncmp(entry->d_name, ".",  1)
                    ||  !strncmp(entry->d_name, "..", 2))
                {
                    continue;
                }
                if (!strncmp(entry->d_name, "items",  5)){
                    init_database(path+"/"+entry->d_name, ITEMS_DIR);
                }
                item = NULL;
                break;

            case (DATA_DIR):
                if (!S_ISDIR(st.st_mode)){
                    continue;
                }
                if (    !strncmp(entry->d_name, ".",  1)
                    ||  !strncmp(entry->d_name, "..", 2))
                {
                    continue;
                }
                if (strncmp(entry->d_name, "2210", 4)){
                    continue;
                }
                PDEBUG(7, "Found user:   '%s'\n", entry->d_name);
                user_file = path + entry->d_name + "/user.dat";
                user = UserClass::loadFromFile(
                    (char *)user_file.c_str()
                );
                if (user != NULL){
                    USERS[user->getHuid()] = user;
                    user->addItem(user);
                    PDEBUG(7, "Created user: '%s' (0x%x)\n",
                        entry->d_name, user);
                }
                PDEBUG(7, "Going read items for user: 0x%x\n", user);
                init_database(path+"/"+entry->d_name, USER_DIR);
                user = NULL;
            break;
        }
    };

    closedir(dir);
ret:
    return err;
};

int shttpd_init()
{
    int err         = 0;
    int record_id   = 0;
    string path     = "";

    file_buffer = (char *)malloc(FILE_BUFFER_SIZE);
    if (file_buffer == NULL){
        PERROR("Malloc failed, was needed: '%d' bytes\n", FILE_BUFFER_SIZE);
        goto fail;
    }

    // init all databaseѕ
    path  = myenv.conf.root_dir;
    path += "profile/";

    init_database(path);
    dumpUsers();

    // all ok
    err = 0;

ret:
    return err;
fail:
    err = 1;
    goto ret;
}

void save_all()
{
    // подготовка к остановке демона
    string path = "";
    string cmd;

    // сохраняем контакт лист
    path = myenv.conf.root_dir;
    mkdir((char *)path.c_str(), 0755);
    path += "profile/";
    mkdir((char *)path.c_str(), 0755);

    Users::iterator user_i;

    for (user_i = USERS.begin();
        user_i != USERS.end();
        user_i++)
    {
        UserClass *user = user_i->second;
        PDEBUG(7, "HUID: '%s'\n", user->getHuid().c_str());

        // save information about user {
        user->save(path);
        // save information about user }
    }

    PDEBUG(7, "Saving all to disk.. [DONE] \n");
}

void shttpd_destroy(int sid)
{
    PDEBUG(5, "server going down..\n");

    save_all();

    PDEBUG(7, "Create backups..\n");
    my_system("/etc/ipnoise/every_min.sh");

    if (file_buffer != NULL){
        free (file_buffer);
    }
    exit(0);
}

string getContentType(string fname)
{
    string ret;

    if (fname.size() >= 5
        && fname.substr(fname.size() - 5, 5) == ".html")
    {
        ret = "text/html";
    } else if (fname.size() >= 3
        && fname.substr(fname.size() - 3, 3) == ".js")
    {
        ret = "application/javascript";
    } else if (fname.size() >= 4
        && fname.substr(fname.size() - 4, 4) == ".css")
    {
        ret = "text/css";
    } else if (fname.size() >= 4
        && fname.substr(fname.size() - 4, 4) == ".gif")
    {
        ret = "image/gif";
    } else if (fname.size() >= 4
        && fname.substr(fname.size() - 4, 4) == ".wav")
    {
        ret = "audio/x-wav";
    } else if (fname.size() >= 4
        && fname.substr(fname.size() - 4, 4) == ".swf")
    {
        ret = "application/x-shockwave-flash";
    } else if (fname.size() >= 4
        && fname.substr(fname.size() - 4, 4) == ".jpg")
    {
        ret = "image/jpeg";
    } else if (fname.size() >= 4
        && fname.substr(fname.size() - 4, 4) == ".png")
    {
        ret = "image/png";
    } else if (fname.size() >= 5
        && fname.substr(fname.size() - 5, 5) == ".jpeg")
    {
        ret = "image/jpeg";
    } else {
        ret = "application/octet-stream";
    }

    return ret;
}

int add2out(struct evbuffer *out, string &data)
{
    string encoded = base64_encode(
        reinterpret_cast<const unsigned char*>(data.c_str()),
        data.length()
    );
    return evbuffer_add_printf(out, "%s\n", encoded.c_str());
}

int API_login(struct evbuffer *out, struct evhttp_request *req,
    xmlNode *node)
{
    xmlNode *cur_node  = NULL;
    xmlNode *cur_node2 = NULL;
    const xmlChar *nodeName = node->name;

    string login    = "";
    string password = "";
    string cmd      = "";

    for (cur_node = node->children; cur_node; cur_node = cur_node->next){
        if (cur_node->type == XML_ELEMENT_NODE){
            nodeName = cur_node->name;
            if (string((const char*)nodeName) == string("login")){
                for (cur_node2 = cur_node->children; cur_node2; cur_node2 = cur_node2->next){
                    if (cur_node2->type == XML_CDATA_SECTION_NODE){
                        login += (const char *)xmlNodeGetContent(cur_node2);
                    }
                }
            } else if (string((const char*)nodeName) == string("password")){
                for (cur_node2 = cur_node->children; cur_node2; cur_node2 = cur_node2->next){
                    if (cur_node2->type == XML_CDATA_SECTION_NODE){
                        password += (const char *)xmlNodeGetContent(cur_node2);
                    }
                }
            }
        }
    }

    UserClass *user = NULL;
    UserClass *item = NULL;

    string huid = genHuid(login, password);

    user = UserClass::getUser(huid);
    if (user == NULL){
        user = new UserClass(huid, login);
        user->addItem(user);
        USERS[huid] = user;
        PDEBUG(5, "Created new user, huid: '%s', login: '%s' (0x%x)\n",
            huid.c_str(),
            login.c_str(),
            user
        );
        item = new UserClass(
            "2211:0:0:0:0:0:0:1", "все заметки"
        );
        item->setGroup((char *)"2212");
        user->addItem(item);
    }

    user->setLogin(login);
    user->setStatus((char *)"images/iconarr_green.gif");

    // setup ip addr for this huid
    cmd = "ip addr replace "+huid+"/128 dev lo";
    my_system(cmd.c_str());

    // setup service "weather" item {
    const char *weather_huid = "2210:0:7309:227d:3030:fb23:816a:cc5e";
    item = user->getItem(weather_huid);
    if (item == NULL){
        item = new UserClass(weather_huid, "погода");
        item->setGroup("2210");
        // add item
        user->addItem(item);
    }

    // setup first route
    item->addRoute(
        "udp0",                 // hw interface
        "85.119.78.13:2210"     // hw addr
    );
    // setup service "weather" item }

    uiSessionClass *session = user->openSession();
    user->EventUpdateContactList(out, session);

    return 0;
}

int API_searchhistory(struct evbuffer *out, UserClass *cur_user,
    xmlNode *node)
{
    int err = 0;
    char buf[TMP_BUF_SIZE];
    History::iterator       hist_i;

    History   *all_chats    = NULL;
    ChatClass *chat         = NULL;
    const char *huid_ptr    = NULL;
    const char *chatid_ptr  = NULL;
    const char *msg_ptr     = NULL;
    UserClass *dst_user     = NULL;
    string huid             = "";
    string answer           = "";
    string search_msg       = "";
    int chatid              = 0;

    huid_ptr   = (const char*)xmlGetProp(node, (xmlChar *)"huid");
    chatid_ptr = (const char*)xmlGetProp(node, (xmlChar *)"chatid");
    msg_ptr    = (const char*)xmlGetProp(node, (xmlChar *)"msg");

    if (huid_ptr != NULL){
        huid = huid_ptr;
    }

    if (chatid_ptr != NULL){
        chatid = atoi(chatid_ptr);
    }

    if (msg_ptr != NULL){
        search_msg = base64_decode(msg_ptr);
    }

    dst_user = cur_user->getItem(huid);
    if (dst_user == NULL){
        // there are no history
        goto ret;
    }

    answer = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>     \n";
    answer += "<ipnoise>                                      \n";
    answer += "  <events>                                     \n";
    answer += "   <event type='search-history' huid='"+huid+"'> \n";
    answer += "       <chats>                                 \n";

    all_chats = dst_user->getAllChats();
    if (all_chats->begin() != all_chats->end()){
        hist_i = all_chats->end();
        hist_i--;
        do {
            chat = hist_i->second;
            string chat_str = "<chat";
            snprintf(buf, sizeof(buf) - 1, "%d", chat->getId());
            chat_str += " id=\""+string(buf)+"\"";
            snprintf(buf, sizeof(buf) - 1, "%d", chat->getCreateDate());
            chat_str += " create_date=\""+string(buf)+"\"";
            snprintf(buf, sizeof(buf) - 1, "%d", chat->historySize());
            chat_str += " msg_count=\""+string(buf)+"\"";
            chat_str += ">\n";

            // messages
            HistoryItems    hitems;
            HistoryItem_i   hitem_i;
            int             found = 0;

            hitems = chat->historyGet();
            for (hitem_i = hitems.begin();
                hitem_i != hitems.end();
                hitem_i++)
            {
                string msg_str = "<msg";
                snprintf(buf, sizeof(buf) - 1, "%d", hitem_i->id);
                msg_str += " id=\""+string(buf)+"\"";
                snprintf(buf, sizeof(buf) - 1, "%d", chat->getId());
                msg_str += " src_chatid=\""+string(buf)+"\"";
                msg_str += " msg_type=\""+hitem_i->msg_type+"\"";

                string message = base64_decode(hitem_i->msg);
                if (search_msg.size()
                    && message.find(search_msg) != string::npos)
                {
                    found = 1;
                    msg_str += " compared=\"1\"";
                }
                snprintf(buf, sizeof(buf) - 1, "%d", hitem_i->time);
                msg_str += " time=\""+string(buf)+"\"";
                msg_str += "><![CDATA["+hitem_i->msg+"]]></msg>\n";
                if (chat->getId() == chatid){
                    chat_str += msg_str;
                }
            }
            chat_str += "</chat>\n";
            if (search_msg.size() && !found){
            } else {
                answer += chat_str;
            }
            hist_i--;
        } while (hist_i != all_chats->begin());
    }
    answer += "          </chats>                            \n";
    answer += "  </event>                                    \n";
    answer += " </events>                                    \n";
    answer += "</ipnoise>                                    \n";

    add2out(out, answer);

ret:
    return err;
}

int API_synchistory(struct evbuffer *out, UserClass *cur_user,
    xmlNode *node)
{
    xmlNode *cur_node  = NULL;
    xmlNode *cur_node2 = NULL;
    const xmlChar *nodeName = node->name;
    char buf[TMP_BUF_SIZE];

    string huid             = "";
    int src_chatid          = 0;
    const char *huid_ptr    = NULL;
    const char *chatid_ptr  = NULL;

    huid_ptr   = (const char*)xmlGetProp(node, (xmlChar *)"huid");
    chatid_ptr = (const char*)xmlGetProp(node, (xmlChar *)"src_chatid");

    HistoryItems    hitems;
    HistoryItem_i   hitem_i;

    History::iterator   history_i_last;
    UserClass *dst_user = NULL;
    ChatClass *chat     = NULL;
    string answer       = "";

    int create_new_chat = 0;

    if (huid_ptr != NULL){
        huid = huid_ptr;
    }
    if (chatid_ptr != NULL){
        src_chatid = atoi(chatid_ptr);
    }

    dst_user = cur_user->getItem(huid);
    if (dst_user == NULL){
        PERROR("Sync history with unknown huid: '%s'\n",
            huid.c_str()
        );
        return 1;
    }

    PDEBUG(5, "SyncHistory with huid: '%s' user: 0x%x\n",
        huid.c_str(), dst_user);

    do {
        // new chat
        if (!src_chatid){
            create_new_chat = 1;
            break;
        }

        // search chat
        chat = dst_user->getHistory(src_chatid);
        if (chat == NULL
            || chat->getState() < ChatClass::CHAT_STATE_ACTIVE)
        {
            create_new_chat = 1;
            break;
        }
    } while (0);

    if (create_new_chat){
        chat = dst_user->getLastChat();
        if (chat != NULL){
            hitems = chat->historyGet();

            snprintf(buf, sizeof(buf) - 1, "Создан новый чат");
            string encoded = base64_encode(
                reinterpret_cast<const unsigned char*>(buf),
                strlen(buf)
            );
            history_item hitem  = chat->createHistItem();
            hitem.msg            = encoded;
            hitem.msg_type       = "system";
            hitems.push_back(hitem);

        }
    }

    if (hitems.size()){
        answer = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>     \n";
        answer += "<ipnoise>                                      \n";
        answer += "  <events>                                     \n";
        answer += "   <event type='sync-history' huid='"+huid+"'> \n";

        for (hitem_i = hitems.begin();
            hitem_i != hitems.end();
            hitem_i++)
        {
            answer += " <msg";
            snprintf(buf, sizeof(buf) - 1, "%d", hitem_i->id);
            answer += " id=\""+string(buf)+"\"";
            snprintf(buf, sizeof(buf) - 1, "%d", chat->getId());
            answer += " src_chatid=\""+string(buf)+"\"";
            answer += " msg_type=\""+hitem_i->msg_type+"\"";
            snprintf(buf, sizeof(buf) -1, "%d", hitem_i->time);
            answer += " time=\""+string(buf)+"\"";
            answer += "><![CDATA["+hitem_i->msg+"]]></msg>\n";
        }

        answer += "      </event>                              \n";
        answer += " </events>                                  \n";
        answer += "</ipnoise>                                  \n";
        add2out(out, answer);
    }
    return 0;
}

int dumpUsers (void)
{
    PDEBUG(7, "Dump users -------------------------------- {\n");

    Users::iterator user_i;

    for (user_i = USERS.begin();
        user_i != USERS.end();
        user_i++)
    {
        UserClass *user = user_i->second;
        PDEBUG(7, "HUID: '%s' (0x%x)\n",
            user->getHuid().c_str(), user);
        user->dump();
    }
    PDEBUG(7, "Dump users -------------------------------- }\n");
    return 0;
}

int API_whoaround(struct evbuffer *out, UserClass *cur_user,
    xmlNode *node)
{
    int err = 0;
    Users::iterator user_i;
    string answer   = "";
    string users    = "";

    for (user_i = USERS.begin();
        user_i != USERS.end();
        user_i++)
    {
        UserClass *user = user_i->second;
        users += user->getXml();
    }

    if (users.size()){
        answer = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>     \n";
        answer += "<ipnoise>                                      \n";
        answer += " <events>                                      \n";
        answer += "  <event type='whoaround'>\n";
        answer += users;
        answer += "  </event>   \n";
        answer += " </events>   \n";
        answer += "</ipnoise>   \n";
        add2out(out, answer);
    }

    return err;
};

int API_delmsg(struct evbuffer *out, UserClass *cur_user,
    xmlNode *node)
{
    int err = 0;
    char buf[TMP_BUF_SIZE];

    string  huid             = "";
    int     chat_id          = 0;
    int     msg_id           = 0;

    const char *huid_ptr     = NULL;
    const char *chat_id_ptr  = NULL;
    const char *msg_id_ptr   = NULL;

    UserClass *item          = NULL;
    string answer;

    huid_ptr = (const char*)xmlGetProp(node,
        (xmlChar *)"huid");

    chat_id_ptr = (const char*)xmlGetProp(node,
        (xmlChar *)"chat_id");

    msg_id_ptr = (const char*)xmlGetProp(node,
        (xmlChar *)"msg_id");

    if (huid_ptr == NULL){
        answer = "ERROR: missing 'huid'\n";
        add2out(out, answer);
        goto fail;
    }

    if (chat_id_ptr == NULL){
        answer = "ERROR: missing 'chat_id'\n";
        add2out(out, answer);
        goto fail;
    }

    if (msg_id_ptr == NULL){
        answer = "ERROR: missing 'msg_id'\n";
        add2out(out, answer);
        goto fail;
    }

    huid    = huid_ptr;
    chat_id = atoi(chat_id_ptr);
    msg_id  = atoi(msg_id_ptr);

    item = cur_user->getItem(huid);
    if (item == NULL){
        // item was not found, nothing to delete
        goto fail;
    }

    // remove message from chat
    item->delChatMsg(chat_id, msg_id);

    answer = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>         \n";
    answer += "<ipnoise>                                          \n";
    answer += " <events>                                          \n";
    answer += "  <event type='del-msg' huid='"+item->getHuid()+"'>\n";
    answer += "   <msg";
    answer += "     id=\""+string(msg_id_ptr)+"\"";
    answer += "     src_chatid=\""+string(chat_id_ptr)+"\">";
    answer += "   </msg>    \n";
    answer += "  </event>   \n";
    answer += " </events>   \n";
    answer += "</ipnoise>   \n";

    cur_user->send2AllUiSessions(answer);

ret:
    return err;
fail:
    err = 1;
    goto ret;
}

int API_delitem(struct evbuffer *out, UserClass *cur_user,
    xmlNode *node)
{
    int err = 0;

    string answer   = "";
    string huid     = "";
    string group_id = "";

    const char *huid_ptr     = NULL;
    const char *group_id_ptr = NULL;

    huid_ptr = (const char*)xmlGetProp(node,
        (xmlChar *)"huid");

    group_id_ptr = (const char*)xmlGetProp(node,
        (xmlChar *)"group_id");

    if (huid_ptr == NULL){
        answer = "ERROR: missing 'huid'\n";
        add2out(out, answer);
        goto fail;
    }

    huid = huid_ptr;

    if (group_id_ptr != NULL){
        group_id = group_id_ptr;
    }

    // TODO delete only from group if group_id specified
    cur_user->delItem(huid);

ret:
    return err;
fail:
    err = 1;
    goto ret;
}

int API_additem(struct evbuffer *out, UserClass *cur_user,
    xmlNode *node)
{
    int err = 0;
    UserClass  *user          = NULL;
    UserClass  *new_user      = NULL;
    ChatClass  *chat          = NULL;
    const char *huid_ptr      = NULL;
    const char *group_id_ptr  = NULL;
    const char *name_ptr      = NULL;
    const char *descr_ptr     = NULL;

    string answer   = "";
    string huid     = "";
    string group_id = "";
    string name     = "";
    string descr    = "";

    huid_ptr = (const char*)xmlGetProp(node,
        (xmlChar *)"huid");
    group_id_ptr = (const char*)xmlGetProp(node,
        (xmlChar *)"group_id");
    name_ptr  = (const char*)xmlGetProp(node,
        (xmlChar *)"name");
    descr_ptr  = (const char*)xmlGetProp(node,
        (xmlChar *)"descr");

    if (group_id_ptr != NULL){
        group_id = group_id_ptr;
    }

    if (huid_ptr != NULL){
        huid = huid_ptr;
    }

    if (huid.size()){
        // it is "add user" case
        if (!group_id.size()){
            group_id = "0";
        }
        user = cur_user->getItem(huid);
        if (user != NULL){
            // item already exist
            goto ret;
        }
        // search user
        user = UserClass::getUser(huid);
        if (user == NULL){
            // unknown user
            answer = "ERROR: unknown user: '"+huid+"'\n";
            add2out(out, answer);
            goto fail;
        }
        new_user = new UserClass(huid, "");
        if (new_user == NULL){
            // internal error
            answer = "ERROR: new UserClass() failed for huid:"
            " '"+huid+"'\n";
            add2out(out, answer);
            goto fail;
        }
        new_user->setLogin(user->getLogin());
        new_user->setHuid(user->getHuid());
        new_user->setGroup(group_id);
        new_user->setStatus(user->getStatus());
        new_user->setAvatar(user->getAvatar());
        new_user->setDescr(user->getDescr());
        cur_user->addItem(new_user);
        goto ret;
    }

    // it is "add note" case
    if (!group_id.size()){
        answer = "ERROR: missing 'group_id'\n";
        add2out(out, answer);
        goto fail;
    }

    if (name_ptr == NULL){
        answer = "ERROR: missing 'name'\n";
        add2out(out, answer);
        goto fail;
   }

    name  = base64_decode(string(name_ptr));
    descr = base64_decode(string(descr_ptr));

    if (!name.size()){
        answer = "ERROR: missing 'name'\n";
        add2out(out, answer);
        goto fail;
    }

    PDEBUG(5, "Request create Item: group_id: '%s',"
        " name: '%s', descr: '%s'\n",
        group_id.c_str(),
        name.c_str(),
        descr.c_str()
    );

    // generate huid
    huid = cur_user->generateFreeHuid();

    if (!huid.size()){
        answer = "ERROR: (internal) cannot create huid\n";
        add2out(out, answer);
        PERROR("%s", answer.c_str());
        goto fail;
    }

    user = new UserClass(huid, name);
    if (user == NULL){
        answer = "ERROR: (internal) cannot create user class\n";
        add2out(out, answer);
        PERROR("%s\n", answer.c_str());
        goto fail;
    }
    user->setDescr(descr);
    user->setGroup(group_id);
    cur_user->addItem(user);

    chat = new ChatClass();
    if (chat == NULL){
        PERROR("[OUTPUT] Cannot create ChatClass()\n");
        goto fail;
    }
    // заметки всегда имеют один чат (chatid = 0)
    chat->setId(0);
    user->addHistory(chat);

ret:
    return err;
fail:
    err = 1;
    goto ret;
}

int INPUT(UserClass *cur_user, xmlNode *node)
{
    int err = 0;
    xmlNode *cur_node       = NULL;
    xmlNode *cur_node2      = NULL;
    const xmlChar *nodeName = node->name;
    char buf[TMP_BUF_SIZE];

    string msg = "";

    string src_huid             = "";
    int src_chatid              = 0;
    int dst_chatid              = 0;
    const char *src_chatid_ptr  = NULL;
    const char *dst_chatid_ptr  = NULL;
    const char *src_huid_ptr    = NULL;
    ChatClass  *chat            = NULL;
 
    history_item hitem;
    history_item hitem_ev;

    src_huid_ptr    = (const char*)xmlGetProp(node,
        (xmlChar *)"srchuid");
    src_chatid_ptr  = (const char*)xmlGetProp(node,
        (xmlChar *)"src_chatid");
    dst_chatid_ptr  = (const char*)xmlGetProp(node,
        (xmlChar *)"dst_chatid");

    if (src_huid_ptr != NULL){
        src_huid = src_huid_ptr;
    }
    if (src_chatid_ptr != NULL){
        src_chatid = atoi(src_chatid_ptr);
    }
    if (dst_chatid_ptr != NULL){
        dst_chatid = atoi(dst_chatid_ptr);
    }

    for (cur_node = node->children;
        cur_node;
        cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE) {
            nodeName = cur_node->name;
            if (string((const char*)nodeName) == string("msg")){
                for (cur_node2 = cur_node->children;
                     cur_node2;
                     cur_node2 = cur_node2->next)
                {
                    if (cur_node2->type == XML_CDATA_SECTION_NODE){
                        msg += (const char *)xmlNodeGetContent(
                            cur_node2);
                    }
                }
                break;
            }
        }
    }

    PDEBUG(5, "[INPUT] Получен пакет, цепочка INPUT\n"
        "src_huid:      %s\n"
        "src_chat_id:   %d\n"
        "dst_chat_id:   %d\n"
        "\n",
        src_huid.c_str(),
        src_chatid,
        dst_chatid
    );

    string src_login    = "unknown source user";
    int create_new_chat = 0;
    UserClass *src_user = NULL;

    PDEBUG(5, "[INPUT] Получение сообщения от пользователя: '%s'\n",
        src_huid.c_str());

    // search src user login name
    src_user = UserClass::getUser(src_huid);
    if (src_user != NULL){
        src_login = src_user->getLogin();
    }

    // search src user in our history
    src_user = cur_user->getItem(src_huid);
    if (src_user == NULL){
        src_user = new UserClass(
            src_huid,
            src_login
        );
        cur_user->addItem(src_user);
    }

    PDEBUG(5, "[INPUT] Чат получателя: '%d'\n", src_chatid);

    do {
        // new chat
        if (!src_chatid){
            create_new_chat = 1;
            break;
        }

        // search chat
        chat = src_user->getChat(src_chatid);
        // if chat found check state
        if (chat == NULL
            || chat->getState() < ChatClass::CHAT_STATE_ACTIVE)
        {
            create_new_chat = 1;
            PDEBUG(5, "[INPUT] Чат получателя: '%d' не найден или протух\n",
                src_chatid);
            break;
        }
    } while (0);

    if (create_new_chat && !msg.size()){
        // dont receive any events if chat not exist
        PDEBUG(5, "[INPUT] Отправка события в тухлый чат: '%d',"
            " событие будет убито\n",
            src_chatid);
        goto drop;
    }

    if (!msg.size()){
        // it's event
        PDEBUG(5, "[INPUT] Обрабатываем событие\n");
        goto process_event;
    }

    if (create_new_chat){
        // create chat ID
        src_chatid = time(NULL);
        PDEBUG(5, "[INPUT] Создаем чат: '%d'\n", src_chatid);
        chat = new ChatClass();
        chat->setState(ChatClass::CHAT_STATE_ACTIVE);
        chat->setCreateDate(time(NULL));
        chat->setId(src_chatid);
        src_user->addHistory(chat);
   }

    if (chat == NULL){
        PDEBUG(5, "[INPUT] Чат не найден и не создан,"
            "убиваем пакет\n");
        goto drop;
    }

    // create history item
    hitem       = chat->createHistItem();
    hitem.msg   = msg;

    // store message in destination user's history
    hitem.msg_type = "incoming";

    PDEBUG(5, "[INPUT] Добавляем сообщение в чат\n");
    chat->historyAdd(hitem);

    // inform client what message was received
    PDEBUG(5, "[INPUT] Информируем получателя: '%s',"
        " что сообщение принято\n",
        cur_user->getHuid().c_str()
    );
    cur_user->historyAdd(src_huid, hitem,
        src_chatid, dst_chatid
    );

    // подтверждение доставки
    hitem.msg       = "";
    hitem.msg_type  = "delivered";
    src_user        = UserClass::getUser(src_huid);

    if (src_user != NULL){
        PDEBUG(5, "[INPUT] Информируем отправителя: '%s',"
            " что сообщение доставлено\n",
            src_huid.c_str()
        );
        src_user->historyAdd(cur_user->getHuid().c_str(),
            hitem,
            dst_chatid, src_chatid
        );
    }

ret:
    PDEBUG(5, "[INPUT] Обработка пакета завершена\n");
    // set status
    cur_user->setStatus((char *)"images/iconarr_green.gif");
    return err;

process_event:
    // it is typeing event
    hitem_ev.msg_type  = "typeing";
    hitem_ev.msg       = "";
    hitem_ev.time      = time(NULL);

    cur_user->historyAdd(src_huid, hitem_ev,
        src_chatid, dst_chatid
    );
    goto ret;

drop:
    PDEBUG(5, "[INPUT] Пакет убит\n");
    goto ret;

    /*

    if (!dst_user->sessions.size()){
        // non online
        string msg = "В данный момент пользователя нет в системе, но как только он появится сообщение будет доставлено";
        string encoded = base64_encode(
            reinterpret_cast<const unsigned char*>(msg.c_str()),
            msg.length()
        );
        snprintf(buf, sizeof(buf) -1, "%d", time(NULL));

        string answer = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
        answer += "<ipnoise>                                      \n";
        answer += "<events>                                       \n";
        answer += "<event type='update-history' huid='"+dst_user->huid+"'>\n";
        answer += "<msg msg_type=\"system\"";
        answer += " time=\""+string(buf)+"\"";
        snprintf(buf, sizeof(buf) - 1, "%d", src_chatid);
        answer += " chatid=\""+string(buf)+"\"";
        answer += ">"+encoded+"</msg>\n";
        answer += "</event>                                       \n";
        answer += " </events>                                     \n";
        answer += "</ipnoise>                                     \n";
        cur_user->send2AllUiSessions(answer);
    }
    */
}

int FORWARD (string src_huid, xmlNode *node)
{
    int err = 0;
    xmlNode *cur_node       = NULL;
    xmlNode *cur_node2      = NULL;
    const xmlChar *nodeName = node->name;
    char buf[TMP_BUF_SIZE];

    string dst_huid             = "";
    int src_chatid              = 0;
    int dst_chatid              = 0;

    UserClass  *cur_user        = NULL;
    const char *src_chatid_ptr  = NULL;
    const char *dst_chatid_ptr  = NULL;
    const char *dst_huid_ptr    = NULL;
    ChatClass  *chat            = NULL;

    string msg      = "";
    dst_huid_ptr    = (const char*)xmlGetProp(node,
        (xmlChar *)"dsthuid");
    src_chatid_ptr  = (const char*)xmlGetProp(node,
        (xmlChar *)"src_chatid");
    dst_chatid_ptr  = (const char*)xmlGetProp(node,
        (xmlChar *)"dst_chatid");

    if (dst_huid_ptr != NULL){
        dst_huid = dst_huid_ptr;
    }
    if (src_chatid_ptr != NULL){
        src_chatid = atoi(src_chatid_ptr);
    }
    if (dst_chatid_ptr != NULL){
        dst_chatid = atoi(dst_chatid_ptr);
    }

    for (cur_node = node->children;
         cur_node;
         cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE){
            nodeName = cur_node->name;
            if (string((const char*)nodeName) == string("msg")){
                for (cur_node2 = cur_node->children;
                     cur_node2;
                     cur_node2 = cur_node2->next)
                {
                    if (cur_node2->type == XML_CDATA_SECTION_NODE){
                        msg += (const char *)xmlNodeGetContent(
                            cur_node2);
                    }
                }
                break;
            }
        }
    }

    PDEBUG(5, "[FORWARD] Получен пакет, цепочка FORWARD\n"
        "dst_huid:      %s\n"
        "src_chat_id:   %d\n"
        "dst_chat_id:   %d\n"
        "\n",
        dst_huid.c_str(),
        src_chatid,
        dst_chatid
    );

    // search destination user or create new
    cur_user = UserClass::getUser(dst_huid);
    if (cur_user == NULL){
        // maybe offline
        cur_user = new UserClass(dst_huid, "unknown source user");
        cur_user->addItem(cur_user);
        USERS[dst_huid] = cur_user;
    }

    snprintf(buf, sizeof(buf) - 1, "%d", src_chatid);
    xmlSetProp(node, (xmlChar *)"dst_chatid", (xmlChar *)buf);

    snprintf(buf, sizeof(buf) - 1, "%d", dst_chatid);
    xmlSetProp(node, (xmlChar *)"src_chatid", (xmlChar *)buf);

    snprintf(buf, sizeof(buf) - 1, "%s", src_huid.c_str());
    xmlSetProp(node, (xmlChar *)"srchuid", (xmlChar *)buf);

    PDEBUG(5, "[FORWARD] Обработка пакета завершена\n");
    err = INPUT(cur_user, node);

    return err;
}

int API_sendmsg(struct evbuffer *out, UserClass *cur_user,
    xmlNode *node)
{
    int err = 0;
    xmlNode *cur_node       = NULL;
    xmlNode *cur_node2      = NULL;
    const xmlChar *nodeName = node->name;
    char buf[TMP_BUF_SIZE];

    string dst_huid             = "";
    int src_chatid              = 0;
    int dst_chatid              = 0;
    const char *src_chatid_ptr  = NULL;
    const char *dst_chatid_ptr  = NULL;
    const char *dst_huid_ptr    = NULL;
    ChatClass  *chat            = NULL;

    history_item hitem;

    UserClass *dst_user = NULL;
    int create_new_chat = 0;

    string msg          = "";
    string msg_decoded  = "";

    dst_huid_ptr    = (const char*)xmlGetProp(node,
        (xmlChar *)"dsthuid");
    src_chatid_ptr  = (const char*)xmlGetProp(node,
        (xmlChar *)"src_chatid");
    dst_chatid_ptr  = (const char*)xmlGetProp(node,
        (xmlChar *)"dst_chatid");

    if (dst_huid_ptr != NULL){
        dst_huid = dst_huid_ptr;
    }
    if (src_chatid_ptr != NULL){
        src_chatid = atoi(src_chatid_ptr);
    }
    if (dst_chatid_ptr != NULL){
        dst_chatid = atoi(dst_chatid_ptr);
    }

    for (cur_node = node->children;
         cur_node;
         cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE){
            nodeName = cur_node->name;
            if (string((const char*)nodeName) == string("msg")){
                for (cur_node2 = cur_node->children;
                     cur_node2;
                     cur_node2 = cur_node2->next)
                {
                    if (cur_node2->type == XML_CDATA_SECTION_NODE){
                        msg = (const char *)xmlNodeGetContent(
                            cur_node2);
                        break;
                    }
                }
                break;
            }
        }
    }

    PDEBUG(5, "[OUTPUT] Получен пакет, цепочка OUTPUT\n"
        "dst_huid:      %s\n"
        "src_chat_id:   %d\n"
        "dst_chat_id:   %d\n"
        "\n",
        dst_huid.c_str(),
        src_chatid,
        dst_chatid
    );

    PDEBUG(5, "[OUTPUT] Отправка сообщения для пользователя: '%s'\n",
        dst_huid.c_str());

    dst_user = cur_user->getItem(dst_huid);
    if (dst_user == NULL){
        PERROR("[OUTPUT] User with huid: '%s' was not found\n",
                dst_huid.c_str()
        );
        goto drop;
    }

    PDEBUG(5, "[OUTPUT] Чат отправителя: '%d'\n", src_chatid);

    do {
        if (!src_chatid){
            create_new_chat = 1;
            break;
        }

        // search chat
        chat = dst_user->getHistory(src_chatid);
        if (chat == NULL
            || chat->getState() < ChatClass::CHAT_STATE_ACTIVE)
        {
            create_new_chat = 1;
            PDEBUG(5, "[OUTPUT] Чат отправителя: '%d' не найден"
                " или протух\n",
                src_chatid);
            break;
        }
    } while (0);


    if (create_new_chat && !msg.size()){
        // dont receive any events if chat not exist
        PDEBUG(5, "[OUTPUT] Отправка события в тухлый чат: '%d',"
            " событие будет убито\n",
            src_chatid);
        goto drop;
    }

    if (!msg.size()){
        // it's event
        PDEBUG(5, "[OUTPUT] Обрабатываем событие\n");
        goto process_event;
    }

    if (dst_user->getGroup() == (char *)"2212"){
        // группа "заметки" у них всегда один чат с ID = 0
        PDEBUG(5, "[OUTPUT] Получено сообщение для группы 'заметки'\n");
        src_chatid = 0;
        chat = dst_user->getHistory(src_chatid);
        if (chat == NULL){
            PDEBUG(5, "[OUTPUT] Создаем чат: '%d'"
                " для группы 'заметки'\n",
                src_chatid); 
            chat = new ChatClass();
            if (chat == NULL){
                PERROR("[OUTPUT] Cannot create ChatClass()\n");
                goto fail;
            }
            chat->setId(src_chatid);
            dst_user->addHistory(chat);
        }
        // create history item
        hitem               = chat->createHistItem();
        hitem.msg           = msg;
        hitem.msg_type      = "outcoming";

        chat->setState(ChatClass::CHAT_STATE_ACTIVE);
        chat->setCreateDate(time(NULL));
        chat->historyAdd(hitem);
        goto process_local;
    }

    if (create_new_chat){
        // create chat ID
        src_chatid = time(NULL);
        PDEBUG(5, "[OUTPUT] Создаем чат: '%d'\n", src_chatid);
        chat = new ChatClass();
        chat->setState(ChatClass::CHAT_STATE_ACTIVE);
        chat->setCreateDate(time(NULL));
        chat->setId(src_chatid);
        dst_user->addHistory(chat);

        snprintf(buf, sizeof(buf) - 1, "%d", src_chatid);
        xmlSetProp(node, (xmlChar *)"src_chatid", (xmlChar *)buf);
    }

    if (chat == NULL){
        PDEBUG(5, "[OUTPUT] Чат не найден и не создан,"
            " убиваем пакет\n");
        goto drop;
    }

    PDEBUG(5, "MSG: '%s'\n", msg.c_str());
    msg_decoded = base64_decode(msg);

    // check commands
    if (msg_decoded == "/help"){
        // create history item
        hitem               = chat->createHistItem();
        hitem.msg           = msg;
        hitem.msg_type      = "outcoming";
        cur_user->historyAdd(dst_huid, hitem,
            src_chatid, dst_chatid
        );
        // process help
        string help  = "";
        help        += "<table style=\"font-size: small;\">";
        help        += "<tr><td>Command</td><td>Action</td></tr>";
        help        += "<tr><td>/me   message   </td><td>send message from 3rd person</td></tr>";
        help        += "<tr><td>/href url [text]</td><td>insert link to url with text</td></tr>";
        help        += "<tr><td>/img  url       </td><td>insert image with from url</td></tr>"; 
        help        += "<tr><td>/pre  message   </td><td>insert unformated message</td></tr>"; 
        help        += "</table>";

        string encoded = base64_encode(
            reinterpret_cast<const unsigned char*>(help.c_str()),
            help.length()
        );

        // create history item
        hitem               = chat->createHistItem();
        hitem.msg           = encoded;
        hitem.msg_type      = "system";
        cur_user->historyAdd(dst_huid, hitem,
            src_chatid, dst_chatid
        );
        goto ret;
    } else if (msg_decoded.substr(0, 3) == "/me"){
         string new_msg = "<small><i>*"+msg_decoded.substr(4)+"*</i></small>";
         string encoded = base64_encode(
            reinterpret_cast<const unsigned char*>(new_msg.c_str()),
            new_msg.length()
        );
        msg = encoded;
    } else if (msg_decoded.substr(0, 5) == "/href"){
         string href = msg_decoded.substr(6);
         string text = href;
         size_t pos  = href.find(" ");
         if (pos != string::npos){
            text = href.substr(pos);
            href = href.substr(0, pos);
         }
         string new_msg = "<a target=\"_blank\" href=\""+href+"\">"+text+"</a>";
         string encoded = base64_encode(
            reinterpret_cast<const unsigned char*>(new_msg.c_str()),
            new_msg.length()
        );
        msg = encoded;
    } else if (msg_decoded.substr(0, 4) == "/img"){
         string src  = msg_decoded.substr(5);
         string new_msg  = "<a target=\"_blank\" href=\""+src+"\">";
         new_msg        += "<img src=\""+src+"\" width=\"300\"></img>";
         new_msg        += "</a>";
         string encoded = base64_encode(
            reinterpret_cast<const unsigned char*>(new_msg.c_str()),
            new_msg.length()
        );
        msg = encoded;
    } else if (msg_decoded.substr(0, 4) == "/pre"){
         string text  = msg_decoded.substr(5);
         string new_msg  = "<pre>";
         new_msg        += text;
         new_msg        += "</pre>";
         string encoded = base64_encode(
            reinterpret_cast<const unsigned char*>(new_msg.c_str()),
            new_msg.length()
        );
        msg = encoded;
    }

    PDEBUG(5, "[OUTPUT] Добавляем сообщение в чат\n");

    // create history item
    hitem           = chat->createHistItem();
    hitem.msg       = msg;
    hitem.msg_type  = "outcoming";
    chat->historyAdd(hitem);

    // inform client what message was send
    PDEBUG(5, "[OUTPUT] Информируем отправителя: '%s',"
        " что сообщение принято к отправке\n",
        cur_user->getHuid().c_str()
    );
    cur_user->historyAdd(dst_huid, hitem,
        src_chatid, dst_chatid
    );

    // put changed message back to packet
    for (cur_node = node->children;
         cur_node;
         cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE){
            nodeName = cur_node->name;
            if (string((const char*)nodeName) == string("msg")){
                for (cur_node2 = cur_node->children;
                     cur_node2;
                     cur_node2 = cur_node2->next)
                {
                    if (cur_node2->type == XML_CDATA_SECTION_NODE){
                        xmlNodeSetContent(cur_node2, (const xmlChar*)msg.c_str());
                        break;
                    }
                }
                break;
            }
        }
    }



process_event:
    // going to receive
    PDEBUG(5, "[OUTPUT] Пакет передается в цепочку FORWARD\n");
    PDEBUG(5, "[OUTPUT] Обработка пакета завершена\n");
    FORWARD(cur_user->getHuid(), node);

ret:
    return err;

drop:
    PDEBUG(5, "[OUTPUT] Пакет убит\n");
    PDEBUG(5, "[OUTPUT] Обработка пакета завершена\n");
    goto ret;

process_local:
    PDEBUG(5, "[OUTPUT] Пакет принят как локальный\n");
    PDEBUG(5, "[OUTPUT] Информируем отправителя: '%s',"
        " что сообщение принято к отправке\n",
        cur_user->getHuid().c_str()
    );
    cur_user->historyAdd(dst_huid, hitem,
        src_chatid, dst_chatid
    );
    PDEBUG(5, "[OUTPUT] Обработка пакета завершена\n");
    goto ret;

fail:
    err = 1;
    PDEBUG(5, "[OUTPUT] Произошла внутренняя ошибка\n");
    PDEBUG(5, "[OUTPUT] Обработка пакета завершена\n");
    goto drop;
}

int parseCommand(struct evbuffer *out, struct evhttp_request *req,
    string sess_id, xmlNode *node)
{
    // receive "command" node and parse it
    string answer           = "";
    UserClass    *cur_user  = NULL;
    uiSessionClass *sess      = NULL;
    UiSessions::iterator sessions_i;

    const xmlChar *nodeName = node->name;

    if (string((const char*)nodeName) != string("command")){
        answer = "Invalid node name: "
            "'"+string((char *)nodeName)+"'"
            " (must be 'command')\n";
        add2out(out, answer);
        return -1;
    }

    xmlChar *cmd = xmlGetProp(node, (xmlChar *)"type");

    if (string((const char*)cmd) == "login"){
        API_login(out, req, node);
        goto ret;
    }

    // all another commands must be only for logined clients
    cur_user = UserClass::searchUser(sess_id);
    if (cur_user == NULL){
        goto unauth;
    }

    if (string((const char*)cmd) == "sendmsg"){
        API_sendmsg(out, cur_user, node);
    } else if (string((const char*)cmd) == "sync-history"){
        API_synchistory(out, cur_user, node);
    } else if (string((const char*)cmd) == "search-history"){
        API_searchhistory(out, cur_user, node);
    } else if (string((const char*)cmd) == "add-item"){
        API_additem(out, cur_user, node);
    } else if (string((const char*)cmd) == "del-item"){
        API_delitem(out, cur_user, node);
    } else if (string((const char*)cmd) == "del-msg"){
        API_delmsg(out, cur_user, node);
    } else if (string((const char*)cmd) == "whoaround"){
        API_whoaround(out, cur_user, node);
    } else {
        answer = "ERROR: Unknown API command type: "
            "'"+string((char *)cmd)+"'\n";
        add2out(out, answer);
    }

ret:
    xmlFree(cmd);
    return 0;
unauth:
    answer = "Authentication required!\n";
    add2out(out, answer);
    goto ret;
}

void closeSession (struct evhttp_connection *evcon, void *args)
{
    uiSessionClass *session = (uiSessionClass *)args;
    UserClass::closeSession(session);
}

int parseCommands(struct evbuffer *out, struct evhttp_request *req,
    string sess_id, xmlNode *node)
{
    // receive "commands" node and parse it
    xmlNode *cur_node = NULL;
    const xmlChar *nodeName = node->name;

    if (string((const char*)nodeName) != string("commands")){
        PERROR("Invalid node name: '%s' (must be 'commands')\n",
            nodeName
        );
        return -1;
    }

    for (cur_node = node->children;
        cur_node;
        cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE) {
            nodeName = cur_node->name;
            if (string((const char*)nodeName) == string("command")){
                parseCommand(out, req, sess_id, cur_node);
            }
        }
    }

}

int s_Events(struct evbuffer *out, struct evhttp_request *req,
    struct parts *pt, void *arg, struct myEnviroment &env)
{
    struct evkeyvalq    params;
    char                *sess_id_ptr;
    string              sess_id;
    UserClass           *cur_user = NULL;
    uiSessionClass      *sess = NULL;

    struct evbuffer *evb = NULL;

    evhttp_parse_query(req->uri, &params);
    sess_id_ptr = (char *)evhttp_find_header(&params,  "sess_id");
    if (sess_id_ptr){
        sess_id  = sess_id_ptr;
        cur_user = UserClass::searchUser(sess_id);
    }
    evhttp_clear_headers(&params);

    if (cur_user == NULL){
        goto unauth;
    }

    sess = cur_user->getSessById(sess_id);
    if (sess == NULL){
        goto unauth;
    }

    evhttp_connection_set_closecb(req->evcon, closeSession, sess);

    // store signal connection
    sess->setReq(req);

    evb = evbuffer_new();
    evbuffer_add_printf(evb,
        "HTTP/1.1 200 OK\r\n"
//        "Connection: close\r\n"
        "Pragma: no-cache\r\n"
        "Cache-Control: no-cache, must-revalidate\r\n"
        "Expires: Sat, 26 Jul 1997 05:00:00 GMT\r\n"   // date in past
        "Content-Length: 1000000\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "\r\n"
    );
    sess->sendEvbuff(evb);
    evbuffer_free(evb);

ret:
    return 0;
unauth:
//    evhttp_add_header(req->output_headers, "Connection",        "close");
    evhttp_add_header(req->output_headers, "Pragma",            "no-cache");
    evhttp_add_header(req->output_headers, "Cache-Control",     "no-cache, must-revalidate");
    evhttp_add_header(req->output_headers, "Expires",           "Sat, 26 Jul 1997 05:00:00 GMT");
    evhttp_add_header(req->output_headers, "Content-Type",      "text/html; charset=utf-8");
    evhttp_send_reply(req, 500, "Authentication required!", NULL);
    goto ret;
}

int s_API(struct evbuffer *out, struct evhttp_request *req,
    struct parts *pt, void *arg, struct myEnviroment &env)
{
    char *content   = (char *)EVBUFFER_DATA(req->input_buffer);
    int length      = EVBUFFER_LENGTH(req->input_buffer);

    xmlNode *cur_node = NULL;
    xmlNodePtr      node;
    xmlDocPtr       doc;
    const xmlChar   *nodeName;

    xmlChar         *cmd;
    string          sess_id;
    string          answer = "";

    doc = xmlReadMemory(content, length, "noname.xml", NULL, 0);
    if (doc == NULL) {
        goto failed;
    }

    node = xmlDocGetRootElement(doc);
    nodeName = node->name;

    if (string((char *)nodeName) != string("ipnoise")){
        answer = "ERROR: Root node is not 'ipnoise'\n";
        add2out(out, answer);
        goto failed;
    }

    // get sess id
    cmd = xmlGetProp(node, (xmlChar *)"sess_id");
    if (cmd != NULL){
        sess_id = (const char*)cmd;
    }

    for (cur_node = node->children;
        cur_node;
        cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE) {
            nodeName = cur_node->name;
            if (string((const char*)nodeName) == string("commands")){
                parseCommands(out, req, sess_id, cur_node);
            }
        }
    }

    xmlFreeDoc(doc);

ret:
//    evhttp_add_header(req->output_headers, "Connection",        "close");
    evhttp_add_header(req->output_headers, "Content-Type",      "text/html; charset=utf-8");
    evhttp_add_header(req->output_headers, "Pragma",            "no-cache");
    evhttp_add_header(req->output_headers, "Cache-Control",     "no-cache, must-revalidate");
    evhttp_add_header(req->output_headers, "Expires",           "Sat, 26 Jul 1997 05:00:00 GMT");
    evhttp_send_reply(req, HTTP_OK, "", out);
    return 0;

failed:
    answer = "ERROR: Cannot parse XML:\n"+string(content)+"\n";
    add2out(out, answer);
    goto ret;
}

int s_file(struct evbuffer *out, struct evhttp_request *req,
    struct parts *pt, void *arg, struct myEnviroment &env)
{
    int res;
    string content_type;
    string path = env.conf.root_dir+"files/";

    char    *real_path  = NULL;
    FILE    *file       = NULL;
    struct  stat st;
    size_t  tmp;

    char   buf[TMP_BUF_SIZE];
    struct evbuffer *evb = NULL;
    string answer;

    if (strlen(req->uri) == 0
        || strcmp(req->uri, "/") == 0)
    {
        path += "/index.html";
    } else {
        // add path from uri
        path += req->uri;
    }

    real_path = realpath((char *)path.c_str(), NULL);
    if (real_path == NULL){
        PERROR("Path: '%s' not exist or not available for read\n",
            path.c_str());
        goto ret_404;
    }

    res = stat(path.c_str(), &st);
    if (res){
        PERROR("Path: '%s' not exist or not available for read\n",
            path.c_str());
        goto ret_404;
    }

    res = stat(real_path, &st);
    if (res){
        PDEBUG(5, "Path: '%s' not exist or not available for read\n",
            real_path);
        goto ret_404;
    }

    if (!S_ISREG(st.st_mode)){
        // it's not regular file
        goto ret_404;
    }

    // try to get content type
    content_type = getContentType(real_path);

    file = fopen(real_path, "rb");
    if (file == NULL){
        PERROR("Cannot open file for read: '%s'\n", real_path);
        goto ret_500;
    }

    PDEBUG(5, "Requested file: '%s'\n", real_path);

    // force to be closed
//    evhttp_add_header(req->output_headers, "Pragma",        "no-cache");
//    evhttp_add_header(req->output_headers, "Connection",    "close"); 
//    evhttp_add_header(req->output_headers, "Cache-Control", "no-cache, must-revalidate");
//    evhttp_add_header(req->output_headers, "Expires",       "Sat, 26 Jul 1997 05:00:00 GMT");  // date in past
    evhttp_add_header(req->output_headers, "Content-Type",  content_type.c_str());
    snprintf(buf, sizeof(buf) - 1, "%d", st.st_size);
    evhttp_add_header(req->output_headers, "Content-Length", buf);
    evhttp_send_reply_start(req, HTTP_OK, "");

    do {
        tmp = fread(file_buffer, sizeof(unsigned char), FILE_BUFFER_SIZE, file);
        PDEBUG(5, "Was read: '%d' bytes\n", tmp);
        if (tmp > 0){
            // write content to request
            evb = evbuffer_new();
            evbuffer_add(evb, file_buffer, tmp);
            evhttp_send_reply_chunk(req, evb);
            evbuffer_free(evb);
        }
    } while (tmp > 0);

    evhttp_send_reply_end(req);

ret:
    if (real_path != NULL){
        free (real_path);
    }
    if (file != NULL){
        fclose(file);
    }
    return 0;

ret_404:
    res = 404;
    answer = (char *)"404 File was not found\n";
    add2out(out, answer);
//    evhttp_add_header(req->output_headers, "Connection",    "close");
    evhttp_send_reply(req, res, "", out);
    goto ret;

ret_500:
    res = 500;
    answer = (char *)"500 Internal error\n";
    add2out(out, answer);
//    evhttp_add_header(req->output_headers, "Connection",    "close");
    evhttp_send_reply(req, res, "", out);
    goto ret;
}


