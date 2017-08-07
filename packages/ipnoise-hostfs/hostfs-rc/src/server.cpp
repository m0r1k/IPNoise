#include "server.hpp"

static struct fuse_lowlevel_ops fuse_ll_oper;

Server::Server()
{
    // libevent
    m_base              = NULL;
    m_bev               = NULL;
    m_timer             = NULL;

    // fuse
    m_fuse_args          = NULL;
    m_fuse_ev            = NULL;
    m_fuse_se            = NULL;
    m_fuse_ch            = NULL;
    m_fuse_mountpoint    = NULL;
};

Server::~Server()
{
};

void Server::shutdown_cb(evutil_socket_t, short, void *a_ctx)
{
    Server *server = (Server *)a_ctx;
    assert(server != NULL);

    PINFO("shutdown..\n");

    // stop fuse
    server->stopFuse();

    event_base_loopexit(server->m_base, NULL);
};

void Server::timer_cb(
    evutil_socket_t,    // a_fd,
    short,              // a_event
    void                *a_arg)
{
    Server *server = (Server *)a_arg;
    assert (server != NULL);

    PDEBUG(30, "timer\n");

    // reschedule
    {
        struct timeval tv;
        evutil_timerclear(&tv);
        tv.tv_sec = 1;
        evtimer_add(server->m_timer, &tv);
    }
}

int32_t Server::do_init(
    struct event_base *a_base)
{
    int32_t             res, err      = -1;
    struct  event       *signal_event = NULL;
    struct  timeval     tv;
    string              address;

    if (not a_base){
        goto fail;
    }

    // store ev base
    m_base = a_base;

    // init fuse
    res = initFuse();
    if (res){
        PERROR("Cannot init fuse subsystem\n");
        goto fail;
    }

    // handle interrupts
    signal_event = evsignal_new(
        m_base,
        SIGINT,
        Server::shutdown_cb,
        (void *)this
    );

    if (    !signal_event
        ||  event_add(signal_event, NULL) < 0)
    {
        PERROR("Could not create/add a signal event!\n");
        goto fail;
    }

    if (!m_timer){
        m_timer = evtimer_new(m_base, Server::timer_cb, (void*)this);
        evutil_timerclear(&tv);
        tv.tv_sec = 1;
        evtimer_add(m_timer, &tv);
    }

