#include "csapp.h"

int main(int argc, char **argv){
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3){
	    fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
	    exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port); // 클라이언트 소켓의 fd를 가져옴.
    Rio_readinitb(&rio, clientfd); // rio구조체 초기화

    while (Fgets(buf, MAXLINE, stdin) != NULL) {
	    Rio_writen(clientfd, buf, strlen(buf)); // buf의 내용을 서버로 보냄
	    Rio_readlineb(&rio, buf, MAXLINE);
	    Fputs(buf, stdout);
    }
    Close(clientfd); // 클라이언트 커널이 열었던 모든 식별자들을 자동으로 닫아주지만, 명시적으로 닫아주는 것이 올바른 프로그래밍이다.
    exit(0);
}