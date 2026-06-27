CC = gcc
CFLAGS = -O3 -Wall -Wextra
LDFLAGS = -lpsapi

SRCS = main.c preprocess.c storage.c search.c compression.c utils.c md5.c miniz.c
OBJS = $(SRCS:.c=.o)
TARGET = ../Project.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)