    // all ok
    err  = 0;

out:
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

// ---------------------- Fuse filesystem -----------------------------------

int32_t Server::initFuse()
{
    const char *argv[3] = { "ipnoise-hostfs", FUSE_DIR, NULL };
    int32_t     argc    = 2;
    int32_t     fd      = -1, err = -1;
    int32_t     res;
    string      std_out, std_err;

    {
        // do umount before
        const char * const argv[] = {
            "/bin/umount",
            FUSE_DIR,
            NULL
        };
        res = my_system(argv, std_out, std_err);
    }

    // going init fuse
    m_fuse_args = (struct fuse_args *)malloc(sizeof(struct fuse_args));
    if (!m_fuse_args){
        PERROR("Malloc failed for 'struct fuse_args',"
            " was needed: '%d' bute(s)\n",
            sizeof(struct fuse_args)
        );
        goto fail;
    }

    *m_fuse_args = FUSE_ARGS_INIT(argc, (char **)argv);

    res = fuse_parse_cmdline(m_fuse_args, &m_fuse_mountpoint, NULL, NULL);
    if (res == -1){
        PERROR("Cannot parse fuse cmdline\n");
        goto fail;
    }

    fuse_ll_oper.lookup    = Server::fuse_ll_lookup;
    fuse_ll_oper.getattr   = Server::fuse_ll_getattr;
    fuse_ll_oper.readdir   = Server::fuse_ll_readdir;
    fuse_ll_oper.open      = Server::fuse_ll_open;
    fuse_ll_oper.read      = Server::fuse_ll_read;
    fuse_ll_oper.release   = Server::fuse_ll_release;

    m_fuse_ch = fuse_mount(m_fuse_mountpoint, m_fuse_args);
    if (!m_fuse_ch){
        PERROR("Cannot mount fuse to mount point: '%s'\n",
            m_fuse_mountpoint
        );
        goto fail;
    }

    m_fuse_se = fuse_lowlevel_new(
        m_fuse_args,
        &fuse_ll_oper,
        sizeof(fuse_ll_oper),
        NULL
    );
    if (!m_fuse_se){
        PERROR("Cannot init fuse lowlevel\n");
        goto fail;
    }

    res = fuse_set_signal_handlers(m_fuse_se);
    if (-1 == res){
        PERROR("Cannot set fuse signal handlers\n");
        goto fail;
    }

    fuse_session_add_chan(m_fuse_se, m_fuse_ch);

    // create event handler
    fd = fuse_chan_fd(m_fuse_ch);
    if (fd < 0){
        PERROR("Was received invalid file descriptor: '%d'"
            " from fuse_chan_fd()\n", fd);
        goto fail;
    }

    res = fcntl(fd, F_SETFL, O_NONBLOCK);
    if (-1 == res){
        PERROR("Cannot setup flags for fuse descriptor (O_NONBLOCK)\n");
        goto fail;
    }

    m_fuse_ev = event_new(
        m_base,
        fd,
        EV_READ | EV_PERSIST,
        Server::fuseSessionLoop,
        (void *)this
    );
    if (!m_fuse_ev){
        PERROR("Cannot create new event\n");
        goto fail;
    }

    event_add(m_fuse_ev, NULL);

    // all ok
    err = 0;

out:
    return err;

fail:
    stopFuse();

    if (!err){
        err = -1;
    }
    goto out;
}

int32_t Server::stopFuse()
{
    int32_t err = 0;

    if (m_fuse_ev){
        event_free(m_fuse_ev);
        m_fuse_ev = NULL;
    }
    if (m_fuse_se && m_fuse_ch){
        fuse_session_remove_chan(m_fuse_ch);
    }
    if (m_fuse_se){
        fuse_remove_signal_handlers(m_fuse_se);
        fuse_session_destroy(m_fuse_se);
        m_fuse_se = NULL;
    }
    if (m_fuse_ch){
        fuse_unmount(m_fuse_mountpoint, m_fuse_ch);
        m_fuse_ch = NULL;
    }
    if (m_fuse_args){
        fuse_opt_free_args(m_fuse_args);
        m_fuse_args = NULL;
    }

    return err;
}

int32_t Server::fuse_stat(
//    DomElement      *, // a_ipnoise,
    fuse_ino_t      a_ino,
    struct stat     *) // a_stbuf
{
    int32_t err = -1;

    PERROR("MORIK a_ino: '%d'\n",
        (int32_t)a_ino
    );

/*
    char buffer[1024]   = { 0x00 };
    int32_t ino_subtype     = 0;

    vector<DomElement *> items;
    DomElement *element = NULL;

    stbuf->st_ino   = ino;
    ino_subtype     = (ino - (int(ino / INODE_NEXT_ELEMENT_OFFSET)
        * INODE_NEXT_ELEMENT_OFFSET));

    if (    (ino_subtype == INODE_SUBTYPE_ATTRIBUTES)
        ||  (ino_subtype == INODE_SUBTYPE_LOG)
        ||  (ino_subtype == INODE_SUBTYPE_XML))
    {
        stbuf->st_mode  = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size  = 100e6;
    } else {
        snprintf(buffer, sizeof(buffer),
            (unsigned int)ino
        );
        items.clear();
        ipnoise->getElementsByXpath(buffer, items);
        if (items.size()){
            element = items[0];
        }

        if (!element){
            goto fail;
        }

        stbuf->st_mode  = S_IFDIR | 0755;
        stbuf->st_nlink = 2;
    }
*/

    // all ok;
    err = 0;

//out:
    return err;

//fail:
//    err = -1;
//    goto out;
}

void Server::fuse_ll_getattr(
    fuse_req_t                  a_req,
    fuse_ino_t                  a_ino,
    struct fuse_file_info *)    // a_fi
{
    PERROR("MORIK req: '%d', ino: '%d'\n",
        (int32_t)a_req,
        (int32_t)a_ino
    );

    if (1 == a_ino){
    }

/*
    int32_t res;
    struct stat stbuf;
    IPNoiseObject *ipnoise = NULL;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = (IPNoiseObject *)server->getDocument()->getRootElement();
    assert(ipnoise);

    // clear buf
    memset(&stbuf, 0, sizeof(stbuf));

    res = fuse_stat(ipnoise, ino, &stbuf);
    if (res == -1){
        goto fail;
    }

    fuse_reply_attr(req, &stbuf, 1.0);

out:
    return;

fail:
    fuse_reply_err(req, ENOENT);
    goto out;
*/

