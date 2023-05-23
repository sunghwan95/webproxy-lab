#include "csapp.h"

void echo(int connfd){
    size_t n; 
    char buf[MAXLINE]; 
    rio_t rio;

    Rio_readinitb(&rio, connfd); // rio 구조체 초기화
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { 
        printf("server received %d bytes\n", (int)n);
	    Rio_writen(connfd, buf, n); // buf에 있는 데이터를 connfd 소켓으로 보냄.
    }
}