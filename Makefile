CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -Wno-unused-parameter
OPTIMIZE = -O2

SRC_DIR = src
OBJ_DIR = obj
SOURCES = $(wildcard $(SRC_DIR)/*.c)
HEADERS = $(wildcard $(SRC_DIR)/*.h)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BENCHMARK_DIR = benchmarks
TOY_PROG = $(BENCHMARK_DIR)/program

all: simpsh $(TOY_PROG)

simpsh: $(OBJ_DIR) $(OBJECTS)
	$(CC) $(CFLAGS) $(OPTIMIZE) -o $@ $(OBJECTS)

$(OBJ_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $(OPTIMIZE) -c $< -o $@

$(TOY_PROG): $(TOY_PROG).c
	gcc $(TOY_PROG).c -o $(TOY_PROG)

TESTS = test.sh #piazza-tests.sh
check: simpsh
	for test in $(TESTS); do \
		./$$test || exit; \
	done


DISTDIR = lab1-michaelli
DIST_FILES = Makefile README $(SRC_DIR) $(TESTS) $(BENCHMARK_DIR)

dist: $(DISTDIR)

$(DISTDIR): clean $(DIST_FILES)
	tar cf - --transform='s|^|$(DISTDIR)/|' $(DIST_FILES) | gzip -9 > $@.tar.gz


clean:
	rm -rf simpsh obj/ *.o *.tmp $(DISTDIR) $(DISTDIR).tar.gz $(TOY_PROG)

.PHONY: all check dist clean test
