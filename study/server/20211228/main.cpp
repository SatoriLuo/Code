#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <memory.h>

#define SERV_PORT 9877
#define SERV_PORT_STR "9877"
#define LISTENNQ 1024
#define MAXLINE 4096

void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    while( (pid = waitpid(-1,&stat,WNOHANG)) > 0 )
    {
        printf("child %d terminated.\n",pid);
    }
    return;
}

ssize_t myWrite(int sockfd,const void* pBuf,size_t bufLen)
{
    size_t nleft = bufLen;
    ssize_t nwritten;
    char* ptr = (char*)pBuf;

    while(nleft > 0)
    {
        if((nwritten = write(sockfd,ptr,nleft)) <= 0)
        {
            if(nwritten < 0 && errno == EINTR)
                continue;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return bufLen;
}

void str_echo(int sockfd)
{
    ssize_t n;
    char buf[MAXLINE];

    while(1)
    {
        while((n = read(sockfd,buf,MAXLINE)) > 0)
            myWrite(sockfd,buf,n);
        if(n < 0 && errno == EINTR)
            continue;
        else if(n < 0)
        {
            printf("str_echo error");
            return;
        }
        else
            return;
    }
}

int main(int argc,char** argv)
{
    int listenfd,connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr,servaddr;
    void sig_child(int);

    //创建socket,指定为IPv4版本的TCP字节流
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd < 0)
    {
        printf("create listen socket faile.\n");
        return 0;
    }
    memset(&servaddr,0,sizeof(servaddr));
    //接受任何ip的连接，端口号为9877
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    //绑定socket
    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
    {
        printf("bind socket fail.\n");
        return 0;
    }

    //监听socket
    if(listen(listenfd,LISTENNQ) < 0)
    {
        printf("listen socket fail.\n");
        return 0;
    }
    
    //注册子进程退出消息的信号处理函数
    signal(SIGCHLD,sig_chld);

    while(1)
    {
        clilen = sizeof(cliaddr);
        if( (connfd = accept(listenfd,(struct sockaddr*)&servaddr,&clilen)) < 0)
        {
            if(errno == EINTR)
                continue;
            else
                printf("accept error.\n");
        }
        if( (childpid = fork()) == 0)
        {
            //子进程处理连接，回射客户消息
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        //父进程关闭连接套接字
        if(close(connfd) < 0)
            printf("close connfd error");
    }
    return 0;
}