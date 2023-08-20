# proxylab

- [proxylab](#proxylab)
  - [准备](#准备)
    - [基本概念](#基本概念)
    - [例子](#例子)
    - [调试](#调试)
  - [实验过程](#实验过程)
    - [Basic](#basic)
    - [Concurrency](#concurrency)


## 准备

所有文件均可以从官网上直接下载：[Lab Assignments](http://csapp.cs.cmu.edu/3e/labs.html)

### 基本概念

一个 `HTTP` 报文由请求行 `request line` 和一个或多个请求头 `request header` 组成，二者的格式分别为：

```
method URI version
header-name: header-data
```

* `URI, uniform resource identifier` 是用来标识和定位互联网上的资源，它有 `URL, uniform resource locator` 和 `URN, uniform resource name` 两种形式。换句话说，`url` 和 `urn` 都是 `uri` 的子集
  * `URL` 用于提供资源的标识以及如何访问该资源，通常由协议类型 `http, https, ftp`、主机名 `www.example.com` 和路径名 `/index.html` 构成，例如：`https://www.example.com/index.html`
  * `URN` 是一种资源名称的字符序列，`URN` 与资源的位置无关，而是一个专注于资源唯一性的名称
  * 总的来说，`URL` 用于标识资源的位置，`URN` 用于标识资源的名称；前者提供访问资源的路径，后者提供该资源的唯一名称

* 在请求行中，`Host` 报头**不是必须的**，那么这里有一个问题是：如果没有 `Host` 报头，那么该报文该发向哪个服务器
  * 实际上，我们在浏览器中输入 `URL` 时，就已经指定了服务器的 `IP` 地址，也就是说**对于客户端而言，需要先与服务器取得联系，再发送 `HTTP` 请求报文**

`HTTP` 的请求行是 `URI`，这里通常只会写路径名，也就是形如 `GET /index.html HTTP/1.1`

在本实验中，`proxy` 收到的一定是类似 `GET https://www.example.com/index.html HTTP/1.1` 这种形式，然后以 `GET /index.html HTTP/1.1` 转发出去，并设置相应的字段值

### 例子

解包后的 `./tiny` 中有一个课本当中的 `TINY WBE` 服务器，我们可以简单实验一下：

我们采用一下端口：

```bash
./port-for-user.pl droh
droh: 45806
```

开一个终端，启动 `tiny` 服务器，然后另开一个终端，用 `telnet` 进行连接：

```bash
./tiny 45806

telnet localhost 45806
```

得到以下输出（`telnet` 的请求行输入完毕后需要按两下回车）：

第一次：

```bash
# telnet

telnet localhost 45806
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
GET / HTTP/1.1

HTTP/1.0 200 OK
Server: Tiny Web Server
Content-length: 120
Content-type: text/html

<html>
<head><title>test</title></head>
<body> 
<img align="middle" src="godzilla.gif">
Dave O'Hallaron
</body>
</html>
Connection closed by foreign host.

# tiny

Accepted connection from (localhost, 35116)
GET / HTTP/1.1
```

第二次：

```bash
# telnet

telnet localhost 45806
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
GET /cgi-bin/adder?15000&213 HTTP/1.0

HTTP/1.0 200 OK
Server: Tiny Web Server
Connection: close
Content-length: 115
Content-type: text/html

Welcome to add.com: THE Internet addition portal.
<p>The answer is: 15000 + 213 = 15213
<p>Thanks for visiting!
Connection closed by foreign host.

# tiny

Accepted connection from (localhost, 60314)
GET /cgi-bin/adder?15000&213 HTTP/1.0
```

### 调试

我们主要使用 `curl` 进行调试，只会使用到两个参数 `-v` 和 `--proxy`，前者可以显示更详细的信息，后者表示使用代理服务器

如果直接需要对 `tiny` 进行通讯，那么：

```bash
curl -v http://localhost:45807/home.html
curl -v http://localhost:45807/cgi-bin/adder?50\&70
```

我们可以通过 `proxy` 与 `tiny` 进行通讯，有：

```bash
curl -v --proxy http://localhost:45806 http://localhost:45807/home.html
curl -v --proxy http://localhost:45806 http://localhost:45807/cgi-bin/adder?50\&70
```

## 实验过程

### Basic

在这一部分，我们需要将浏览器传入给 `proxy` 的报文转发给服务器，具体地：

假设浏览器向 `proxy` 发出一个请求头：

```
GET http://www.cmu.edu/hub/index.html HTTP/1.1
```

`proxy` 需要将其转换为：

```
GET /hub/index.html HTTP/1.1
Host: http://www.cmu.edu
User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3)Gecko/20120305 Firefox/10.0.3
Connection: close
Proxy-Connection: close
```

也就是说，我们需要将原先的请求头从完整的 `URL` 改为 `URL` 的后缀，然后分别对 `Host, User-Agent, Connection, Proxy-Connection` 字段进行修改（如果浏览器发出的报文包含请求头，那么我们会忽略对应的请求头）

其次，我们尤其需要注意，在转发时我们的函数需要对文本文件和二进制文件均有效

在底层文件的表示上，二者都是由 `0` 和 `1` 组成的，但文本文件中每一行会有 `\r\n` ，因此我们可以读取文本文件的一行，而对于二进制文件则无法这么做（因为我们无法确定一行到底有多长）

我们关注转发代码：

```c
    forwardfd = Open_clientfd(header.hostname, header.port);
    Rio_readinitb(&server_rio, forwardfd);
    Rio_writen(forwardfd, forwardBuf, strlen(forwardBuf));

    size_t n;
    while((n = Rio_readlineb(&server_rio, forwardBuf, MAXLINE)) != 0) {
        fprintf(stdout, "proxy recived %ld bytes\n", n);
        Rio_writen(fd, forwardBuf, strlen(forwordBuf));
    }
```

这个代码只能针对文本文件运行，这是因为在 `while` 中调用 `Rio_writen` 时，我们是一行一行写回的，而对于二进制文件则没有行这个概念，所以该代码无法对二进制文件生效

将 `while` 循环中的 `Rio_writen` 改为：`Rio_writen(fd, forwardBuf, n);` 即可

这里的 `n` 为实际读取到的字节数，我们用 `n` 来表示每次写回的缓冲器大小

完整代码如下：

```c
#include "csapp.h"
#include <strings.h>
//#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

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

void sighandler(int sig)
{
    ;
}

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
    Signal(SIGPIPE, sighandler);
    
    listenfd = Open_listenfd(argv[1]);
    while(1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
        printf("Accept Connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }
    return 0;
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

    printf("recived header: %s\n", buf);

    sscanf(buf, "%s %s %s", method, uri, version);
    //ignore the case of characters
    if(strcasecmp(method, "GET")) {
        clienterror(fd, method, "501", "Not implemented", "Proxy dose not implement this method\n");
        fprintf(stderr, "%s: Proxy dose not implement this method\n", method);
        return;
    }

    prase_url(uri, &header);
    forwordMessege(forwardBuf, &header, &client_rio);


    printf("-----------------------------------------\n");
    printf("%s\n", forwardBuf);
    printf("host: %s, port: %s, file: %s\n", header.hostname, header.port, header.filename);


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
```

### Concurrency

