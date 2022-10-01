#include "myLib/based.h"
#include <form.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdio.h>

#define BUFF_SIZE 4096

struct thread_data {
  int socketfd;
  struct sockaddr_in serv_addr;
};

void read_from_server(void *args);
void read_input(void *arg);
void destroy_win(WINDOW *win);
WINDOW *create_newwin(int y, int x, int starty, int startx, int border);
void initwin();

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

WINDOW *output_win;
WINDOW *input_win;
FIELD *input_field[2];
FORM *form;

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Usage: %s <ip> <port>\n", argv[1]);
    return 1;
  }
  int socketfd, n;
  char buff[BUFF_SIZE + 1];
  socketfd = _socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serv_addr;
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_port = htons(atoi(argv[2]));
  serv_addr.sin_family = AF_INET;
  char *ipaddr = argv[1];
  _inet_pton(AF_INET, ipaddr, &serv_addr.sin_addr);

  _connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

  struct thread_data data;
  data.socketfd = socketfd;
  data.serv_addr = serv_addr;
  pthread_t pid;

  initwin();

  pthread_create(&pid, NULL, (void *)&read_from_server, (void *)&data);

  pthread_create(&pid, NULL, (void *)&read_input, (void *)&socketfd);

  pthread_join(pid, NULL);

  endwin();
  return 0;
}

void read_from_server(void *args) {
  struct thread_data *data = (struct thread_data *)args;
  int socketfd, n;
  char buff[BUFF_SIZE + 1];
  socketfd = data->socketfd;
  struct sockaddr_in serv_addr = data->serv_addr;

  while ((n = read(socketfd, buff, BUFF_SIZE)) > 0) {
    buff[n] = 0;
    wprintw(output_win, buff);
    wrefresh(output_win);
  }
}

void read_input(void *arg) {
  int ch;
  int *socketfd = (int *)arg;

  char message[BUFF_SIZE];

  while ((ch = getch()) != KEY_F(1)) {
    switch (ch) {
    case KEY_BACKSPACE:
      form_driver(form, REQ_DEL_PREV);
      break;
    case 13:
      form_driver(form, REQ_VALIDATION);
      snprintf(message, BUFF_SIZE, "%s", field_buffer(input_field[0], 0));
      trim_whitespaces(message);
      wprintw(output_win, "%s\n", message);
      wrefresh(output_win);
      if (strcmp(message, "/exit") != 0)
        if (strnlen(message, BUFF_SIZE) > 0)
          _write(*socketfd, message, BUFF_SIZE);
      set_field_buffer(input_field[0], 0, "");
      form_driver(form, REQ_VALIDATION);
      break;
    default:
      form_driver(form, ch);
      break;
    }
  }

  /*
  while (strcmp(message, "/exit") != 0) {
    fgets(message, BUFF_SIZE, stdin);
    if (strnlen(message, BUFF_SIZE) > 0)
      _write(*socketfd, message, BUFF_SIZE);
  }
  */
}

WINDOW *create_newwin(int y, int x, int starty, int startx, int border) {
  WINDOW *win;
  win = newwin(y, x, starty, startx);
  if (border != -1)
    box(win, border, border);
  wrefresh(win);
  return win;
}

void destroy_win(WINDOW *win) {
  wborder(win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
  wrefresh(win);
  delwin(win);
}

void initwin() {
  int startx, starty, x, y;
  initscr();
  noecho();
  raw();
  nonl();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);

  getmaxyx(stdscr, y, x);
  y -= 3;
  startx = 0;
  starty = 0;

  refresh();
  output_win = create_newwin(y, x, starty, startx, -1);
  starty = y;
  y = 3;
  input_win = create_newwin(y, x, starty, startx, 0);

  input_field[0] = new_field(1, x - 2, starty + 1, startx + 1, 0, 0);
  input_field[1] = NULL;
  set_field_back(input_field[0], A_UNDERLINE);
  field_opts_off(input_field[0], O_AUTOSKIP);

  form = new_form(input_field);
  post_form(form);
  refresh();

  scrollok(output_win, TRUE);
  wrefresh(output_win);
  wrefresh(input_win);
}
