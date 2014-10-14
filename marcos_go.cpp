
#include <stdio.h> 
#include "play_gtp.h"
#include <cstring>

int main(int argc,char *argv[])
{
    FILE *gtp_input_FILE = stdin;
    FILE *gtp_output_FILE = stdout;
    FILE *gtp_dump_commands_FILE = NULL;
    
    Game *game;
    if(argc>2 && (strncmp(argv[1],"--patterns",10)==0))
        game= new Game(19,argv[2]);
    else
        game= new Game(19);

    play_gtp(gtp_input_FILE, gtp_output_FILE, gtp_dump_commands_FILE, game);

    delete game;
    return 0;
}

