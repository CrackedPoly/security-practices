#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


void print_usage(char *cmd) {
  fprintf(stderr, " %s usage:\n", cmd);
  fprintf(stderr, "%s IP_Addr [start_port] [end_port]\n", cmd);
}

int main(int argc, char **argv) {
  struct sockaddr_in server;
  struct servent *sptr;
  int ret;
  int start_port;
  int end_port;
  int sockfd;

  if (4 != argc) {
    print_usage(argv[0]);
    exit(1);
  }
  start_port = atoi(argv[2]);
  end_port = atoi(argv[3]);

  for (int i = start_port; i <= end_port; i++) {
    if (-1 == (sockfd = socket(AF_INET, SOCK_STREAM, 0))) {
      perror("can not create socket\n");
      exit(1);
    }

    memset(&server, 0, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(i);

    // printf("connecting port %d\n", i);
    if (0 == (ret = connect(sockfd, (struct sockaddr *)&server,
                            sizeof(struct sockaddr)))) {
      sptr = getservbyport(htons(i), "tcp");
      if (sptr != NULL) {
        printf("port:%5d | service: %s\n", i, sptr->s_name);
      } else {
        printf("port:%5d | service: unknown\n", i);
      }
    }
    close(sockfd);
  }
  return 0;
}
