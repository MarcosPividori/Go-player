
BINDIR = bin
OBJDIR = obj
SRCDIR = src
SRCDIR_MCTS = $(SRCDIR)/mcts
SRCDIR_GO = $(SRCDIR)/go
SRCDIR_TATETI = $(SRCDIR)/tateti
SRCDIR_CONNECT4 = $(SRCDIR)/connect4
SRCDIR_HEXA = $(SRCDIR)/hexa

CC = g++
CFLAGS = -std=c++0x -pthread -O2 -I $(SRCDIR_MCTS) 
DEBUGFLAGS = -DDEBUG -g
CFLAGS_HEXA = -DRAVE 

SOURCESC_GO = play_gtp.c gtp.c
SOURCESCPP_GO = state_go.cpp game.cpp pattern_list.cpp mcts_go.cpp
OBJS_GO = $(SOURCESCPP_GO:%.cpp=$(OBJDIR)/%.o) $(SOURCESC_GO:%.c=$(OBJDIR)/%.o)
OBJS_DEBUG_GO = $(SOURCESCPP_GO:%.cpp=$(OBJDIR)/%_debug.o) $(SOURCESC_GO:%.c=$(OBJDIR)/%_debug.o)

$(OBJDIR):
	mkdir $(OBJDIR)

$(BINDIR):
	mkdir $(BINDIR)

$(OBJDIR)/%_debug.o: $(SRCDIR_GO)/%.cpp $(OBJDIR)
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/%_debug.o: $(SRCDIR_GO)/%.c $(OBJDIR)
		$(CC) $(CFLAGS) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR_GO)/%.cpp $(OBJDIR)
		$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR_GO)/%.c $(OBJDIR)
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		$(RM) -r $(BINDIR) $(OBJDIR)

all: go debug tateti connect4 hexa

go: $(OBJS_GO) $(BINDIR)
		$(CC) $(CFLAGS) $(SRCDIR_GO)/marcos_go.cpp -o $(BINDIR)/marcos_go $(OBJS_GO)

debug: $(OBJS_DEBUG_GO) $(BINDIR)
		$(CC) $(CFLAGS) $(DEBUGFLAGS) $(SRCDIR_GO)/marcos_go.cpp -o $(BINDIR)/marcos_debug $(OBJS_DEBUG_GO)

tateti: $(BINDIR)
		$(CC) $(CFLAGS) $(SRCDIR_TATETI)/marcos_tateti.cpp $(SRCDIR_TATETI)/state_tateti.cpp -o $(BINDIR)/marcos_tateti

connect4: $(BINDIR)
		$(CC) $(CFLAGS) $(SRCDIR_CONNECT4)/marcos_connect4.cpp $(SRCDIR_CONNECT4)/state_connect4.cpp -o $(BINDIR)/marcos_connect4

hexa: $(BINDIR)
		$(CC) $(CFLAGS) $(CFLAGS_HEXA) $(SRCDIR_HEXA)/marcos_hexa.cpp $(SRCDIR_HEXA)/state_hexa.cpp -o $(BINDIR)/marcos_hexa

