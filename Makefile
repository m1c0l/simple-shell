CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-unused-parameter
OPTIMIZE = -g# -O2

all: simpsh

SOURCES = main.c filedesc.c command.c util.c stream.c
_HEADERS = filedesc.h command.h stream.h util.h
HEADERS = $(HEADERS:%=src/%)
OBJECTS = $(SOURCES:%.c=obj/%.o)

simpsh: obj $(OBJECTS)
	$(CC) $(CFLAGS) $(OPTIMIZE) -o $@ $(OBJECTS)

obj:
	mkdir -p $@

obj/%.o: src/%.c
	$(CC) $(CFLAGS) $(OPTIMIZE) -c $< -o $@

$(SOURCES): $(_HEADERS)


TESTS = test.sh piazza-tests.sh
check: clean simpsh
	for test in $(TESTS); do \
		./$$test || exit; \
	done


DISTDIR = lab1-michaelli
DIST_FILES = Makefile README src/ $(TESTS)

dist: $(DISTDIR)

$(DISTDIR): $(DIST_FILES)
	tar cf - --transform='s|^|$(DISTDIR)/|' $(DIST_FILES) | gzip -9 > $@.tar.gz


clean:
	rm -rf simpsh obj/ *.o *.tmp $(DISTDIR) $(DISTDIR).tar.gz

.PHONY: all check dist clean test
