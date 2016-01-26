CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-unused-parameter
OPTIMIZE = -O2

all: simpsh

SRC_DIR = src
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

simpsh: $(OBJ_DIR) $(OBJECTS)
	$(CC) $(CFLAGS) $(OPTIMIZE) -o $@ $(OBJECTS)

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $(OPTIMIZE) -c $< -o $@


TESTS = test.sh #piazza-tests.sh
check: simpsh
	for test in $(TESTS); do \
		./$$test || exit; \
	done


DISTDIR = lab1-michaelli
DIST_FILES = Makefile README $(SRC_DIR) $(TESTS)

dist: $(DISTDIR)

$(DISTDIR): $(DIST_FILES)
	tar cf - --transform='s|^|$(DISTDIR)/|' $(DIST_FILES) | gzip -9 > $@.tar.gz


clean:
	rm -rf simpsh obj/ *.o *.tmp $(DISTDIR) $(DISTDIR).tar.gz

.PHONY: all check dist clean test