    fuse_reply_err(a_req, ENOENT);
}

void Server::fuse_ll_lookup(
    fuse_req_t      a_req,
    fuse_ino_t      a_parent,
    const char      *a_name)
{
    PERROR("MORIK req: '%d', parent: '%d', name: '%s'\n",
        (int32_t)a_req,
        (int32_t)a_parent,
        a_name
    );

/*
    struct fuse_entry_param e;
    char buffer[1024] = { 0x00 };
    int32_t i;

    DomElement *element         = NULL;
    DomElement *parent_element  = NULL;
    DomElement *ipnoise         = NULL;

    vector<DomElement *>            items;
    vector<DomElement *>::iterator  items_i;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    if (!strncmp(name, ".attributes", 11)){
        memset(&e, 0, sizeof(e));
        e.ino               = (parent - 1) + INODE_SUBTYPE_ATTRIBUTES;
        e.attr_timeout      = 1.0;
        e.entry_timeout     = 1.0;
        fuse_stat(ipnoise, e.ino, &e.attr);
        fuse_reply_entry(req, &e);
        goto out;
    }

    if (!strncmp(name, ".log", 4)){
        memset(&e, 0, sizeof(e));
        e.ino               = (parent - 1) + INODE_SUBTYPE_LOG;
        e.attr_timeout      = 1.0;
        e.entry_timeout     = 1.0;
        fuse_stat(ipnoise, e.ino, &e.attr);
        fuse_reply_entry(req, &e);
        goto out;
    }
    if (!strncmp(name, ".xml", 4)){
        memset(&e, 0, sizeof(e));
        e.ino               = (parent - 1) + INODE_SUBTYPE_XML;
        e.attr_timeout      = 1.0;
        e.entry_timeout     = 1.0;
        fuse_stat(ipnoise, e.ino, &e.attr);
        fuse_reply_entry(req, &e);
        goto out;
    }

    snprintf(buffer, sizeof(buffer),
        (unsigned int)parent
    );
    items.clear();
    ipnoise->getElementsByXpath(buffer, items);
    if (items.size()){
        parent_element = items[0];
    }

    // check what parent exist
    if (!parent_element){
        goto fail;
    }

    element = NULL;
    items.clear();
    parent_element->getChilds(items);
    for (i = 0; i < (int)items.size(); i++){
        DomElement *cur_element = items[i];
        string cur_fuse_name    = cur_element->getFuseFullName();
        if (cur_fuse_name == name){
            element = cur_element;
            break;
        }
    }
    if (!element){
        goto fail;
    }

    // fill data
    memset(&e, 0, sizeof(e));
    e.ino               = element->getInoInt();
    e.attr_timeout      = 1.0;
    e.entry_timeout     = 1.0;

    fuse_stat(ipnoise, e.ino, &e.attr);
    fuse_reply_entry(req, &e);

out:
    return;

fail:
    fuse_reply_err(req, ENOENT);
    goto out;
*/

    fuse_reply_err(a_req, ENOENT);
}

void Server::dirbuf_add(
    fuse_req_t      a_req,
    struct dirbuf   *a_b,
    const char      *a_name,
    fuse_ino_t      a_ino)
{
    struct stat stbuf;
    size_t      oldsize = a_b->size;

    a_b->size += fuse_add_direntry(
        a_req,
        NULL,
        0,
        a_name,
        NULL,
        0
    );
    a_b->p = (char *)realloc(a_b->p, a_b->size);

    memset(&stbuf, 0, sizeof(stbuf));

    stbuf.st_ino = a_ino;
    fuse_add_direntry(
        a_req,
        a_b->p + oldsize,
        a_b->size - oldsize,
        a_name,
        &stbuf,
        a_b->size
    );
}

void Server::fuse_ll_readdir(
    fuse_req_t                  a_req,
    fuse_ino_t                  a_ino,
    size_t,                     // size
    off_t,                      // off
    struct fuse_file_info *)    // fi
{
    PERROR("MORIK req: '%d', ino: '%d'\n",
        (int32_t)a_req,
        (int32_t)a_ino
    );

/*
    int32_t i;
    char buffer[1024] = { 0x00 };

    vector<DomElement *> items;
    DomElement *element         = NULL;
    DomElement *parent_element  = NULL;
    DomElement *ipnoise         = NULL;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    // search element
    snprintf(buffer, sizeof(buffer),
*/
//        "//*[@_ino=\"%x\"]",
/*
        (unsigned int)ino
    );
    element = ipnoise->getElementByXpath(buffer);

    if (element){
        struct dirbuf b;
        memset(&b, 0, sizeof(b));

        int32_t element_id          = element->getInoInt();
        int32_t parent_element_id   = element->getInoInt();

        parent_element = element->getParentNode();
        if (parent_element){
            parent_element_id = parent_element->getInoInt();
        }

        items.clear();
        element->getChilds(items);

        dirbuf_add(req, &b, ".",            element_id);
        dirbuf_add(req, &b, "..",           parent_element_id);
        dirbuf_add(req, &b, ".attributes",  element_id + INODE_SUBTYPE_ATTRIBUTES);
        dirbuf_add(req, &b, ".log",         element_id + INODE_SUBTYPE_LOG);
        dirbuf_add(req, &b, ".xml",         element_id + INODE_SUBTYPE_XML);

        for (i = 0; i < (int)items.size(); i++){
            DomElement *cur_element     = items[i];
            string fuse_name            = cur_element->getFuseFullName();

            dirbuf_add(
                req,
                &b,
                fuse_name.c_str(),
                cur_element->getInoInt()
            );
        }

        fuse_reply_buf_limited(req, b.p, b.size, off, size);
        free(b.p);
    } else {
        fuse_reply_err(req, ENOTDIR);
    }
*/
    fuse_reply_err(a_req, ENOTDIR);
}

void Server::fuse_ll_open(
    fuse_req_t                  a_req,
    fuse_ino_t                  a_ino,
    struct fuse_file_info *) // fi
{
    PERROR("MORIK req: '%d', ino: '%d'\n",
        (int32_t)a_req,
        (int32_t)a_ino
    );


/*
    char buffer[1024]       = { 0x00 };
    int32_t ino_subtype         = 0;
    int32_t ino_element         = 0;
    DomElement *element     = NULL;
    DomElement *ipnoise     = NULL;
    FuseClient *fuse_client = NULL;

    vector <DomElement *> items;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    // calculate node subtype
    ino_subtype = (ino - (int(ino / INODE_NEXT_ELEMENT_OFFSET)
        * INODE_NEXT_ELEMENT_OFFSET));

    // calculate inode number for element
    ino_element = (ino + 1) - ino_subtype;

    if (    ino_subtype != INODE_SUBTYPE_ATTRIBUTES
        &&  ino_subtype != INODE_SUBTYPE_LOG
        &&  ino_subtype != INODE_SUBTYPE_XML)
    {
        // look like is not file..
        goto error_is_dir;
    }

    if ((fi->flags & 3) != O_RDONLY){
        // must be read only access
        goto error_eaccess;
    }

    // search element
    snprintf(buffer, sizeof(buffer),
        ino_element
    );

    element = ipnoise->getElementByXpath(buffer);
    if (!element){
        goto error_eaccess;
    }

    // open file handle
    fuse_client = element->openFuseClient();
    if (!fuse_client){
        PERROR("Cannot create new file handle\n");
        goto error_eaccess;
    }

    // set client type (inode type)
    fuse_client->setType(ino_subtype);

    // store
    fi->fh = (uint64_t)fuse_client;

    // force direct io for avoid kernel mind
    fi->direct_io = 1;

    // all ok
    fuse_reply_open(req, fi);

out:
    return;

error_is_dir:
    fuse_reply_err(req, EISDIR);
    goto out;

error_eaccess:
    fuse_reply_err(req, EACCES);
    goto out;
*/

