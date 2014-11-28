
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
CFLAGS_GO =
DEBUGFLAGS = -DDEBUG -g

SOURCESC_GO = play_gtp.c gtp.c
SOURCESCPP_GO = state_go.cpp game.cpp pattern_list.cpp opening_book.cpp mcts_go.cpp
OBJS_GO = $(SOURCESCPP_GO:%.cpp=$(OBJDIR)/%.o) $(SOURCESC_GO:%.c=$(OBJDIR)/%.o)
OBJS_DEBUG_GO = $(SOURCESCPP_GO:%.cpp=$(OBJDIR)/%_debug.o) $(SOURCESC_GO:%.c=$(OBJDIR)/%_debug.o)

$(OBJDIR)/%_debug.o: $(SRCDIR_GO)/%.cpp
		$(CC) $(CFLAGS) $(CFLAGS_GO) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/%_debug.o: $(SRCDIR_GO)/%.c
		$(CC) $(CFLAGS) $(CFLAGS_GO) $(DEBUGFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR_GO)/%.cpp
		$(CC) $(CFLAGS) $(CFLAGS_GO) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR_GO)/%.c
		$(CC) $(CFLAGS) $(CFLAGS_GO) -c $< -o $@

clean:
		$(RM) $(BINDIR)/* $(OBJDIR)/*.o

all: go debug tateti connect4 hexa

go: $(OBJS_GO)
		$(CC) $(CFLAGS) $(CFLAGS_GO) $(SRCDIR_GO)/marcos_go.cpp -o $(BINDIR)/marcos_go $(OBJS_GO)

debug: $(OBJS_DEBUG_GO)
		$(CC) $(CFLAGS) $(CFLAGS_GO) $(DEBUGFLAGS) $(SRCDIR_GO)/marcos_go.cpp -o $(BINDIR)/marcos_debug $(OBJS_DEBUG_GO)

tateti:
		$(CC) $(CFLAGS) $(SRCDIR_TATETI)/marcos_tateti.cpp $(SRCDIR_TATETI)/state_tateti.cpp -o $(BINDIR)/marcos_tateti

connect4:
		$(CC) $(CFLAGS) $(SRCDIR_CONNECT4)/marcos_connect4.cpp $(SRCDIR_CONNECT4)/state_connect4.cpp -o $(BINDIR)/marcos_connect4

hexa: 
		$(CC) $(CFLAGS) $(CFLAGS_HEXA) $(SRCDIR_HEXA)/marcos_hexa.cpp $(SRCDIR_HEXA)/state_hexa.cpp -o $(BINDIR)/marcos_hexa

env:
	wget -O gogui-1.4.9.zip http://sourceforge.net/projects/gogui/files/gogui/1.4.9/gogui-1.4.9.zip/download
	unzip gogui-1.4.9.zip
	wget -O gnugo-3.8.tar.gz http://ftp.gnu.org/gnu/gnugo/gnugo-3.8.tar.gz
	tar -zxvf gnugo-3.8.tar.gz
	cd gnugo-3.8;\
	./configure;\
	make

install:
	cd gnugo-3.8;\
	make install
	cd gogui-1.4.9; \
	./install.sh
