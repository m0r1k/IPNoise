#include <string.h>

#include "nc_tcp_server.h"
#include "nc_tcp_client.h"

#include "main.h"

int32_t g_debug_level = 1;

void iov_alloc(
    struct iovec    *iov,
    int32_t         size)
{
    if (!iov){
        PERROR("attempt to alloc, but iov empty\n");
        goto fail;
    }

    if (size <= 0){
        iov->iov_len  = 0;
        iov->iov_base = NULL;
    } else {
        iov->iov_len  = size;
        iov->iov_base = malloc(iov->iov_len);
    }

out:
    return;
fail:
    goto out;
}

int32_t iov_read(
    int32_t         fd,
    struct iovec    *iov,
    int32_t         need_read)
{
    int32_t res;
    int32_t total_read = 0;

    if (!iov){
        PERROR("attempt to read in empty iov\n");
        goto fail;
    }

    for (;;){
        if (!iov->iov_base){
            PERROR("cannot allocate iov_base\n");
            goto fail;
        }
        res = read(fd,
            (unsigned char *)iov->iov_base + total_read,
            iov->iov_len - total_read
        );
//        PERROR("already read: '%d',"
//            " read res: '%d',"
//            " errno: '%d',"
//            " iov->iov_base: 0x'%x',"
//            " iov->iov_len: %d\n",
//            total_read,
//            res,
//            errno,
//            (unsigned int)iov->iov_base,
//            iov->iov_len
//        );
        if (res < 0){
            if (EINTR == errno){
                continue;
            }
            if (EAGAIN == errno){
                continue;
            }
            break;
        } else if (!res){
            PERROR("eof\n");
            break;
        }
        total_read += res;
        need_read  -= res;
        if (need_read <= 0){
            break;
        }
        if ((iov->iov_len - total_read) <= 0){
            iov->iov_len *= 2;
            iov->iov_base = realloc(iov->iov_base, iov->iov_len);
        }
    }

out:
    return total_read;
fail:
    if (iov->iov_base){
        free(iov->iov_base);
        iov->iov_base = NULL;
    }
    iov->iov_len = 0;
    if (iov){
        free(iov);
        iov = NULL;
    }
    goto out;
}

void iov_fill(struct iovec *iov)
{
    int32_t         i;
    unsigned char   *iov_base = NULL;

    if (!iov){
        PERROR("attempt to fill empty iov\n");
        goto fail;
    }

    if (!iov->iov_base){
        PERROR("attempt to fill iov with empty iov_base\n");
        goto fail;
    }

    iov_base = (unsigned char *)iov->iov_base;

    for (i = 0; i < iov->iov_len; i++){
        char c = '1' + (unsigned char)(rand() & 7);
        iov_base[i] = c;
    }

    iov_base[iov->iov_len - 2] = '\n';
    iov_base[iov->iov_len - 1] = '\0';

out:
    return;
fail:
    goto out;
}

int32_t iov_cmp(
    struct iovec    *iov1,
    struct iovec    *iov2,
    int32_t         length)
{
    int32_t len = 0;
    int32_t ret = -1;

    if (!iov1 || !iov1->iov_base){
        goto out;
    }

    if (!iov2 || !iov2->iov_base){
        goto out;
    }

    if (length < iov1->iov_len){
        PERROR("not equal (was read '%d',"
            " but need '%d' byte(s))\n",
            length,
            iov1->iov_len
        );
        goto out;
    }

    len = min(iov1->iov_len, length);

    PERROR("compare %d byte(s)\n", len);
    ret = memcmp(iov1->iov_base, iov2->iov_base, len);
    PERROR("%s\n", ret ? "not equal" : "equal");

out:
    return ret;
}

void iov_free(struct iovec *iov)
{
    if (iov->iov_base){
        free(iov->iov_base);
        iov->iov_base = NULL;
    }
    iov->iov_len = 0;
}

int32_t create_tcp_socket()
{
    int32_t fd = -1;

    fd = socket(
        conf.hostos ? PF_HOSTOS : PF_INET,
        SOCK_STREAM,
        TCP_PROTO_NUM
    );
    if (fd < 0){
        PERROR("socket(%d, SOCK_STREAM, %d) failed\n",
            conf.hostos ? PF_HOSTOS : PF_INET,
            TCP_PROTO_NUM
        );
        goto fail;
    }

out:
    return fd;
fail:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    goto out;
}

