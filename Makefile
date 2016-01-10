CC = gcc
CFLAGS = -std=gnu99 -g -Wall -Wextra -Wno-unused-parameter # -O2

all: simpsh

objects = main.o filedesc.o command.o

simpsh: $(objects)
	$(CC) $(CFLAGS) -o $@ $(objects)

c: command.o
	$(CC) $(CFLAGS) -o command $(objects)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

main.c filedesc.c: filedesc.h

main.c command.c: command.h

#main.o raymath.o: raymath.h

check: simpsh
	./test.sh

dist:

clean:
	rm -f *.o *.tmp simpsh simsh.tar.gz
