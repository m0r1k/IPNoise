#include "chat.h"
#include "shttp.h"
#include "parts.h"

history_item ChatClass::createHistItem()
{
    history_item hitem;
    hitem.id    = next_msg_id++; 
    hitem.time  = time(NULL);
    return hitem;
}

HistoryItems ChatClass::historyGet()
{
    HistoryItems ret;
    HistoryItems::iterator history_i;
    for (history_i  = history.begin();
         history_i != history.end();
         history_i++)
    {
        history_item &item = *history_i;
        if (item.msg.size()){
            ret.push_back(item);
        }
    }
    return ret;
};

int ChatClass::historyAdd(history_item &hitem)
{
    history.push_back(hitem);
};

void ChatClass::historyDel(int msg_id)
{
    HistoryItems::iterator history_i;
    for (history_i  = history.begin();
         history_i != history.end();
         history_i++)
    {
        history_item &item = *history_i;
        if (item.id == msg_id){
            item.msg = "";
        }
    }
};

int ChatClass::historySize()
{
    int count = 0;
    HistoryItems::iterator history_i;
    for (history_i  = history.begin();
         history_i != history.end();
         history_i++)
    {
        history_item &item = *history_i;
        if (item.msg.size()){
            count++;
        }
    }
    return count;
};

int ChatClass::save(string root_dir)
{
    char buffer[128];
    FILE *out = NULL;
    int res, err;

    string chat_dir = root_dir + string("/chats/");
    mkdir((char *)chat_dir.c_str(), NEW_DIRS_MODE);

    snprintf(buffer, sizeof(buffer) - 1, "%d", chat_id);
    chat_dir += buffer;
    mkdir((char *)chat_dir.c_str(), NEW_DIRS_MODE);

    string filename = chat_dir + string("/history.dat");
    string fname    = chat_dir + string("/history.dat.new");

    err = 0;
    do {
        out = fopen(fname.c_str(), "w+");
        if (out == NULL){
            PERROR("Cannot open file for write: '%s'\n",
                fname.c_str());
            err = 1;
            break;
        }

        HistoryItems hist_items = historyGet();
        HistoryItems::iterator hist_item_i;

        res = fprintf(out,
            "# FILE_VERSION=1\n"
        );
        if (res <= 0){
            PERROR("fprintf() failed\n");
            err = 1;
            break;
        }
        res = fprintf(out,
            "# ID\tTIME\tDIRECTION\tMESSAGE\n"
        );
        if (res <= 0){
            PERROR("fprintf() failed\n");
            err = 1;
            break;
        }

        for (hist_item_i = hist_items.begin();
            hist_item_i != hist_items.end();
            hist_item_i++)
        {
            history_item &v = *hist_item_i;
            res = fprintf(out,
                "%d\t%d\t%s\t%s\n",
                v.id,
                v.time,
                v.msg_type.c_str(),
                v.msg.c_str()
            );
            if (res <= 0){
                PERROR("fprintf() failed\n");
                err = 1;
                break;
            }
        }
        if (err){
            break;
        }
    } while (0);

    if (out != NULL){
        res = fclose(out);
        if (res){
            err = 1;
        }
    }

    PDEBUG(5, "SAVE HISTORY FILE: ERR: '%d'\n", err);
    if (!err){
        rename(fname.c_str(), filename.c_str());
    }
    unlink(fname.c_str());
    return err;
}

void ChatClass::dump()
{
    HistoryItems::iterator hist_item_i;
    for (hist_item_i = history.begin();
        hist_item_i != history.end();
        hist_item_i++)
    {
        history_item &v = *hist_item_i;
        if (!v.msg.size()){
            continue;
        }
        string answer = "\t\t\t";
        if (v.msg_type == "incoming"){
            answer += " <<";
        } else if (v.msg_type == "outcoming"){
            answer += " >>";
        } else {
            answer += " ??";
        }
        answer += " "+v.msg;
        PDEBUG(7, "%s\n", answer.c_str());
    }
}

int ChatClass::getId()
{
    return chat_id;
};

void ChatClass::setId(int new_id)
{
    chat_id = new_id;
};

ChatClass::States ChatClass::getState()
{
    return state;
};

void ChatClass::setState(ChatClass::States new_state)
{
    state = new_state;
};

time_t ChatClass::getCreateDate()
{
    return create_date;
}

void ChatClass::setCreateDate(time_t new_create_date)
{
    create_date = new_create_date;
}

ChatClass::ChatClass()
{
    create_date = time(NULL);
    chat_id     = 0;
    next_msg_id = 1;
};

ChatClass::~ChatClass()
{
};

// static

ChatClass *ChatClass::loadFromFile(char *filename)
{
    FILE *in            = NULL;
    char *line          = NULL;
    int file_version    = 0;
    ChatClass *chat     = new ChatClass();
    struct parts ps;
    size_t tmp, n;

    filename = realpath(filename, NULL);
    if (filename == NULL){
        PDEBUG(7, "realpath() failed for: '%s'\n", filename);
        goto ret;
    }

    PDEBUG(10, "Trying to load ChatClass from: '%s'\n", filename);

    in = fopen(filename, "rb");
    if (in == NULL){
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

        // READ FILE VERSION {
        char *text = (char *)"# FILE_VERSION=";
        char *pos  = strstr(line, text);
        if (pos != NULL){
            pos += strlen(text);
            if (*pos != '\0'){
                file_version = atoi(pos);
            }
        }
        if (line[0] == '#'){
            // comment line
            free(line);
            continue;
        }
        // READ FILE VERSION }

        if (   file_version != 0
            && file_version != 1)
        {
            PERROR("Unsupported file version: '%d' ('%s')\n",
                file_version,
                filename
            );
            free(line);
            goto fatal;
        }

        initPartsMulti2(&ps, line, (char *)"\t ");
        if (ps.count >= 3){
            history_item item;
            if (file_version == 0){
                item.id         = chat->next_msg_id++;
                item.time       = atoi(ps.part[0]);
                item.msg_type   = ps.part[1];
                item.msg        = ps.part[2];
                chat->history.push_back(item);
            } else if (file_version == 1){
                item.id         = atoi(ps.part[0]);
                item.time       = atoi(ps.part[1]);
                item.msg_type   = ps.part[2];
                item.msg        = ps.part[3];
                chat->history.push_back(item);
                if (item.id > chat->next_msg_id){
                    // store max message ID value
                    chat->next_msg_id = item.id + 1;
                }
            }
        }
        freeParts(&ps);
        free(line);
    } while (in);
    fclose (in);

ret:
    if (filename){
        free(filename);
    }
    return chat;

fatal:
    if (filename){
        free(filename);
    }
    fclose (in);
    exit(1);
}


