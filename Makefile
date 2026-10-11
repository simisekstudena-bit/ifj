CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = ifj26
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)

.PHONY: build clean check build-check


build: $(TARGET)
	
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# TODO
# check:
# build-check:


clean:
	rm -f $(TARGET) $(OBJS)