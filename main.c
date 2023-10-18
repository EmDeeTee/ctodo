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
#define KEY_Q 113
#define KEY_SPACE 32

#define TODO_CAP 32

// NOTE: All db-reading functions assume the db file is in the woroking directory
#define DB_FILENAME "todo.db"

size_t todo_selected = 0;

typedef struct TodoItem {
  char* content;
  bool isCompleted;
}TodoItem;

TodoItem todos[TODO_CAP] = {0};
size_t todo_count = 0;

void notify(const char* msg) {
  printw("%s\n", msg);
  getch();
}

void print_todos(void) {
  for (int i=0; i < todo_count; i++) {
    if (i == todo_selected) {
      attron(A_STANDOUT);
    }
    if (todos[i].isCompleted) {
      printw("[x] ");
    } else {
      printw("[] ");
    }
    printw("%s\n", todos[i].content);
    attroff(A_STANDOUT);
  }
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
      if (todo_selected > 0)
        todo_selected--;
      break;
    case KEY_S:
      if (todo_selected < (todo_count - 1))
        todo_selected++;
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
    }
    clear();
    refresh();
    print_todos();
  }
}
void init_todo_app(void) {
  initscr();
  noecho();
  signal(SIGINT, exit_app);
  read_state_from_db();
  print_todos();
  refresh();
}

int main(void) {
  init_todo_app();
  app_loop();
  
  endwin();
  return 0;
}
