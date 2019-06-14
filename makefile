PROG := gamma

ifeq ($(OS),Windows_NT)
S := \\
else
S := /
endif

SRCDIR := src
OBJDIR := obj
BINDIR := bin

SOURCES =	main.c \
			beta.c \
			term.c \
			cga.c \
			betalib$(S)betalib.c \
			betalib$(S)debug.c \
			
INCLUDES = -Isrc$(S)inc -I/usr/local/include -I/usr/include/lua5.1
OBJECTS = $(patsubst %,$(OBJDIR)$(S)%,$(SOURCES:.c=.o))

CFLAGS := -Wall -pedantic -std=gnu99 -g -O0
LFLAGS = `sdl-config --libs` -lSDL -L/usr/local/lib -llua5.1 -lSDL_image
CC := gcc

all: $(PROG)

run: $(PROG)
	$(BINDIR)$(S)$(PROG)
	
debug: $(PROG)
	gdb $(BINDIR)$(S)$(PROG)

# linking the program.
$(PROG): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $(BINDIR)$(S)$(PROG) $(LFLAGS)

# compiling source files.
$(OBJDIR)$(S)%.o: $(SRCDIR)$(S)%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
ifeq ($(OS),Windows_NT)
	del $(OBJECTS)
else
	rm $(OBJECTS)
endif

.PHONY: all clean

