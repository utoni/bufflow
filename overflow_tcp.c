#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>

#define MAXLINE 1024
#define BUFLEN 256
#define SERV_PORT 3000
#define LISTENQ 8


int exploitable(int fd)
{
  int n;
  char buf[BUFLEN];

  memset(buf, 0, BUFLEN);
  if (buf[0] == EOF) {
    return (0);
  }

  n = recv(fd, buf, MAXLINE, 0);
  fprintf(stderr, "Received string(%d): %s", n, buf);
  return (n);
}

int main (int argc, char **argv)
{
  int listenfd, connfd, n;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) <0) {
    perror("socket");
    exit(1);
  }

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(SERV_PORT);

  if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
    perror("bind");
    exit(2);
  }
  if (listen(listenfd, LISTENQ) != 0) {
    perror("listen");
    exit(3);
  }

  fprintf(stderr, "Server running on port %d ...\n", SERV_PORT);
  for (;;) {
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
    if (connfd < 0) break;
    fprintf(stderr, "Client connected.\n");
    while ((n = exploitable(connfd)) > 0)  {
    }
    fprintf(stderr, "disconnecting client ..\n");
    close(connfd);
  }

  fprintf(stderr, "shutdown ..\n");
  close(listenfd);
  return (0);
}
