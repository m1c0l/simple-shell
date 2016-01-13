CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-unused-parameter
OPTIMIZE = -g # -O2

all: simpsh

SOURCES = main.c filedesc.c command.c util.c
HEADERS = filedesc.h command.h util.h
OBJECTS = $(subst .c,.o,$(SOURCES))
TESTS = test.sh

simpsh: $(OBJECTS)
	$(CC) $(CFLAGS) $(OPTIMIZE) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


main.c filedesc.c command.c: filedesc.h
main.c command.c: command.h
main.c command.c util.c: util.h

check: test piazza

test: clean
	./test.sh


DISTDIR = lab1-michaelli
DIST_FILES = Makefile README $(SOURCES) $(HEADERS) $(TESTS)

dist: $(DISTDIR)

$(DISTDIR): $(DIST_FILES)
	tar cf - --transform='s|^|$(DISTDIR)/|' $(DIST_FILES) | gzip -9 > $@.tar.gz


clean:
	rm -rf *.o *.tmp $(DISTDIR) $(DISTDIR).tar.gz

.PHONY: all check dist clean
