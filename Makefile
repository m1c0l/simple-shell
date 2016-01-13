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

main.c filedesc.c command.c: filedesc.h
main.c command.c: command.h

#main.o raymath.o: raymath.h

check: clean simpsh
	./test.sh

piazza: clean simpsh
	./piazza-tests.sh

checkall: check piazza

targz = lab1-michaelli
sources = Makefile main.c filedesc.c filedesc.h command.c command.h README

dist: $(targz)

$(targz): $(sources)
	tar cf - --transform='s|^|$(targz)/|' $(sources) | gzip -9 > $@.tar.gz

clean:
	rm -rf *.o *.tmp $(targz) $(targz).tar.gz
