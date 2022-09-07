//
// Created by robotknik on 07.09.22.
//

#ifndef REROUTER_MINE_SOCK_H
#define REROUTER_MINE_SOCK_H

#include <stdbool.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct sockaddr SA;
#define LISTENQ  1024  /* second argument to listen() */

struct {
    int sock_fd;
    bool usable;
    char *recv_cache;
    ssize_t cache_size;
} typedef msock;

int open_clientfd(char *hostname, int port);
int open_listenfd(int port);
ssize_t mine_recvall(msock *sockfd, char **out_result);
ssize_t mine_read_until(msock *sockfd, char *target, char **out_result);
ssize_t mine_write(msock *sockfd, char *buf, int size);

#endif //REROUTER_MINE_SOCK_H
