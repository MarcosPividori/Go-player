
#include <stdio.h> 
#include "play_gtp.h"
#include <cstring>

using namespace std;

int main(int argc,char *argv[])
{
    FILE *gtp_input_FILE = stdin;
    FILE *gtp_output_FILE = stdout;
    FILE *gtp_dump_commands_FILE = NULL;
   
    Config cfg_input; 
    for(int c=1;c<argc;c++){
      try
      {
          if(c<argc-1 && !strcmp(argv[c],"--patterns"))
            cfg_input.pattern_file=argv[++c];
          else if(c<argc-1 && !strcmp(argv[c],"--bandit_coeff"))
            cfg_input.bandit_coeff=stod(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--amaf_coeff"))
            cfg_input.amaf_coeff=stod(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--fill_board"))
            cfg_input.number_fill_board_attemps=stod(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--long_game_coeff"))
            cfg_input.long_game_coeff=stod(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--pattern_coeff"))
            cfg_input.pattern_coeff=stod(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--capture_coeff"))
            cfg_input.capture_coeff=stod(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--atari_delete_coeff"))
            cfg_input.atari_delete_coeff=stod(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--atari_escape_coeff"))
            cfg_input.atari_escape_coeff=stod(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--limit_atari"))
            cfg_input.limit_atari=stoi(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--limit_expansion"))
            cfg_input.limit_expansion=stoi(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--cycles_mcts"))
            cfg_input.number_cycles_mcts=stoi(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--threads_mcts"))
            cfg_input.num_threads_mcts=stoi(argv[++c]);
          else if(c<argc-1 && !strcmp(argv[c],"--resign_limit"))
            cfg_input.resign_limit=stod(argv[++c]);
          else if(!strcmp(argv[c],"--root_parallel"))
            cfg_input.root_parallel=true;
          else if(!strcmp(argv[c],"--japanese_rules"))
            cfg_input.japanese_rules=true;
          else if(!strcmp(argv[c],"--totally_random_sim"))
            cfg_input.knowledge=false;
          else if(!strcmp(argv[c],"--no_rave"))
            cfg_input.rave=false;
          else if(!strcmp(argv[c],"--help")){
            cout<<"usage: marcos_go [--patterns file] [--bandit_coeff double] [--amaf_coeff double] [--help]\n"
                     <<"                 [--fill_board double] [--long_game_coeff double] [--limit_expansion int]\n"
                     <<"                 [--cycles_mcts int] [--threads_mcts int] [--resign_limit double]\n"
                     <<"                 [--root_parallel] [--japanese_rules] [--totally_random_sim] [--no_rave]\n";
            return 0;
          }
          else{
            cerr<<"Unknown option: "<<argv[c]<<endl;
            return -1;
          }
      }
      catch(invalid_argument){
        cerr<<"Not proper value on "<<argv[c-1]<<" argument."<<endl;
        return -1;
      }
    }

    Game game(9,cfg_input);

    play_gtp(gtp_input_FILE, gtp_output_FILE, gtp_dump_commands_FILE, &game);

    return 0;
}