int32_t do_bind(int32_t a_fd)
{
    int32_t res, err = -1;
    struct sockaddr_in addr;

    memset(&addr, 0x00, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(conf.port);
    memcpy(
        &addr.sin_addr.s_addr,
        &conf.addr,
        sizeof(addr.sin_addr.s_addr)
    );

    {
        // enable reuse
        int32_t on = 1;
        res = setsockopt(
            a_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (char *)&on,
            sizeof(on)
        );
        if (res){
            err = res;
            PERROR("setsockopt("
                "%d, SOL_SOCKET, SO_REUSEADDR, %d, %d)"
                " failed\n",
                a_fd,
                on,
                sizeof(on)
            );
            goto fail;
        }
    }

    // bind
    res = bind(a_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (res){
        err = res;
        PERROR("bind failed\n");
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

// ---------------- echo server ----------------

int32_t echo_server()
{
    int32_t             res, err = -1;
    int32_t             client_fd, fd;
    struct sockaddr     client_addr;
    socklen_t           client_addrlen = sizeof(client_addr);

    struct iovec iov1;
    iov_alloc(&iov1, ECHO_SERVER_DATA_SIZE);
    iov_fill(&iov1);

    fd = create_tcp_socket();
    if (fd < 0){
        PERROR("cannot create tcp socket\n");
        goto fail;
    }

    res = do_bind(fd);
    if (res < 0){
        PERROR("bind failed\n");
        goto fail;
    }

    // listen
    res = listen(fd, 1);
    if (res){
        PERROR("listen failed\n");
        goto fail;
    }

    while (1){
        struct iovec iov2;
        iov_alloc(&iov2, ECHO_SERVER_DATA_SIZE);

        PINFO("waiting clients\n");
        client_fd = accept(fd, &client_addr, &client_addrlen);
        if (client_fd < 0){
            break;
        }

        PINFO("accepted, client_fd: '%d'\n",
            client_fd);
        PINFO("attempt to write '%d' byte(s)\n",
            iov1.iov_len);

        int32_t total_wrote = 0;
        while ((iov1.iov_len - total_wrote) > 0){
            res = write(client_fd,
                (unsigned char *)iov1.iov_base + total_wrote,
                iov1.iov_len - total_wrote
            );
            PINFO("after write"
                " fd: '%d',"
                " res: '%d',"
                " total_wrote: '%d'\n",
                client_fd,
                res,
                total_wrote
            );
            if (res < 0 && errno != EINTR){
                break;
            }
            if (!res){
                break;
            }
            total_wrote += res;
        }

        if (total_wrote <= 0){
            break;
        }

        if (0){
            int32_t flags = 0;
            flags = fcntl(client_fd, F_GETFL);
            flags |= O_NONBLOCK;
            fcntl(client_fd, F_SETFL, flags);
        }

        PINFO("{ attempt to iov_read\n");
        res = iov_read(client_fd, &iov2, total_wrote);
        PINFO("} read res: '%d'\n", res);

        if (res <= 0){
            break;
        }

        iov_cmp(&iov1, &iov2, res);

        close(client_fd);

        iov_free(&iov2);
    }

    close(fd);

    // all ok
    err = 0;

    iov_free(&iov1);

out:
    return err;
fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int32_t echo_client()
{
    int32_t res, err        = -1;
    int32_t sd              = -1;
    int32_t total_read      = 0;
    int32_t total_wrote     = 0;
    char    *buffer         = NULL;
    struct sockaddr_in addr;

    buffer = (char *)malloc(ECHO_SERVER_DATA_SIZE);
    if (!buffer){
        PERROR("cannot allocate memory,"
            " was needed: '%d' byte(s)\n",
            ECHO_SERVER_DATA_SIZE
        );
    }

    // prepare
    memset(&addr, sizeof(addr), 0x00);

    // trying to open socket
    sd = create_tcp_socket();
    if (sd < 0){
        PERROR("cannot create socket\n");
        goto fail;
    }

    // setup address
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(conf.port);
    memcpy(&addr.sin_addr, &conf.addr, sizeof(addr.sin_addr));

    // trying to connect
    res = connect(
        sd,
        (const struct sockaddr *)&addr,
        sizeof(addr)
    );
    if (res < 0){
        inet_ntop(
            AF_INET,    (const void *)&conf.addr,
            buffer,     (socklen_t)(sizeof buffer)
        );
        PERROR("cannot connect to %s:%d\n",
            buffer,
            conf.port
        );
        goto fail;
    }

    for (;;){
        // read
        PINFO("attempt to read\n");
        res = read(sd, buffer, ECHO_SERVER_DATA_SIZE);
        if (0){
            PINFO("after read"
                " fd: '%d',"
                " res: '%d',"
                " total_read: '%d'\n",
                sd,
                res,
                total_read
            );
        }
        if (res <= 0){
            break;
        }
        total_read += res;

        // write
        PINFO("{ attempt to write\n");
        res = write(sd, buffer, res);
        PINFO("} after write"
            " fd: '%d',"
            " res: '%d',"
            " total_wrote: '%d'\n",
            sd,
            res,
            total_wrote
        );
        if (res <= 0){
            break;
        }
        total_wrote += res;
    }

    // all ok
    err = 0;

out:
    if (buffer){
        free(buffer);
        buffer = NULL;
    }
    if (sd >= 0){
        close(sd);
        sd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

void usage(int32_t argc, char **argv)
{
    PINFO(
        "Usage:     %s [-options]                                \n"
        "options:                                                \n"
        "         -h, --help         This help                   \n"
        "         -t, --type <type>  Variants below:             \n"
        "             echo-server    random trafic generator     \n"
        "             echo-client    trafic reflector            \n"
        "             send-file      (required --fname file)     \n"
        "             recv-file      (required --fname file)     \n"
        "             nc-tcp-server  net cat tcp server          \n"
        "             nc-tcp-client  net cat tcp client          \n"
        "             nc-udp-server  net cat udp server          \n"
        "             nc-udp-client  net cat udp client          \n"
        "         -f, --fname        path to content file        \n"
        "         -c, --content      content                     \n"
        "         -a, --addr         Address for bind/connect    \n"
        "         -p, --port         Port for bind/connect       \n"
        "         -o, --hostos       Use HOSTOS for bind/connect \n"
        "         -v, --version      Program version             \n"
        "         --debug <level>    Debug level                 \n"
        "\n",
        argv[0]
    );
}

int32_t recv_file()
{
    int32_t res, err        = -1;
    int32_t fd              = -1;
    int32_t listen_sd       = -1;
    int32_t client_sd       = -1;
    int32_t total_recv      = 0;
    char    buffer[65536]   = { 0x00 };

    struct sockaddr_in  client_addr;
    socklen_t           client_addrlen = sizeof(client_addr);

    // trying to open
    fd = open(
        conf.fname,
        O_CREAT | O_WRONLY | O_CLOEXEC | O_TRUNC, 0755
    );
    if (fd < 0){
        PERROR("Cannot open file: '%s' for write\n",
            conf.fname);
        goto fail;
    }

    // trying to open socket
    listen_sd = create_tcp_socket();
    if (listen_sd < 0){
        PERROR("cannot create socket\n");
        goto fail;
    }

    // do bind
    res = do_bind(listen_sd);
    if (res < 0){
        PERROR("bind failed\n");
        goto fail;
    }

    // do listen
    res = listen(listen_sd, 1);
    if (res < 0){
        PERROR("listen failed\n");
        goto fail;
    }

    // do accept
    PINFO("waiting clients\n");
    client_sd = accept(
        listen_sd,
        (struct sockaddr *)&client_addr,
        &client_addrlen
    );
    if (client_sd < 0){
        goto fail;
    }

    inet_ntop(
        AF_INET,    (const void *)&client_addr.sin_addr,
        buffer,     (socklen_t)(sizeof buffer)
    );
    PINFO("accepted from %s:%d\n",
        buffer,
        ntohs(client_addr.sin_port)
    );

    for (;;){
        // read from socket
        res = read(client_sd, buffer, sizeof(buffer));
        if (res <= 0){
            break;
        }
        total_recv += res;
        res = write(fd, buffer, res);
        if (res <= 0){
            break;
        }
    }

    PINFO("total_recv: '%d'\n", total_recv);

    // all ok
    err = 0;

out:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    if (client_sd >= 0){
        close(client_sd);
        client_sd = -1;
    }
    if (listen_sd >= 0){
        close(listen_sd);
        listen_sd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int32_t send_file()
{
    int32_t res, err        = -1;
    int32_t fd              = -1;
    int32_t sd              = -1;
    int32_t total_send      = 0;
    char    buffer[65536]   = { 0x00 };
    struct sockaddr_in addr;

    // prepare
    memset(&addr, sizeof(addr), 0x00);

    // trying to open
    fd = open(conf.fname, O_RDONLY);
    if (fd < 0){
        PERROR("cannot open file: '%s' for read\n",
            conf.fname);
        goto fail;
    }

    // trying to open socket
    sd = create_tcp_socket();
    if (sd < 0){
        PERROR("cannot create socket\n");
        goto fail;
    }

    // setup address
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(conf.port);
    memcpy(&addr.sin_addr, &conf.addr, sizeof(addr.sin_addr));

    // trying to connect
    res = connect(
        sd,
        (const struct sockaddr *)&addr,
        sizeof(addr)
    );
    if (res < 0){
        inet_ntop(
            AF_INET,    (const void *)&conf.addr,
            buffer,     (socklen_t)(sizeof buffer)
        );
        PERROR("cannot connect to %s:%d\n",
            buffer,
            conf.port
        );
        goto fail;
    }

    for (;;){
        // read from file
        res = read(fd, buffer, sizeof(buffer));
        if (res <= 0){
            break;
        }
        res = write(sd, buffer, res);
        if (res <= 0){
            break;
        }
        total_send += res;
    }

    PINFO("total_send: '%d'\n", total_send);

    // all ok
    err = 0;

out:
    if (fd >= 0){
        close(fd);
        fd = -1;
    }
    if (sd >= 0){
        close(sd);
        sd = -1;
    }
    return err;

fail:
    if (err >= 0){
        err = -1;
    }
    goto out;
}

int32_t main(int32_t argc, char **argv)
{
    int32_t res, err = -1;

    char buffer[1024] = { 0x00 };

    // reset conf
    memset(&conf, 0x00, sizeof(conf));

    // get args
    while (1){
        int32_t option_index = 0;
        static struct option long_options[] = {
            {"help",        0, 0, 'h'},
            {"type",        1, 0, 't'},
            {"fname",       1, 0, 'f'},
            {"addr",        1, 0, 'a'},
            {"port",        1, 0, 'p'},
            {"hostos",      1, 0, 'o'},
            {"version",     0, 0, 'v'},
            {"debug",       1, 0, 'd'},
            {0, 0, 0, 0}
        };

        int32_t option = getopt_long(
            argc, argv,
            "ht:f:c:a:p:ovd:",
            long_options, &option_index
        );
        if (option == -1){
            break;
        }

        switch (option) {
            case 'h':
                usage(argc, argv);
                exit(0);

            case 't':
                snprintf(conf.type, sizeof(conf.type), "%s", optarg);
                break;

            case 'f':
                snprintf(conf.fname, sizeof(conf.fname), "%s", optarg);
                break;

            case 'c':
                conf.content_len = strlen(optarg);
                snprintf(
                    conf.content,
                    min(conf.content_len, sizeof(conf.content)),
                    "%s",
                    optarg
                );
                break;

            case 'a':
                res = inet_aton(optarg, &conf.addr);
                if (!res){
                    PERROR("cannot parse addr: '%s'\n",
                        optarg
                    );
                    goto fail;
                }
                break;

            case 'p':
                conf.port = atoi(optarg);
                if (    conf.port < 0
                    ||  conf.port > 65535)
                {
                    PERROR("invalid port: '%d'\n",
                        conf.port
                    );
                    goto fail;
                }
                break;

            case 'o':
                conf.hostos = 1;
                break;

            case 'd':
                g_debug_level = atoi(optarg);
                break;

            case 'v':
                PINFO("Version: '%s'\n", VERSION);
                exit(0);

            case '?':
                exit (1);
        }
    }

    // check another args
    if (optind < argc){
        char *buffer_ptr = buffer;
        int32_t res;
        res = snprintf(
            buffer_ptr,
            (buffer + sizeof(buffer)) - buffer_ptr,
            "Warning: argument(s) will be ignored: "
        );
        if (res > 0){
            buffer_ptr += res;
        }
        while (optind < argc){
            snprintf(
                buffer_ptr,
                (buffer + sizeof(buffer)) - buffer_ptr,
                "'%s' ",
                argv[optind++]
            );
        }
        PINFO("%s\n", buffer);
    }

    PINFO("Options:\n");
    PINFO("type:            '%s'\n", conf.type);
    PINFO("content fname:   '%s'\n", conf.fname);
    PINFO("content:         '%s'\n", conf.content);
    PINFO("content_len:     '%d'\n", conf.content_len);
    inet_ntop(
        AF_INET,    (const void *)&conf.addr,
        buffer,     (socklen_t)(sizeof buffer)
    );
    PINFO("addr:            '%s'\n", buffer);
    PINFO("port:            '%d'\n", conf.port);
    PINFO("hostos:          '%d'\n", conf.hostos);
    PINFO("debug-level:     '%d'\n", g_debug_level);
    PINFO("\n");

    if (!strcmp("echo-server", conf.type)){
        err = echo_server();
    } else if (!strcmp("echo-client", conf.type)){
        err = echo_client();
    } else if (!strcmp("send-file", conf.type)){
        err = send_file();
    } else if (!strcmp("recv-file", conf.type)){
        err = recv_file();
    } else if (!strcmp("nc-tcp-server", conf.type)){
        err = nc_tcp_server();
    } else if (!strcmp("nc-tcp-client", conf.type)){
        err = nc_tcp_client();
    } else if (!strcmp("nc-udp-server", conf.type)){
        err = nc_udp_server();
    } else if (!strcmp("nc-udp-client", conf.type)){
        err = nc_udp_client();
    } else {
        PERROR("unsupported test type: ''\n"
            "use %s -t <type>\n"
            "use %s --help for more info\n",
            argv[0],
            argv[0],
            conf.type
        );
        goto fail;
    }

out:
    return err;
fail:
    err = -1;
    goto out;
}

void set_non_block(
    int32_t a_fd)
{
    int32_t flags = 0;
    flags = fcntl(a_fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(a_fd, F_SETFL, flags);
}

