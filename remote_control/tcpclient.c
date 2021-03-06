#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define BUFSIZE 2048

void print_usage(char *cmd) {
  fprintf(stderr, " %s usage:\n", cmd);
  fprintf(stderr, "%s IP_Addr [port]\n", cmd);
}

int main(int argc, char **argv) {
  struct sockaddr_in server;
  int ret;
  int len;
  int port;
  int sockfd;
  int sendnum;
  int recvnum;
  char send_buf[2048];
  char recv_buf[2048];

  if ((2 > argc) || (argc > 3)) {
    print_usage(argv[0]);
    exit(1);
  }
  if (3 == argc) {
    port = atoi(argv[2]);
  }
  if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0))) {
    perror("can not create socket\n");
    exit(1);
  }

  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = inet_addr(argv[1]);
  server.sin_port = htons(port);
  if (0 > (ret = connect(sockfd, (struct sockaddr *)&server,
                         sizeof(struct sockaddr)))) {
    perror("connect error");
    close(sockfd);
    exit(1);
  }

  // memset(send_buf,0,2048);
  // memset(recv_buf,0,2048);

  while (1) {
    memset(send_buf, 0, BUFSIZE);
    memset(recv_buf, 0, BUFSIZE);

    printf("%s>", argv[1]);
    fgets(send_buf, 2048, stdin);
    if (0 == strcmp(send_buf, "quit\n")) {
      printf("Terminate!\n");
      close(sockfd);
      exit(1);
    }
    if (0 > (len = send(sockfd, send_buf, strlen(send_buf), 0))) {
      perror("send data error\n");
      close(sockfd);
      exit(1);
    }
    if (0 > (len = recv(sockfd, recv_buf, 2048, 0))) {
      perror("recv data error\n");
      close(sockfd);
      exit(1);
    }

    recv_buf[len] = '\0';

    printf("%s", recv_buf);
  }
  close(sockfd);
}
