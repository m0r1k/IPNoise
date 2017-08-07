/*
 *  Roman E. Chechnev
 */

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

int os_open_socket(int domain, int type, int protocol)
{
	return socket(domain, type, protocol);
}

int os_read_socket(int fd, void *buf, int len)
{
	int n = read(fd, buf, len);

	if (n < 0)
		return -errno;
	return n;
}

int os_write_socket(int fd, const void *buf, int len)
{
	int n = write(fd, (void *) buf, len);

	if (n < 0)
		return -errno;
	return n;
}

void os_close_socket(int fd)
{
	close (fd);
}

int os_socket_connect(int socket, const struct sockaddr *address,
	int address_len)
{
    int n = connect(socket, address, address_len);
	if (n < 0)
		return -errno;
	return n;
}

int os_socket_bind(int socket, const struct sockaddr *address,
	int address_len)
{
	return bind (socket, address, address_len);
}

int os_socket_accept(int socket, struct sockaddr *address,
	int *address_len)
{
	return accept (socket, address, (socklen_t *)address_len);
}

int os_socket_listen(int socket, int backlog)
{
    return listen (socket, backlog);
}

int os_socket_setopt(int socket, int level, int optname, const void *optval, int optlen)
{
    return setsockopt (socket, level, optname, optval, (socklen_t)optlen);
}

int os_socket_getopt(int socket, int level, int optname, void *optval, int *optlen)
{
    return getsockopt(socket, level, optname, optval, (socklen_t *)optlen);
}

int os_socket_recvfrom(int s, void *buf, int len, int flags,
    struct sockaddr *from, int *fromlen)
{
    return recvfrom(s, buf, len, flags, from, fromlen);
}

int os_socket_recvmsg(int socket, void *msg,
	int flags)
{
	return recvmsg(socket, msg, flags);
}

int os_socket_sendto(int s, const void *msg, int len, int flags,
    const struct sockaddr *to, int tolen)
{
    return sendto(s, msg, len, flags, to, tolen);
}

int os_socket_sendmsg(int socket, void *msg,
	int flags)
{
	return sendmsg(socket, msg, flags);
}