    fuse_reply_err(a_req, EACCES);
}

void Server::fuse_ll_read(
    fuse_req_t                  a_req,
    fuse_ino_t                  a_ino,
    size_t,                     // size
    off_t,                      // off
    struct fuse_file_info *)    // fi
{
    PERROR("MORIK req: '%d', ino: '%d'\n",
        (int32_t)a_req,
        (int32_t)a_ino
    );


/*
    char        buffer[1024]    = { 0x00 };
    DomElement  *element        = NULL;
    int32_t     ino_subtype     = 0;
    int32_t     ino_element     = 0;
    int32_t     count           = 0;
    int32_t     res;
    string      tmp;

    DomElement *ipnoise     = NULL;
    FuseClient *fuse_client = NULL;

    vector <DomElement *>           items;
    map <string, string>            attributes;
    map <string, string>::iterator  attributes_i;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    fuse_client = (FuseClient *)fi->fh;

    // calculate node subtype
    ino_subtype = (ino - (int(ino / INODE_NEXT_ELEMENT_OFFSET)
        * INODE_NEXT_ELEMENT_OFFSET));

    // calculate inode number for element
    ino_element = (ino + 1) - ino_subtype;

    // search element
    snprintf(buffer, sizeof(buffer),
        ino_element
    );

    element = ipnoise->getElementByXpath(buffer);
    if (!element){
        goto fail;
    }

    if (!element->isFuseClientOpen(fuse_client)){
        goto fail;
    }

    switch (ino_subtype){
        case (INODE_SUBTYPE_ATTRIBUTES):
            // get all element's attributes
            attributes = element->getAllAttributes();

            // init buffer ptr
            count = 0;
            for (attributes_i = attributes.begin();
                attributes_i != attributes.end();
                attributes_i++)
            {
                res = snprintf(buffer + count, sizeof(buffer) - count,
                    "%s=%s\n",
                    attributes_i->first.c_str(),
                    attributes_i->second.c_str()
                );
                if (res > 0){
                    count += res;
                } else {
                    break;
                }
            }

            fuse_reply_buf(req, buffer, count);
            element->closeFuseClient(fuse_client);
            break;

        case (INODE_SUBTYPE_LOG):
            // read from log
            element->processLog(fuse_client->getFd(), req);
            break;

        case (INODE_SUBTYPE_XML):
            tmp = element->serialize(1);
            tmp += "\n";
            fuse_reply_buf(req, tmp.c_str(), tmp.size());
            element->closeFuseClient(fuse_client);
            break;

        default:
            goto fail;
            break;
    };

out:
    return;

fail:

    fuse_reply_buf(req, NULL, 0);
    goto out;
*/

