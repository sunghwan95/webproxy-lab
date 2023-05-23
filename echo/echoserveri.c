/* Iterative server */
// 한 번에 한 개의 클라이언트를 반복해서 실행하는 서버
#include "csapp.h"

void echo(int connfd);

int main(int argc, char **argv){
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(0);
    }

    listenfd = Open_listenfd(argv[1]);// 소켓 생성 후 연결 대기 상태로 만듦.
    while (1) {
	    clientlen = sizeof(struct sockaddr_storage); 
	    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0); // 클라이언트의 호스트 이름, 포트번호 get
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
	    echo(connfd); // 클라이언트와 에코통신 수행.
	    Close(connfd); // 통신이 완료 되면 소켓을 닫음.
    }
    exit(0);
}