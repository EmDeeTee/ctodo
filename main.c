#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>

#define KEY_W 119
#define KEY_S 115
#define KEY_A 97
#define KEY_D 100
#define KEY_Q 113
#define KEY_SPACE 32

#define TODO_CAP 256

// NOTE: All db-reading functions assume the db file is in the woroking directory
#define DB_FILENAME "todo.db"

typedef struct TodoItem {
  char* content;
  bool isCompleted;
}TodoItem;

WINDOW* pad;
size_t todo_selected = 0;
int pad_start = 0;
TodoItem todos[TODO_CAP] = {0};
size_t todo_count = 0;

void notify(const char* msg) {
  printw("%s\n", msg);
  getch();
}

void print_todos(void) {
  int max_y, max_x = 0;
  max_x = 0 - max_x; // Unused
  const int msg_line = LINES - 1;
  getmaxyx(pad, max_y, max_x);
  int visible_rows = max_y - pad_start - 1;
  for (int i = 0; i < todo_count; i++) {
    if (i == todo_selected) {
      wattron(pad, A_STANDOUT);
    }
    if (todos[i].isCompleted) {
      wprintw(pad, "[x] ");
    } else {
      wprintw(pad, "[] ");
    }
    wprintw(pad, "%s\n", todos[i].content);
    wattroff(pad, A_STANDOUT);
  }
  if (visible_rows > LINES - 1)
    mvprintw(msg_line, 0, "SCROLL FOR MORE");
  else
    mvprintw(msg_line, 0, "               ");
  prefresh(pad, pad_start, 0, 0, 0, msg_line - 1, COLS);
}

void add_todo(void) {
  if ((todo_count + 1) > TODO_CAP) {
    notify("TODO_CAP exhausted. No more todos can be added");
    return;
  }

  char str[80];
  printw("New todo: ");
  echo();
  getstr(str);
  todos[todo_count++] = (TodoItem){.content=strdup(str), .isCompleted=false};
  noecho();
}

void remove_todo(size_t at) {
  if (at < todo_count) {
    free(todos[at].content);
    for (int i = at; i < todo_count - 1; i++) {
      todos[i] = todos[i + 1];
    }
    todo_count--;
  }
}

void save_state_to_db(void) {
  FILE* fdb = fopen(DB_FILENAME, "w");
  for (int i = 0; i < todo_count; i++) {
    fprintf(fdb, "%s:%d\n", todos[i].content, todos[i].isCompleted);
  }
  fflush(fdb);
  fclose(fdb);
}

void read_state_from_db(void) {
  if (access(DB_FILENAME, F_OK) != 0) {
    FILE* _f = fopen(DB_FILENAME, "w");
    fclose(_f);
  }

  FILE* fdb = fopen(DB_FILENAME, "r");
  char str[128];
  int compl = 0;

  while (fscanf(fdb, "%127[^:]:%d\n", str, &compl) == 2) {
    todos[todo_count++] = (TodoItem){.content=strdup(str), .isCompleted=compl};
  }
}

void exit_app(int _non) {
  save_state_to_db();
  endwin();
  exit(0);
}
void app_loop(void) {
  while (1==1) {
    int c = getch();
    switch (c) {
    case KEY_W:
      if (todo_selected > 0) {
        todo_selected--;
	pad_start--;
      }
      break;
    case KEY_S:
      if (todo_selected < (todo_count - 1)) {
        todo_selected++;
	pad_start++;
      }
      break;
    case KEY_SPACE:
      todos[todo_selected].isCompleted = !todos[todo_selected].isCompleted;
      break;
    case KEY_A:
      add_todo();
      break;
    case KEY_Q:
      exit_app(0);
      break;
    case KEY_D:
      remove_todo(todo_selected);
      break;
    }
    wclear(pad);
    wrefresh(pad);
    print_todos();
  }
}
void init_todo_app(void) {
  initscr();
  noecho();
  signal(SIGINT, exit_app);
  read_state_from_db();
  pad = newpad(todo_count + 1, COLS);
  print_todos();
  refresh();
  wrefresh(pad);
}

int main(void) {
  init_todo_app();
  app_loop();
  endwin();
  return 0;
}
