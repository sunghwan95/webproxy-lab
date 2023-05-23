#include "csapp.h"

int main(int argc, char **argv){
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    if (argc != 2) {
	    fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
	    exit(0);
    }

    // memset 함수 사용하여 hints를 0으로 초기화
    memset(&hints, 0, sizeof(struct addrinfo));                         
    hints.ai_family = AF_INET;       // IPv4 사용
    hints.ai_socktype = SOCK_STREAM; // TCP 사용
    if ((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc));
        exit(1);
    }
    flags = NI_NUMERICHOST;
    
    for (p = listp; p; p = p->ai_next) { // for문을 돌면서 각각의 ip주소 출력.
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
    } 
    Freeaddrinfo(listp);

    exit(0);
}