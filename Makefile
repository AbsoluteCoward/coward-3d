CC = gcc
WARNINGS = -Wall -Wextra -Wpedantic -Wshadow -Wstrict-overflow=5
RELEASE_CFLAGS = -std=c99 $(WARNINGS) -O3 -march=native -I. -DNDEBUG
DEBUG_CFLAGS = -std=c99 $(WARNINGS) -g3 -O0 -I.

# Detect operating system
ifeq ($(OS),Windows_NT)
    LDFLAGS = ./lib/libraylib.a -lopengl32 -lgdi32 -lwinmm
    RELEASE_LDFLAGS = -s -static
    EXE_EXT = .exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LDFLAGS = ./lib/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
        RELEASE_LDFLAGS = -s -static
        EXE_EXT =
    endif
    ifeq ($(UNAME_S),Darwin)
        LDFLAGS = ./lib/libraylib.a -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
        RELEASE_LDFLAGS = -s
        EXE_EXT =
    endif
endif

SRC = ./src/main.c
RELEASE_OUT = ./build/game$(EXE_EXT)
DEBUG_OUT = ./build/game_debug$(EXE_EXT)

.PHONY: all debug release clean

all: release

release:
	$(CC) $(RELEASE_CFLAGS) $(SRC) -o $(RELEASE_OUT) $(LDFLAGS) $(RELEASE_LDFLAGS)

debug:
	$(CC) $(DEBUG_CFLAGS) $(SRC) -o $(DEBUG_OUT) $(LDFLAGS)

clean:
	rm -f $(RELEASE_OUT) $(DEBUG_OUT)
