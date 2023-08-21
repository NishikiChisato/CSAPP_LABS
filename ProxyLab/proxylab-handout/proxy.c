#include <strings.h>
#include "csapp.h"
#include "sbuf.h"
//#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

//the number of thread
#define NTHREAD 16
#define SBUF_SIZE 32
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

typedef struct requestHeaders {
    char hostname[MAXLINE];
    char port[MAXLINE];
    char filename[MAXLINE];
}requesthdrs;

void doit(int fd);
void prase_url(char* uri, requesthdrs* header);
void read_requesthdrs(rio_t* rp);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
void forwordMessege(char* buf, requesthdrs* headers, rio_t* rp);
void* thread(void* vargp);

void sighandler(int sig)
{
    ;
}

//golbal variable
sbuf_t sbuf;

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
    char hostname[MAXLINE], port[MAXLINE];
    pthread_t tid;

    Signal(SIGPIPE, sighandler);
    
    listenfd = Open_listenfd(argv[1]);

    sbuf_init(&sbuf, SBUF_SIZE);
    for(int i = 0; i < NTHREAD; i ++) {
        Pthread_create(&tid, NULL, thread, NULL);
    }

    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accept Connection from (%s, %s)\n", hostname, port);
        sbuf_insert(&sbuf, connfd);
    }
    return 0;
}

void* thread(void* vargp)
{
    Pthread_detach(Pthread_self());
    while(1) {
        int connfd = sbuf_remove(&sbuf);
        doit(connfd);
        Close(connfd);
        return NULL;        
    }
}

void doit(int fd)
{
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char forwardBuf[MAXLINE];
    requesthdrs header;
    int forwardfd;
    rio_t client_rio, server_rio;
    Rio_readinitb(&client_rio, fd);

    Rio_readlineb(&client_rio, buf, MAXLINE);

    //printf("recived header: %s\n", buf);

    sscanf(buf, "%s %s %s", method, uri, version);
    //ignore the case of characters
    if(strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not implemented", "Proxy dose not implement this method\n");
        fprintf(stderr, "%s: Proxy dose not implement this method\n", method);
        return;
    }

    prase_url(uri, &header);
    forwordMessege(forwardBuf, &header, &client_rio);


    // printf("-----------------------------------------\n");
    // printf("%s\n", forwardBuf);
    // printf("host: %s, port: %s, file: %s\n", header.hostname, header.port, header.filename);


    forwardfd = Open_clientfd(header.hostname, header.port);
    Rio_readinitb(&server_rio, forwardfd);
    Rio_writen(forwardfd, forwardBuf, strlen(forwardBuf));

    size_t n;
    while((n = Rio_readlineb(&server_rio, forwardBuf, MAXLINE)) != 0) {
        fprintf(stdout, "proxy recived %ld bytes\n", n);
        Rio_writen(fd, forwardBuf, n);
    }

    Close(forwardfd);
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

void forwordMessege(char* buf, requesthdrs* headers, rio_t* rp)
{
    char tmp[MAXLINE], getLine[MAXLINE], hostLine[MAXLINE];
    char userAgentLine[MAXLINE], connectionLine[MAXLINE], proxyConnectionLine[MAXLINE];
    sprintf(getLine, "GET %s HTTP/1.0\r\n", headers->filename);
    sprintf(hostLine, "Host: %s\r\n", headers->hostname);
    sprintf(userAgentLine, "User-Agent: %s", user_agent_hdr);
    sprintf(connectionLine, "Connection: close\r\n");
    sprintf(proxyConnectionLine, "Proxy-Connection: close\r\n");

    char* ptr = buf;
    sprintf(ptr, getLine); ptr += strlen(getLine);
    sprintf(ptr, hostLine); ptr += strlen(hostLine);
    sprintf(ptr, userAgentLine); ptr += strlen(userAgentLine);
    sprintf(ptr, connectionLine); ptr += strlen(connectionLine);
    sprintf(ptr, proxyConnectionLine); ptr += strlen(proxyConnectionLine);

    Rio_readlineb(rp, tmp, MAXLINE);
    while(strcmp(tmp, "\r\n")) {
        if(!strncasecmp(tmp, "Host", strlen("Host")) || !strncasecmp(tmp, "User-Agent", strlen("User-Agent")) 
           || !strncasecmp(tmp, "Connection", strlen("Connection")) 
           || !strncasecmp(tmp, "Proxy-Connection", strlen("Proxy-Connection"))) {
                Rio_readlineb(rp, tmp, MAXLINE);
                continue;
           }
        sprintf(ptr, tmp); ptr += strlen(tmp);
        Rio_readlineb(rp, tmp, MAXLINE);
    }
    sprintf(ptr, "\r\n");
}

void prase_url(char* uri, requesthdrs* header)
{
    //example: GET http://www.cmu.edu/hub/index.html HTTP/1.1
    char* ptr = strstr(uri, "//");
    if(ptr == NULL) {
        //example: GET /index.html HTTP/1.1 or GET / HTTP/1.1
        char* idx = index(uri, '/');
        strcat(header->filename, idx);
        strcpy(header->port, "80");
        return;
    } else {
        //GET http://www.cmu.edu/hub/index.html HTTP/1.1 or GET http://www.cmu.edu:80/hub/index.html HTTP/1.1
        char* idx = index(ptr + 2, '/');
        char* port = index(ptr + 2, ':');
        if(port) {
            int portNum;
            //example: GET http://www.cmu.edu:80/hub/index.html HTTP/1.1
            sscanf(port + 1, "%d%s", &portNum, header->filename);
            sprintf(header->port, "%d", portNum);
            *port = '\0';
        } else {
            //example: GET http://www.cmu.edu/hub/index.html HTTP/1.1
            sscanf(idx, "%s", header->filename);
            strcpy(header->port, "80");
            *idx = '\0';
        }
        strcpy(header->hostname, ptr + 2);
    }
}

void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    Rio_writen(fd, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Tiny Error</title>");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    Rio_writen(fd, buf, strlen(buf));
    sprintf(buf, "<hr><em>The Tiny Web server</em>\r\n");
    Rio_writen(fd, buf, strlen(buf));
}