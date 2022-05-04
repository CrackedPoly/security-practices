#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8900
#define BUFSIZE 2048

FILE *mypopen(char *cmd, const char *type) {
  int pipefd[2];
  pid_t pid;
  if (type[0] != 'r' && type[0] != 'w') {
    // 类型错误
    return NULL;
  }
  if (pipe(pipefd) < 0) {
    // 建立管道失败
    return NULL;
  }
  pid = fork();
  if (pid < 0) {
    // 创建子进程失败
    return NULL;
  }
  if (pid == 0) {
    if (type[0] == 'r') {
      close(pipefd[0]);
      dup2(pipefd[1], STDOUT_FILENO); // 重定向
      close(pipefd[1]);
    } else {
      close(pipefd[1]);
      dup2(pipefd[0], STDIN_FILENO);
      close(pipefd[0]);
    }
    char *argv[] = {cmd, NULL};
    execl("/bin/sh", "sh", "-c", cmd, (char *)0);
    return NULL;
  }
  wait(0);
  if (type[0] == 'r') {
    close(pipefd[1]);
    return fdopen(pipefd[0], "r");
  } else {
    close(pipefd[0]);
    return fdopen(pipefd[1], "w");
  }
}

void exec(const char *command, char *result) {
  FILE *fpRead;

  memset(result, 0, BUFSIZE);
  fpRead = mypopen(command, "r");
  char buf[BUFSIZ] = {0};
  memset(buf, '\0', sizeof(buf));

  while (fgets(buf, BUFSIZ - 1, fpRead) != NULL) {
    strcat(result, buf);
  }
  if (fpRead != NULL)
    pclose(fpRead);
}

int main(int argc, char **argv) {
  struct sockaddr_in server;
  struct sockaddr_in client;
  int len;
  int port;
  int listend;
  int connectd;
  int sendnum;
  int opt;
  int recvnum;
  char send_buf[BUFSIZE];
  char recv_buf[BUFSIZE];
  char command[BUFSIZE];
  char cmd[] = "sh -c ";

  // if (2==argc)

  port = PORT;
  memset(send_buf, 0, sizeof(send_buf));
  memset(recv_buf, 0, sizeof(recv_buf));

  opt = SO_REUSEADDR;

  if (-1 == (listend = socket(AF_INET, SOCK_STREAM, 0))) {
    perror("create listen socket error\n");
    exit(1);
  }
  setsockopt(listend, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); //

  memset(&server, 0, sizeof(struct sockaddr_in));
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(port);

  if (-1 ==
      bind(listend, (struct sockaddr *)&server, sizeof(struct sockaddr))) {
    perror("bind error\n");
    exit(1);
  }
  if (-1 == listen(listend, 5)) {
    perror("listen error\n");
    exit(1);
  }

  while (1) {
    if (-1 == (connectd = accept(listend, (struct sockaddr *)&client,
                                 (socklen_t *)&len))) //建立tcp三次握手
      perror("create connect socket error\n");
    pid_t pid = fork();
    if (pid > 0) {
      while (1) {
        memset(send_buf, 0, BUFSIZ);
        memset(recv_buf, 0, sizeof(recv_buf));
        memset(command, 0, sizeof(command));

        if (0 > (recvnum = recv(connectd, recv_buf, sizeof(recv_buf), 0))) {
          perror("recv error\n");
          continue;
        }
        recv_buf[recvnum] = '\0';
        fprintf(stderr, "recv message:%s", recv_buf);
        sprintf(command, "%s%s", cmd, recv_buf);
        exec(command, send_buf);
        if (*send_buf == '\0') {
          sprintf(send_buf, "command is not vaild,check it please\n");
        }
        fprintf(stderr, "send message:\n%s\n", send_buf);

        sendnum = send(connectd, send_buf, sizeof(send_buf), 0); //
      }
      close(connectd);
      exit(0);
    } else if (pid < 0) {
      perror("fork error\n");
      break;
    }
  }
  close(connectd);
  close(listend);
  return 0;
}
