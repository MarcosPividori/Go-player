
#include <stdio.h> 
#include "play_gtp.h"
#include <cstring>

int main(int argc,char *argv[])
{
    FILE *gtp_input_FILE = stdin;
    FILE *gtp_output_FILE = stdout;
    FILE *gtp_dump_commands_FILE = NULL;
   
    Config cfg_input; 
    for(int c=1;c<argc;c++){
      try
      {
          if(c<argc-1 && (strncmp(argv[c],"--patterns",10)==0))
            cfg_input.pattern_file=argv[++c];
          else if(c<argc-1 && (strncmp(argv[c],"--bandit_coeff",14)==0))
            cfg_input.bandit_coeff=std::stod(argv[++c]);
          else if(c<argc-1 && (strncmp(argv[c],"--amaf_coeff",12)==0))
            cfg_input.amaf_coeff=std::stod(argv[++c]);
          else if(c<argc-1 && (strncmp(argv[c],"--fill_board",12)==0))
            cfg_input.number_fill_board_attemps=std::stod(argv[++c]);
          else if(c<argc-1 && (strncmp(argv[c],"--long_game_coeff",17)==0))
            cfg_input.long_game_coeff=std::stod(argv[++c]);
          else if(c<argc-1 && (strncmp(argv[c],"--limit_expansion",17)==0))
            cfg_input.limit_expansion=std::stoi(argv[++c]);
          else{
            std::cerr<<"Unknown option: "<<argv[c]<<std::endl;
            return -1;
          }
      }
      catch(std::invalid_argument){
        std::cerr<<"Not proper value on "<<argv[c-1]<<" argument."<<std::endl;
        return -1;
      }
    }

    Game game(19,cfg_input);

    play_gtp(gtp_input_FILE, gtp_output_FILE, gtp_dump_commands_FILE, &game);

    return 0;
}

