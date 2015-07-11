# Project: SDLPoP
# Makefile created by Dev-C++ 4.9.9.2

CC = gcc
RM = rm -f

HFILES = common.h config.h data.h proto.h types.h
OBJ = main.o data.o seg000.o seg001.o seg002.o seg003.o seg004.o seg005.o seg006.o seg007.o seg008.o seg009.o roomscript.o
BIN = prince

LIBS := $(shell pkg-config --libs   sdl2 SDL2_image SDL2_mixer)
INCS := $(shell pkg-config --cflags sdl2 SDL2_image SDL2_mixer)

CFLAGS += $(INCS) -Wall -std=gnu99

all: $(BIN)

clean:
	$(RM) $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ) -o $@ $(LIBS)

%.o: %.c $(HFILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $<

.PHONY: all clean
