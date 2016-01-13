CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-unused-parameter
OPTIMIZE = -g # -O2

all: simpsh

SOURCES = main.c filedesc.c command.c
OBJECTS = $(subst .c,.o,$(SOURCES))

simpsh: $(OBJECTS)
	$(CC) $(CFLAGS) $(OPTIMIZE) -o $@ $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


main.c filedesc.c command.c: filedesc.h
main.c command.c: command.h


check: test piazza

test: clean simpsh
	./test.sh

piazza: clean simpsh
	./piazza-tests.sh


DISTDIR = lab1-michaelli
DIST_FILES = Makefile $(SOURCES) filedesc.h command.h README $(TESTS)

dist: $(DISTDIR)

$(DISTDIR): $(DIST_FILES)
	tar cf - --transform='s|^|$(DISTDIR)/|' $(DIST_FILES) | gzip -9 > $@.tar.gz


clean:
	rm -rf *.o *.tmp $(DISTDIR) $(DISTDIR).tar.gz

.PHONY: all check dist clean
