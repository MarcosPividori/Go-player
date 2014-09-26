
#include <stdio.h> 
#include "play_gtp.h"
#include <cstring>

int main(int argc,char *argv[])
{
    FILE *gtp_input_FILE = stdin;
    FILE *gtp_output_FILE = stdout;
    FILE *gtp_dump_commands_FILE = NULL;

    Game game(19);
    play_gtp(gtp_input_FILE, gtp_output_FILE, gtp_dump_commands_FILE, &game);

    return 0;
}

