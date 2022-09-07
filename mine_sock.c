//
// Created by robotknik on 07.09.22.
//

#include "include/mine_sock.h"

/*
 * open_clientfd - open connection to server at <hostname, port>
 *   and return a socket descriptor ready for reading and writing.
 *   Returns -1 and sets errno on Unix error.
 *   Returns -2 and sets h_errno on DNS (gethostbyname) error.
 */
int open_clientfd(char *hostname, int port)
{
    int clientfd;
    struct hostent *hp;
    struct sockaddr_in serveraddr;

    if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1; /* check errno for cause of error */

    /* Fill in the server's IP address and port */
    if ((hp = gethostbyname(hostname)) == NULL)
        return -2; /* check h_errno for cause of error */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)hp->h_addr_list[0],
          (char *)&serveraddr.sin_addr.s_addr, hp->h_length);
    serveraddr.sin_port = htons(port);

    /* Establish a connection with the server */
    if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    return clientfd;
}

/*
 * open_listenfd - open and return a listening socket on port
 *     Returns -1 and sets errno on Unix error.
 */
int open_listenfd(int port)
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;

    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;

    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
}


ssize_t mine_recvall(msock *sockfd, char **out_result) {
    char tmp_buf[1000];
    char *result=NULL;
    ssize_t received=0, tmp_received;

    while (sockfd->usable) {
        tmp_received = recv(sockfd->sock_fd, tmp_buf, sizeof(tmp_buf), 0);
        if (tmp_received == -1 || tmp_received == 0) {
            sockfd->usable = false;
            break;
        }
        received += tmp_received;
        result = realloc(result, received + 1);
        memcpy(result+received-tmp_received, tmp_buf, tmp_received);
        result[received] = '\0';
    }
    *out_result = result;
    return received;
}

// Todo: rewrite function
ssize_t mine_read_until(msock *sockfd, char *target, char **out_result) {
    char tmp_buf[50];
    char *result=NULL, *tmp_chr;
    ssize_t received=0, tmp_received;
    ssize_t diff;

    // flush cache
    if (sockfd->cache_size) {
        result = sockfd->recv_cache;
        received = sockfd->cache_size;
        sockfd->recv_cache = NULL;
        sockfd->cache_size = 0;
    }

    while (sockfd->usable) {
        tmp_received = recv(sockfd->sock_fd, tmp_buf, sizeof(tmp_buf), 0);
        if (tmp_received == -1) {
            sockfd->usable = false;
            break;
        }

        received += tmp_received;
        result = realloc(result, received + 1);
        memcpy(result+received-tmp_received, tmp_buf, tmp_received);
        result[received] = '\0';
        if ((tmp_chr = strstr(result, target))) {
            tmp_chr += strlen(target);
            // Number of bytes that should be saved in cache
            diff = received-(tmp_chr-result);
            sockfd->cache_size = diff;
            sockfd->recv_cache = malloc(diff+1);
            memcpy(sockfd->recv_cache, tmp_chr, diff);
            sockfd->recv_cache[diff] = '\0';
            // And  strip-off received buffer
            *tmp_chr = '\0';
            received = result - tmp_chr;
            break;
        }
    }
    *out_result = result;
    return received;
}


ssize_t mine_write(msock *sockfd, char *buf, int size) {
    return send(sockfd->sock_fd, buf, size, 0);
}