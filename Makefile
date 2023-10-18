all:
	gcc -Wall -std=c99 `pkg-config --cflags ncurses` `pkg-config --libs ncurses` main.c -o ctodo
