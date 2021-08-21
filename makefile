WARNS=-Wall -ansi -pedantic -std=c89
LINUX=-I /usr/local/include -L /usr/local/lib -lSDL2
MACOS=-framework SDL2

macos:
	gcc main.c -O3 $(WARNS) $(MACOS) -o game && ./game

linux:
	gcc main.c -O3 $(WARNS) -lm -lSDL2 -o game && ./game

