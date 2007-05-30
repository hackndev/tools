all: hexterm

hexterm: hexterm.c
	$(CC) -g -std=c99 -o hexterm hexterm.c -lreadline

clean:
	rm -f hexterm *.o
