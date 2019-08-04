#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stddef.h>

#define BACKLOGNUM 128

int open_clientfd(char *hostname, char *port);
int open_listenfd(char *port);
void Setsockopt(int s, int level, int optname, const void *optval, int optlen);
void Close(int fd);
void unix_error(char *msg);
void posix_error(int code, char *msg);
void Getaddrinfo(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res);
void gai_error(int code, char *msg);
void *thread(void *arg);
size_t rio_readnb(int fd, char *usrbuf, size_t n);

int main(int argc, char *argv[])
{
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;

    if (argc < 2)
    {
        fprintf(stderr, "usage: %s <port> \n", argv[0]);
        exit(0);
    }

    listenfd = open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        // 连接后产生的描述符，要传给单独的线程进行处理
        connfdp = malloc(sizeof(int));
        *connfdp = accept(listenfd, &clientaddr, &clientlen);
        pthread_create(&tid, NULL, thread, connfdp);
    }
}

void *thread(void *arg)
{
    pthread_t tid;
    int status;
    int connfd = *((int *)arg);
    tid = pthread_self();
    pthread_detach(tid);
    free(arg);

    echo(connfd);
    status = close(connfd);
    if (status < 0)
    {
        unix_error("close connection socket failed!");
    }
    return (void *)0;
}

void echo(int connfd)
{
}


int open_clientfd(char *hostname, char *port)
{
    int clientfd;
    struct addrinfo hints, *listp, *p;

    memset(&hints, 0, sizeof(struct addrinfo));
    // 采用数据流的方式(TCP支持数据流)
    hints.ai_socktype = SOCK_STREAM;
    // 指明必须要有数字类型的Host
    hints.ai_flags = AI_NUMERICSERV;
    /**
     * If the AI_ADDRCONFIG bit is set, getaddrinfo will resolve only if 
     * a global address is configured.
     * 只有当本地机器支持对应的协议，才会返回相应的的global address
     * 只有本地支持ipv4协议，才会返回ipv4地址
     * 只有本地支持ipv6协议，才会返回ipv6地址
     */
    hints.ai_flags |= AI_ADDRCONFIG;
    Getaddrinfo(hostname, port, &hints, &listp);

    for (p = listp; p; p = p->ai_next)
    {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
        {
            continue;
        }
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
        {
            break;
        }
        Close(clientfd);
    }

    /* Clean Up*/
    freeaddrinfo(listp);
    if (!p)
    {
        return -1;
    }
    else
    {
        return clientfd;
    }
}

int open_listenfd(char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;

    /**
     * 如果AI_PASSIVE指定，并且node传递为空，返回的socket address将适合于bind一个socket,
     * 并且用于接受新的连接。返回的地址中包含INADDR_ANY和INADDR_ANY_INIT(接收来自本机的所有连接)
     */
    Getaddrinfo(NULL, port, &hints, &listp);
    for (p = listp; p; p = p->ai_next)
    {
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)))
        {
            continue;
        }
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
        if (bind(listenfd, p->ai_addr, p->ai_addrlen))
        {
            break;
        }
        Close(listenfd);
    }
    freeaddrinfo(listp);
    if (!p)
    {
        return -1;
    }
    if (listen(listenfd, BACKLOGNUM) < 0)
    {
        Close(listenfd);
        return -1;
    }
    return listenfd;
}

void Setsockopt(int s, int level, int optname, const void *optval, int optlen)
{
    int rc;
    if ((rc = setsockopt(s, level, optname, optval, optlen)) < 0)
    {
        unix_error("set socketopt failed!");
    }
}

void Close(int fd)
{
    char buf[128];
    int rc;
    if ((rc = close(fd)) < 0)
    {
        sprintf(buf, "close fd:%d failed!\n", fd);
        unix_error(buf);
    }
}

void unix_error(char *msg)
{
    fprintf(stderr, "%s: %s \n", msg, strerror(errno));
    exit(0);
}

void posix_error(int code, char *msg)
{
    fprintf(stderr, "%s: %s \n", msg, strerror(code));
    exit(0);
}

void Getaddrinfo(const char *node, const char *service,
                 const struct addrinfo *hints, struct addrinfo **res)
{
    int rc;

    if ((rc = getaddrinfo(node, service, hints, res) != 0))
    {
        gai_error(rc, "Getaddrinfo error");
    }
}

void gai_error(int code, char *msg)
{
    fprintf(stderr, "%s:%s \n", msg, gai_strerror(code));
    exit(0);
}
