#include "../myLib/based.h"
#include <pthread.h>
#include <stdio.h>

struct client {
  int connfd;
  struct sockaddr_in addr;
};

struct thread_data {
  int connfd;
  struct client *client;
};

int client_count = 0;
struct client clients[100];

#define BUFF_SIZE 4096

// void do_work(int connfd, struct client *c);
void do_work(void *args);
void send_message_to_all(char *message, struct client *messageSender,
                         struct client *clients);

int main(void) {
  pid_t pid;
  int listenfd, connfd;

  listenfd = _socket(AF_INET, SOCK_STREAM, 0);

  // IMPORTANT !!!! ! ! !!
  int reuse = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(10000);
  addr.sin_family = AF_INET;

  _bind(listenfd, (struct sockaddr *)&addr, sizeof(addr));
  _listen(listenfd, LISTENQ);

  struct sockaddr_in client_addr;
  socklen_t addr_len = sizeof(client_addr);

  for (;;) {
    connfd = _accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
    printf("connfd -> %d\n", connfd);
    clients[client_count].connfd = connfd;
    clients[client_count++].addr = client_addr;

    struct thread_data data;
    data.connfd = connfd;
    data.client = &clients[client_count - 1];
    pthread_t pid;
    pthread_create(&pid, NULL, (void *)do_work, (void *)&data);
    // do_work(connfd, clients[client_count - 1]);
  }
  return 0;
}

void do_work(void *args) {
  // void do_work(int connfd, struct client *c) {
  struct thread_data *data = (struct thread_data *)args;
  int connfd = data->connfd;
  struct client *client = data->client;
  char buff[BUFF_SIZE];
  for (;;) {
    int n;
    if ((n = read(connfd, buff, sizeof(buff))) != 0) {
      buff[n] = 0;
      _write(connfd, buff, sizeof(buff));
      send_message_to_all(buff, client, clients);
    }
    // snprintf(buff, BUFF_SIZE, "Hello");
    //_write(connfd, buff, BUFF_SIZE);
  }
  _close(connfd);
}

void send_message_to_all(char *message, struct client *messageSender,
                         struct client *clients) {
  int sockfd = _socket(AF_INET, SOCK_STREAM, 0);
  char buff[BUFF_SIZE];
  for (int i = 0; i < client_count; i++) {
    int connfd = clients[i].connfd;
    if (messageSender->connfd != connfd) {
      printf("messageSender -> %d\n current -> %d\n", messageSender->connfd,
             connfd);
      // if (messageSender->addr.sin_addr.s_addr !=
      //     clients[i].addr.sin_addr.s_addr) {
      //_connect(sockfd, (struct sockaddr *)&clients[i], sizeof(clients[i]));
      snprintf(buff, BUFF_SIZE, message, BUFF_SIZE);
      //_write(sockfd, buff, BUFF_SIZE);
      _write(connfd, buff, BUFF_SIZE);
    }
  }
}
