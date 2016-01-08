CC = gcc
CFLAGS = -std=gnu99 -g -O2 -Wall -Wextra -Wno-unused-parameter

all: simpsh

objects = main.o
simpsh: $(objects)
	$(CC) $(CFLAGS) -o $@ obj/$(objects)

%.o: %.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c $< -o obj/$@

#main.o raymath.o: raymath.h

clean:
	rm -f */*.o *.tmp simsh.tar.gz
