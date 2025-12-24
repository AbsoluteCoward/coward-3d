CC = gcc
WARNINGS = -Wall -Wextra -Wpedantic -Wshadow -Wstrict-overflow=5
RELEASE_CFLAGS = -std=c99 $(WARNINGS) -O3 -march=native -I. -DNDEBUG
DEBUG_CFLAGS   = -std=c99 $(WARNINGS) -g3 -O0 -I.

# Detect operating system
ifeq ($(OS),Windows_NT)
    LDFLAGS = ./lib/libraylib.a -lopengl32 -lgdi32 -lwinmm
    RELEASE_LDFLAGS = -s -static
    EXE_EXT = .exe
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LDFLAGS = ./lib/libraylib.a -lGL -lm -lpthread -ldl -lrt -lX11
        RELEASE_LDFLAGS = -s 
        EXE_EXT =
    endif
    ifeq ($(UNAME_S),Darwin)
        LDFLAGS = ./lib/libraylib.a -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
        RELEASE_LDFLAGS = -s
        EXE_EXT =
    endif
endif

SRC          = $(wildcard ./src/*.c)
RELEASE_OUT  = ./build/game$(EXE_EXT)
DEBUG_OUT    = ./build/game_debug$(EXE_EXT)

.PHONY: all release debug clean

all: release

release: $(RELEASE_OUT)

debug: $(DEBUG_OUT)

$(RELEASE_OUT): $(SRC)
	mkdir -p $(dir $@)
	$(CC) $(RELEASE_CFLAGS) $^ -o $@ $(LDFLAGS) $(RELEASE_LDFLAGS)

$(DEBUG_OUT): $(SRC)
	mkdir -p $(dir $@)
	$(CC) $(DEBUG_CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf build

