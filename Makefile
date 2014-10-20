
CC = g++
CFLAGS = -std=c++0x -pthread -DJAPANESE -DRAVE -O2 -DKNOWLEDGE -O2
DEBUGFLAGS = -DDEBUG -g

BINDIR = bin
OBJDIR = obj
SRCDIR = src

SOURCESC = play_gtp.c gtp.c
SOURCESCPP = state_go.cpp game.cpp pattern_list.cpp
OBJS = $(SOURCESCPP:%.cpp=$(OBJDIR)/%.o) $(SOURCESC:%.c=$(OBJDIR)/%.o)
OBJS_DEBUG = $(SOURCESCPP:%.cpp=$(OBJDIR)/%_debug.o) $(SOURCESC:%.c=$(OBJDIR)/%_debug.o)

all: marcos_go tateti connect4 2048 hexa

marcos_go: $(OBJS)
		$(CC) $(CFLAGS) $(SRCDIR)/marcos_go.cpp -o $(BINDIR)/marcos_go $(OBJS)

marcos_debug: $(OBJS_DEBUG)
		$(CC) $(CFLAGS) $(DEBUGFLAGS) $(SRCDIR)/marcos_go.cpp -o $(BINDIR)/marcos_debug $(OBJS_DEBUG)

$(OBJDIR)/%_debug.o: $(SRCDIR)/%.cpp
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/%_debug.o: $(SRCDIR)/%.c
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		$(RM) $(BINDIR)/* $(OBJDIR)/*

tateti:
		$(CC) $(CFLAGS) $(SRCDIR)/marcos_tateti.cpp -o $(BINDIR)/marcos_tateti

connect4:
		$(CC) $(CFLAGS) $(SRCDIR)/marcos_connect4.cpp -o $(BINDIR)/marcos_connect4

2048:
		$(CC) $(CFLAGS) $(SRCDIR)/marcos_2048.cpp -o $(BINDIR)/marcos_2048

hexa:
		$(CC) $(CFLAGS) $(SRCDIR)/marcos_hexa.cpp -o $(BINDIR)/marcos_hexa

