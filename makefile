CC=cc
WARNS=-Wall -ansi -pedantic -std=c89
CFLAGS=-O0
LINUX=-I /usr/local/include -L /usr/local/lib -lSDL2
MACOS=-framework SDL2
CMD=$(CC) $(MACOS) $(CFLAGS) main.c $(WARNS)

main:
	$(CMD) -o game

run:
	make && ./game

debug:
	$(CMD) -g && lldb a.out

clean:
	rm game a.out && rm -rf a.out.dSYM
	