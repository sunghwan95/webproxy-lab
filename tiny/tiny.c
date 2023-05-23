/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}

void doit(int fd){
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  rio_t rio;

  /* 요청 라인과 헤더 읽기 */
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  if(strcasecmp(method, "GET")){ // 요청헤더가 GET이 아니면 에러 처리
    clienterror(fd, method, "501", "Not implemented", "Tiny doesn't implement this method");
    return;
  }
  read_requesthdrs(&rio);

  is_static = parse_uri(uri, filename, cgiargs); // 요청contents의 정적여부 파악
  if(stat(filename, &sbuf) < 0){ // 파일이 디스크에 없으면
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  if(is_static){ // 정적 콘텐츠
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
      return;
    }
    serve_static(fd, filename, sbuf.st_size);
  }else{// 동적 콘텐츠
    if(!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)){
      clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
      return;
    }
    serve_dynamic(fd, filename, cgiargs);
  }
}

void clienterror(int fd, char* cause, char* errnum, char* shortmsg, char* longmsg){
  // buf : HTTP 응답 헤더, body : HTML 응답의 본문인 문자열
  char buf[MAXLINE], body[MAXLINE];

  /* HTTP 응답 본문 생성 */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* HTTP 응답 출력 */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));                              
  Rio_writen(fd, body, strlen(body));        
}

void read_requesthdrs(rio_t* rp){
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  while(strcmp(buf, "\r\n")){// 버퍼에서 읽은 줄이 '\r\n'이 아닐 때 까지 루프돌기
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf); // 헤더 필드 출력
  }
  return;
}

int parse_uri(char* uri, char* filename, char* cgiargs){
  char *ptr;

  if(!strstr(uri, "cgi-bin")){
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);
    if(uri[strlen(uri)-1] == '/'){
      strcat(filename, "home.html");
    }
    return 1;
  }else{
    ptr = index(uri, '?');
    if(ptr){
      strcpy(cgiargs, ptr+1);
      *ptr = '\0';
    }else{
      strcpy(cgiargs, "");
    }
    strcpy(filename, ".");
    strcat(filename, uri);
    return 0;
  }
}

void serve_static(int fd, char* filename, int filesize){
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];

  get_filetype(filename, filetype); // 파일 타입 검사
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer : Tiny Web Server\r\n", buf);
  sprintf(buf, "%sConnection : close\r\n", buf);
  sprintf(buf, "%sContent-length : %d\r\n", buf, filesize);
  sprintf(buf, "%sContent-type : %s\r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf)); // buf에서 fd로 전송
  printf("Response headers : \n");
  printf("%s", buf);

  srcfd = Open(filename, O_RDONLY, 0);
  /*
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); // 파일을 가상메모리에 매핑
  Close(srcfd);
  Rio_writen(fd, srcp, filesize);
  Munmap(srcp, filesize); // 매핑된 가상메모리 해제
  */

  // 과제 11.9
  srcp = (char*)malloc(filesize);
  Rio_readn(srcfd, srcp, filesize);
  Close(srcfd); // 메모리 누수 방지를 위해 더 이상 필요없는 식별자는 닫아버리기
  Rio_writen(fd, srcp, filesize); // 주소 srcp에서 시작하는 filesize를 클라이언트의 연결 식별자인 fd로 복사
  free(srcp); // 매핑된 가상메모리 주소 반환
}

void get_filetype(char* filename, char* filetype){
  if(strstr(filename, ".html")){
    strcpy(filetype, "text/html");
  }else if(strstr(filename, ".gif")){
    strcpy(filetype, "image/gif");
  }else if(strstr(filename, ".png")){
    strcpy(filetype, "image/png");
  }else if(strstr(filename, ".jpg")){
    strcpy(filetype, "image/jpeg");
  }else{
    strcpy(filetype, "text/plain");
  }
}

void serve_dynamic(int fd, char* filename, char* cgiargs){
  char buf[MAXLINE], *emptylist[] = {NULL};

  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Server : Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));

  if(Fork() == 0){ // fork라는 시스템 콜을 통해 자식 프로세스 생성
    setenv("QUERY_STRING", cgiargs, 1); // QUERY_STRING 환경 변수를 URI에서 추출한 CGI 인수로 설정
    Dup2(fd, STDOUT_FILENO); // 자식 프로세스의 표준 출력을 클라이언트 소켓에 연결된 fd로 변경
    Execve(filename, emptylist, environ); // 현재 프로세스의 이미지를 filename 프로그램으로 대체
  }
  Wait(NULL);
}