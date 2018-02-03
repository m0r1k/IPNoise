#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/in.h>

#define PF_HOSTOS 44

int main(void)
{
    int res, fd = -1;
    struct sockaddr_in addr;
    char buffer[1024] = { 0x00 };

    fd = socket(PF_HOSTOS, SOCK_STREAM, 0);
    printf("open socket, fd: '%d'\n", fd);

    addr.sin_port           = htons(2210);
    addr.sin_family         = PF_INET;
    addr.sin_addr.s_addr    = htonl(0x7f000001);

    res = connect (fd, (struct sockaddr *)&addr, sizeof(addr));
    printf("connect, res: '%d'\n", res);

    if (res){
        goto out;
    }

    snprintf(buffer, sizeof(buffer), "test\n");

    res = write(fd, buffer, strlen(buffer));
    printf("write, res: '%d'\n", res);

    res = read(fd, buffer, sizeof(buffer));
    printf("read, res: '%d', buffer: '%s'\n", res, buffer);

    res = read(fd, buffer, sizeof(buffer));
    printf("read, res: '%d', buffer: '%s'\n", res, buffer);

out:
    printf("close fd\n");
    close(fd);
    return 0;
}

