#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>

#include "include/misc.h"
#include "include/mine_sock.h"



//prints error message and quits
void myerror(){
    perror("server");
    exit(1);
}

void handle_request(int clientfd) {
    int serverfd;
    ssize_t n;
    char *buffer = NULL;
    char *header = NULL;

    msock cur_client = {
            .sock_fd = clientfd,
            .usable = true,
            .recv_cache = NULL,
            .cache_size = 0
    };

    // In general, request looks like:
    // <HTTP method> <request URL> <HTTP version>\r\nHost: <Domain>\r\nUser-Agent: <UA>"...
    // Buffer should contain only first line
    mine_read_until(&cur_client, "\r\n", &buffer);
    mine_read_until(&cur_client, "\r\n\r\n", &header);

//    printf("\nThe request in buffer is\n%s\n", buffer);

    if (strcmp(buffer, "EXIT") == 0)
        return;

    char *method = strtok(buffer, " ");
    char *url = strtok(NULL, " ");
    char *version = strtok(NULL, " ");

    printf("Method: %p : %s\n", method, method);
    printf("URL: %p : %s\n", url, url);
    printf("Version: %p : %s\n", version, version);

    // Skip "http[s]:// part
    if (strncmp(url, "https", 5) == 0) {
        url += 8;
    } else if (strncmp(url, "http", 4) == 0) {
        url += 7;
    } else {
        TRACE("Malformed request from client: %d\t Unable to read url\n", cur_client.sock_fd);
        mine_write(&cur_client, "Malformed request", strlen("Malformed request"));
        return;
    }

    char *url_path = strstr(url, "/");
    if (url_path) {
        *url_path = '\0';
        url_path++;
    } else {
        url_path = " ";
    }

    int port = 80;
    // Parse port from url (if present)
    char *tmp_url = strstr(url, ":");
    if (url_path > tmp_url && tmp_url) {
        port = atoi(tmp_url);
        *tmp_url = '\0';
    }

    // When url and port stripped, it's safe to assume that url == host
    char* host = url;

    if((serverfd = open_clientfd(host, port)) < 0)
        myerror();
    else
        TRACE("Succesfully connected to server %s:%d\n", host, port);

    msock cur_server = {
            .sock_fd = serverfd,
            .usable = true,
            .recv_cache = NULL,
            .cache_size = 0
    };

    //send GET request to server
    int request_size = strlen(url) + strlen(url_path) + strlen(header) + 40;
    char *request =  malloc(request_size);
    if (tmp_url) {

    }
    sprintf(request, "GET http://%s:%d/%s HTTP/1.1\r\n%s\r\n\r\n", url, port, url_path, header);

    printf("Are going to send request: ---\n");
    print_headers(request);
    printf("----\n");
    mine_write(&cur_server, request, request_size);


    //receive reply
    free(buffer);
    buffer = NULL;

    while((n = mine_recvall(&cur_server, &buffer)) > 0 ) {
        print_headers(buffer);
        mine_write(&cur_client, buffer, n);
        free(buffer);
        buffer = 0;
    }

    close(serverfd);
    free(header);
    free(request);
}

int main(int argc, char **argv)
{
    int clientfd;
    struct sockaddr_in clientaddr;
    int clientlen;
    int port;
    int sockfd;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
        exit(0);
    }

    port = atoi(argv[1]);

    //open connection on given port, bind, listen
    if((sockfd=open_listenfd(port)) < 0)
        myerror();

    //infinite server loop
    while(1) {
        clientlen = sizeof(clientaddr);
        if((clientfd = accept(sockfd, (SA *)&clientaddr, &clientlen)) < 0)
            myerror();

        int new_pid = fork();
        if (!new_pid) {
            handle_request(clientfd);
            close(clientfd);
            break;
        }
        close(clientfd);
    }

    return 0;
}

