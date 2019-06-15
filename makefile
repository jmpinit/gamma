PROG := gamma

SRCDIR := src
OBJDIR := obj
BINDIR := bin

SOURCES =	main.c \
			beta.c \
			term.c \
			cga.c \
			betalib.c \
			debug.c

LUA_CFLAGS=`pkg-config lua5.1 --cflags`
LDFLAGS=`pkg-config lua5.1 --libs` `sdl2-config --libs` -lSDL2_image

CC=clang
CCOPT= -fomit-frame-pointer
CCWARN= -Wall

INCLUDES = -Isrc/inc -I/usr/local/include
OBJECTS = $(patsubst %,$(OBJDIR)/%,$(SOURCES:.c=.o))

CFLAGS= -g $(CCOPT) $(CCWARN) $(INCLUDES) $(LUA_CFLAGS)

all: $(PROG)

run: $(PROG)
	$(BINDIR)/$(PROG) src/scripts/run.lua

test: $(PROG)
	$(BINDIR)/$(PROG) src/scripts/checker.lua

debug: $(PROG)
	gdb $(BINDIR)/$(PROG) src/scripts/run.lua

# linking the program.
$(PROG): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $(BINDIR)/$(PROG) $(LDFLAGS)

# compiling source files.
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -r $(OBJDIR) $(BINDIR)

.PHONY: all clean

