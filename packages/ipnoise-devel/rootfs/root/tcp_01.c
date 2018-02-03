#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#define PF_HOSTOS 44

int main(void)
{
    int         res, err        = 0;
    int         sd              = 0;
    struct      sockaddr_in     addr;
    socklen_t   addrlen         = sizeof(addr);
    int         port            = 2525;
    char        buffer[65535]   = { 0x00 };

    sd = socket(PF_HOSTOS, SOCK_STREAM, 0);
    if (0 > sd){
        fprintf(stderr, "sd: '%d'\n", sd);
        goto fail;
    }

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons (port);
    addr.sin_addr.s_addr = htonl (INADDR_ANY);

    res = connect(
        sd,
        (const struct sockaddr *)&addr,
        addrlen
    );
    if (0 > res){
        fprintf(stderr, "connect: '%d'\n", res);
        goto fail;
    }

    snprintf(buffer, sizeof(buffer),
        "hello fucking world\n"
    );

    fcntl(sd, F_SETFL, O_NONBLOCK);

    res = read(sd, buffer, sizeof(buffer));
    if (0 > res){
        fprintf(stderr, "read failed: '%d'\n", res);
    }

    res = write(sd, buffer, strlen(buffer));
    if (0 > res){
        fprintf(stderr, "write failed: '%d'\n", res);
        goto fail;
    }

    close(sd);

    // all ok
    err = 0;

out:
    return err;
fail:
    if (0 >= err){
        err = -1;
    }
    goto out;
}

