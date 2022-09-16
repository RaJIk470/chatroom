#include "../myLib/based.h"
#include <stdio.h>
#include <sys/socket.h>

#define BUFF_SIZE 4096

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

  while ((n = read(socketfd, buff, BUFF_SIZE)) > 0) {
    buff[n] = 0;
    fputs(buff, stdout);
    fflush(stdout);
  }
  return 0;
}
