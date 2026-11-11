CC = gcc
TARGET = seaquest
SRC = SeaQuest.c

CFLAGS = $(shell pkg-config --cflags allegro-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 allegro_image-5)
LIBS = $(shell pkg-config --libs allegro-5 allegro_font-5 allegro_ttf-5 allegro_primitives-5 allegro_image-5) -lm

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)