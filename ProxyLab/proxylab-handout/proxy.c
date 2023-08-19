#include "csapp.h"
//#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void doit(int fd);
void prase_url(char* uri, char* hostname_port, char* filename);
void getHostNamePortNumber(char* hostname_port, char* hostname, char* port);
void read_requesthdrs(rio_t* rp);

int main(int argc, char* argv [])
{
    //printf("%s", user_agent_hdr);
    if(argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    
    listenfd = Open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        fprintf(stdout, "success connect\n");
        doit(connfd);
        Close(connfd);
    }
    return 0;
}


void doit(int fd)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname_port[MAXLINE], filename[MAXLINE];
    char hostname[MAXLINE], port[MAXLINE];
    int forwardfd;
    rio_t client_rio, server_rio;
    Rio_readinitb(&client_rio, fd);
    Rio_readlineb(&client_rio, buf, MAXLINE);
    sscanf(buf, "%s %s %s", method, uri, version);
    //ignore the case of characters
    if(strcasecmp(method, "GET")) {
        fprintf(stderr, "client error, file: %s, line %d\n", __FILE__, __LINE__);
        return;
    }
    //read_requesthdrs(&rio);
    prase_url(uri, hostname_port, filename);
    //get hostname and port number
    getHostNamePortNumber(hostname_port, hostname, port);

    forwardfd = Open_clientfd(hostname, port);
    Rio_readinitb(&server_rio, forwardfd);
    
}

void read_requesthdrs(rio_t* rp)
{
    char buf[MAXLINE];

    Rio_readlineb(rp, buf, MAXLINE);
    while(strcmp(buf, "\r\n")) {
        Rio_readlineb(rp, buf, MAXLINE);
    }
    return;
}

void prase_url(char* uri, char* hostname_port, char* filename)
{
    char* ptr;
    ptr = index(uri, '/');
    ptr += 2;
    ptr = index(ptr, '/');
    //not found
    if(ptr == NULL) {
        strcpy(hostname_port, uri);
        strcpy(filename, "");
    } else {
        size_t dis = ptr - uri;
        for(int i = 0; i < dis; i ++) {
            hostname_port[i] = ptr[i];
        }
        hostname_port[dis] = '\0';
        //strcpy(filename, ".");
        strcpy(filename, ptr);
    }
}

void getHostNamePortNumber(char* hostname_port, char* hostname, char* port)
{
    char* ptr = index(hostname_port, ':');
    if(ptr == NULL) {
        strcpy(port, "80");
        strcpy(hostname, hostname_port);
    } else {
        strcpy(port, ptr + 1);
        size_t dis = ptr - hostname_port;
        for(int i = 0; i < dis; i ++) {
            hostname[i] = hostname_port[i];
        }
        hostname[dis] = '\0';
    }

}