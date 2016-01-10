CC = gcc
CFLAGS = -std=gnu99 -g -Wall -Wextra -Wno-unused-parameter # -O2

all: simpsh

objects = main.o
obj_dir = obj

simpsh: $(objects)
	$(CC) $(CFLAGS) -o $@ obj/$(objects)

%.o: %.c $(obj_dir)
	$(CC) $(CFLAGS) -c $< -o obj/$@

$(obj_dir):
	mkdir -p $@

#main.o raymath.o: raymath.h

check: simpsh
	./test.sh

dist:

clean:
	rm -f */*.o *.tmp simsh.tar.gz
