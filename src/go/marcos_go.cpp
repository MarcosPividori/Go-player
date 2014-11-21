
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
          else if(c<argc-1 && (strncmp(argv[c],"--cycles_mcts",13)==0))
            cfg_input.number_cycles_mcts=std::stoi(argv[++c]);
          else if(c<argc-1 && (strncmp(argv[c],"--threads_mcts",14)==0))
            cfg_input.num_threads_mcts=std::stoi(argv[++c]);
          else if(c<argc-1 && (strncmp(argv[c],"--resign_limit",14)==0))
            cfg_input.resign_limit=std::stod(argv[++c]);
          else if(strncmp(argv[c],"--root_parallel",15)==0)
            cfg_input.root_parallel=true;
          else if(strncmp(argv[c],"--japanese_rules",16)==0)
            cfg_input.japanese_rules=true;
          else if(strncmp(argv[c],"--totally_random_sim",20)==0)
            cfg_input.knowledge=false;
          else if(strncmp(argv[c],"--no_rave",9)==0)
            cfg_input.rave=false;
          else if(strncmp(argv[c],"--help",6)==0){
            std::cout<<"usage: marcos_go [--patterns file] [--bandit_coeff double] [--amaf_coeff double] [--help]\n"
                     <<"                 [--fill_board double] [--long_game_coeff double] [--limit_expansion int]\n"
                     <<"                 [--cycles_mcts int] [--threads_mcts int] [--resign_limit double]\n"
                     <<"                 [--root_parallel] [--japanese_rules] [--totally_random_sim] [--no_rave]\n";
            return 0;
          }
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

