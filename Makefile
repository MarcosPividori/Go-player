
BINDIR = bin
OBJDIR = obj
SRCDIR_MCTS = src/mcts
SRCDIR_GO = src/go
SRCDIR_TATETI = src/tateti
SRCDIR_CONNECT4 = src/connect4
SRCDIR_HEXA = src/hexa
SRCDIR_2048 = src/2048

CC = g++
CFLAGS = -std=c++0x -pthread -O2 -I $(SRCDIR_MCTS) 
CFLAGS_GO = -DJAPANESE -DRAVE -DKNOWLEDGE
DEBUGFLAGS = -DDEBUG -g

SOURCESC_GO = play_gtp.c gtp.c
SOURCESCPP_GO = state_go.cpp game.cpp pattern_list.cpp
OBJS_GO = $(SOURCESCPP_GO:%.cpp=$(OBJDIR)/%.o) $(SOURCESC_GO:%.c=$(OBJDIR)/%.o)
OBJS_DEBUG_GO = $(SOURCESCPP_GO:%.cpp=$(OBJDIR)/%_debug.o) $(SOURCESC_GO:%.c=$(OBJDIR)/%_debug.o)

all: marcos_go marcos_debug tateti connect4 2048 hexa

marcos_go: $(OBJS_GO)
		$(CC) $(CFLAGS) $(CFLAGS_GO) $(SRCDIR_GO)/marcos_go.cpp -o $(BINDIR)/marcos_go $(OBJS_GO)

marcos_debug: $(OBJS_DEBUG_GO)
		$(CC) $(CFLAGS) $(CFLAGS_GO) $(DEBUGFLAGS) $(SRCDIR_GO)/marcos_go.cpp -o $(BINDIR)/marcos_debug $(OBJS_DEBUG_GO)

$(OBJDIR)/%_debug.o: $(SRCDIR_GO)/%.cpp
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/%_debug.o: $(SRCDIR_GO)/%.c
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR_GO)/%.cpp
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR_GO)/%.c
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		$(RM) $(BINDIR)/* $(OBJDIR)/*

tateti:
		$(CC) $(CFLAGS) $(SRCDIR_TATETI)/marcos_tateti.cpp $(SRCDIR_TATETI)/state_tateti.cpp -o $(BINDIR)/marcos_tateti

connect4:
		$(CC) $(CFLAGS) $(SRCDIR_CONNECT4)/marcos_connect4.cpp -o $(BINDIR)/marcos_connect4

2048:
		$(CC) $(CFLAGS) $(SRCDIR_2048)/marcos_2048.cpp -o $(BINDIR)/marcos_2048

hexa: 
		$(CC) $(CFLAGS) $(CFLAGS_HEXA) $(SRCDIR_HEXA)/marcos_hexa.cpp $(SRCDIR_HEXA)/state_hexa.cpp -o $(BINDIR)/marcos_hexa
