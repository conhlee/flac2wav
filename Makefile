CC = gcc

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Darwin)
    CFLAGS = -I/opt/homebrew/Cellar/flac/1.4.3/include
    LDFLAGS = -L/opt/homebrew/Cellar/flac/1.4.3/lib -lflac
else ifeq ($(UNAME_S), Linux)
    CFLAGS = -I/usr/include
    LDFLAGS = -L/usr/lib -lflac
else ifeq ($(UNAME_S), Windows_NT)
    CFLAGS = -IC:/msys/usr/include
    LDFLAGS = -LC:/msys/usr/lib -lflac
else
    $(error ??? what kind of system is this??)
endif

TARGET = flac2wav
SOURCES = main.c files.c list.c common.c
OBJECTS = $(SOURCES:.c=.o)
HEADERS = files.h common.h flacProcess.h wavProcess.h

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)