#include "myLib/based.h"
#include <pthread.h>
#include <stdio.h>

#define MAX_CLIENTS 100
#define BUFF_SIZE 2048
#define NAME_LEN 32
#define SAI struct sockaddr_in
#define SA struct sockaddr
#define LISTENQ 5
#define forever for (;;)

static _Atomic unsigned int client_count;
static unsigned int uid = 10;

typedef struct {
  SAI addr;
  int sockfd;
  int uid;
  char name[NAME_LEN];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static char *trim_whitespaces(char *str) {
  char *end;

  while ((*str) == ' ')
    str++;

  if (*str == 0)
    return str;

  end = str + strnlen(str, 128) - 1;

  while (end > str && (*end) == ' ')
    end--;

  *(end + 1) = '\0';

  return str;
}

void add_client(client_t *client) {
  pthread_mutex_lock(&mutex);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (!clients[i]) {
      clients[i] = client;
      break;
    }
  }

  pthread_mutex_unlock(&mutex);
}

void remove_client(int uid) {
  pthread_mutex_lock(&mutex);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i]->uid == uid) {
      clients[i] = NULL;
      break;
    }
  }

  pthread_mutex_unlock(&mutex);
}

void send_to_all(char *message, int sender_uid) {
  pthread_mutex_lock(&mutex);

  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (clients[i] != NULL && clients[i]->uid != sender_uid) {
      _write(clients[i]->sockfd, message, strnlen(message, BUFF_SIZE));
    }
  }

  pthread_mutex_unlock(&mutex);
}

void trimstr(char *str, int len) {
  for (int i = 0; i < len; i++) {
    if (str[i] == 10) {
      str[i] = '\0';
    }
  }
}

void *handle_client(void *arg) {
  char buff[BUFF_SIZE];
  char name[NAME_LEN];
  int leave_flag = 0;
  client_count++;
  fflush(stdout);
  client_t *client = (client_t *)arg;
  if (recv(client->sockfd, name, NAME_LEN, 0) <= 0 ||
      strlen(name) >= NAME_LEN - 1 || strlen(name) < 2) {
    printf("icnr name");
    leave_flag = 1;
  } else {
    trimstr(name, NAME_LEN);
    strcpy(client->name, name);
    snprintf(buff, BUFF_SIZE, "%s has joined\n", client->name);
    printf("%s", buff);

    send_to_all(buff, client->uid);
  }

  int n = recv(client->sockfd, buff, BUFF_SIZE, 0);
  bzero(buff, BUFF_SIZE);
  fflush(stdout);

  while (!leave_flag) {
    int n = recv(client->sockfd, buff, BUFF_SIZE, 0);
    trimstr(buff, BUFF_SIZE);

    if (strcmp(buff, "/exit") == 0 || n == 0) {
      snprintf(buff, BUFF_SIZE, "%s has left\n", client->name);
      send_to_all(buff, client->uid);
      printf("%s\n", buff);
      leave_flag = 1;
    } else if (n > 0) {
      if (strnlen(buff, BUFF_SIZE) > 0) {
        char buffcp[BUFF_SIZE];
        strcpy(buffcp, buff);
        snprintf(buff, BUFF_SIZE, "%s: %s\n", client->name, buffcp);
        send_to_all(buff, client->uid);
        printf("%s\n", buff);
      }
    } else {
      perror("Unknown error");
      leave_flag = 1;
    }
    bzero(buff, BUFF_SIZE);
    fflush(stdout);
  }
  close(client->sockfd);
  remove_client(client->uid);
  free(client);
  client_count--;
  pthread_detach(pthread_self());
  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s <port>\n", argv[0]);
    return EXIT_FAILURE;
  }

  char *ip = "127.0.0.1";
  int port = atoi(argv[1]);

  int socketfd;
  int option = 1;

  SAI server_addr, client_addr;
  socklen_t client_len = sizeof(client_addr);

  pthread_t pid;

  server_addr.sin_addr.s_addr = inet_addr(ip);
  server_addr.sin_port = htons(port);
  server_addr.sin_family = AF_INET;

  socketfd = _socket(AF_INET, SOCK_STREAM, 0);
  _setsockopt(socketfd, SOL_SOCKET, (SO_REUSEADDR | SO_REUSEPORT), &option,
              sizeof(option));
  _bind(socketfd, (SA *)&server_addr, sizeof(server_addr));
  _listen(socketfd, LISTENQ);

  printf("Server has successfully started...\n");

  forever {
    int connfd = _accept(socketfd, (SA *)&client_addr, &client_len);

    if (client_count + 1 >= MAX_CLIENTS) {
      char buff[BUFF_SIZE];
      inet_ntop(socketfd, &client_addr.sin_addr, buff, sizeof(buff));
      printf("Maximum clients connected. Rejected connection from: %s\n", buff);
      _close(connfd);
      continue;
    }

    client_t *client = (client_t *)malloc(sizeof(client_t));
    client->addr = client_addr;
    client->sockfd = connfd;
    client->uid = uid++;

    add_client(client);
    char hello[BUFF_SIZE] = "Hello. Firstly enter your name\n";
    _write(client->sockfd, hello, strnlen(hello, BUFF_SIZE));
    pthread_create(&pid, NULL, &handle_client, (void *)client);

    sleep(1);
  }

  return EXIT_SUCCESS;
}
