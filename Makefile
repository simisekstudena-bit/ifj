CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -MMD -MP
TARGET = ifj26
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

.PHONY: build clean check build-check


build: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# TODO
# check:
# build-check:

-include $(DEPS)

clean:
	rm -f $(TARGET) $(OBJS) $(DEPS)
