#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define LISTENQ 5

int _socket(int domain, int type, int protocol);

int _inet_pton(int af, const char *__restrict cp, void *__restrict buf);

int _connect(int fd, const struct sockaddr *addr, socklen_t len);

int _bind(int fd, const struct sockaddr *addr, socklen_t len);

int _listen(int fd, int n);

int _accept(int fd, struct sockaddr *__restrict addr,
            socklen_t *__restrict addr_len);

int _close(int fd);

int _write(int fd, const void *buf, size_t n);
