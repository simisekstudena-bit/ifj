# Projekt: Implementace prekladace
# Author: Patrik Kadleček - xkadlep01
#         Jakub Ferencik - xferenj00
 
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -MMD -MP
TARGET = ifj26
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
IN = /dev/null

.PHONY: build clean check build-check debug valgrind


build: $(TARGET)

all: build

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

debug: clean
	$(MAKE) CFLAGS="$(CFLAGS) -g -fsanitize=address,undefined"

valgrind: clean
	$(MAKE) CFLAGS="$(CFLAGS) -g"
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET) < $(IN)

# TODO
# check:
# build-check:

-include $(DEPS)

clean:
	rm -f $(TARGET) src/*.o src/*.d