CC = gcc
CFLAGS = -ggdb -I/opt/homebrew/Cellar/flac/1.4.3/include
LDFLAGS = -L/opt/homebrew/Cellar/flac/1.4.3/lib -lflac
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
