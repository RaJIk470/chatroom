#include "../myLib/based.h"
#include "clients.c"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>

#define BUFF_SIZE 4096

struct thread_data {
  int socketfd;
  struct sockaddr_in serv_addr;
};

void read_from_server(void *args);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("дурачк веде апишк\n");
    return 1;
  }
  int socketfd, n;
  char buff[BUFF_SIZE + 1];
  socketfd = _socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_port = htons(10000);
  serv_addr.sin_family = AF_INET;
  char *ipaddr = argv[1];
  //_inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
  _inet_pton(AF_INET, ipaddr, &serv_addr.sin_addr);

  _connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  struct thread_data data;
  data.socketfd = socketfd;
  data.serv_addr = serv_addr;
  pthread_t pid;
  pthread_create(&pid, NULL, (void *)&read_from_server, (void *)&data);

  char message[BUFF_SIZE] = "";
  while (strcmp(message, "exit\n") != 0) {
    if (strnlen(message, BUFF_SIZE) > 0)
      _write(socketfd, message, BUFF_SIZE);
    fgets(message, BUFF_SIZE, stdin);
  }

  return 0;
}

void read_from_server(void *args) {
  struct thread_data *data = (struct thread_data *)args;
  int socketfd, n;
  char buff[BUFF_SIZE + 1];
  // socketfd = _socket(AF_INET, SOCK_STREAM, 0);
  socketfd = data->socketfd;
  struct sockaddr_in serv_addr = data->serv_addr;
  //_connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  while ((n = read(socketfd, buff, BUFF_SIZE)) > 0) {
    buff[n] = 0;
    printf("message from server -> %s\n", buff);
    fputs(buff, stdout);
    fflush(stdout);
  }
}