    fuse_reply_buf(a_req, NULL, 0);
}

void Server::fuse_ll_release(
    fuse_req_t                  a_req,
    fuse_ino_t                  a_ino,
    struct fuse_file_info *) // fi
{
    PERROR("MORIK req: '%d', ino: '%d'\n",
        (int32_t)a_req,
        (int32_t)a_ino
    );

/*
    char        buffer[1024]    = { 0x00 };
    DomElement  *element        = NULL;
    int32_t     ino_subtype     = 0;
    int32_t     ino_element     = 0;
    DomElement  *ipnoise        = NULL;
    FuseClient  *fuse_client    = NULL;

    vector <DomElement *>           items;
    map <string, string>            attributes;
    map <string, string>::iterator  attributes_i;

    // this file must be single place where used global variable "ipn_server"
    ipnoise = server->getDocument()->getRootElement();
    assert(ipnoise);

    fuse_client = (FuseClient *)fi->fh;

    // calculate node subtype
    ino_subtype = (ino - (int(ino / INODE_NEXT_ELEMENT_OFFSET)
        * INODE_NEXT_ELEMENT_OFFSET));

    // calculate inode number for element
    ino_element = (ino + 1) - ino_subtype;

    // search element
    snprintf(buffer, sizeof(buffer),
        ino_element
    );

    element = ipnoise->getElementByXpath(buffer);
    if (!element){
        goto fail;
    }

    if (!element->isFuseClientOpen(fuse_client)){
        goto fail;
    }

    // close client
    element->closeFuseClient(fuse_client);

out:
    return;

fail:
    goto out;
*/
}

void Server::fuseSessionLoop(
    int32_t,    //  a_socket
    short,      //  a_event
    void        *a_arg)
{
    PERROR("Server::fuseSessionLoop\n");

    int32_t             res         = 0;
    struct fuse_session *fuse_se    = NULL;
    struct fuse_chan    *fuse_ch    = NULL;
    struct fuse_chan    *tmpch      = NULL;
    size_t bufsize                  = 0;
    char                *buff       = NULL;
    Server              *server     = NULL;

    server = (Server*)a_arg;
    assert(server);

    fuse_se = server->m_fuse_se;
    fuse_ch = fuse_session_next_chan(fuse_se, NULL);
    bufsize = fuse_chan_bufsize(fuse_ch);
    tmpch   = fuse_ch;

    buff = (char *) malloc(bufsize);
    if (!buff) {
        PERROR("fuse: failed to allocate read buffer\n");
        goto fail;
    }

    res = fuse_chan_recv(&tmpch, buff, bufsize);
    if (res > 0){
        fuse_session_process(fuse_se, buff, res, tmpch);
    }

    free(buff);
    fuse_session_reset(fuse_se);

out:
    return;

fail:
    goto out;
}


