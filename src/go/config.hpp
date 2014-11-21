
struct Config
{
    const char *pattern_file; 
    double bandit_coeff;
    double amaf_coeff;
    double number_fill_board_attemps;
    double long_game_coeff;
    int limit_expansion;
    int number_cycles_mcts;
    int num_threads_mcts;
    double resign_limit;
    bool root_parallel;
    bool japanese_rules;
    bool rave;
    bool knowledge;
    Config() : pattern_file(NULL),
               bandit_coeff(0),
               amaf_coeff(1000),
               number_fill_board_attemps(6),
               long_game_coeff(3),
               limit_expansion(2),
               number_cycles_mcts(75000),
               num_threads_mcts(5),
               resign_limit(0.1),
               root_parallel(false),
               japanese_rules(false),
               rave(true),
               knowledge(true)
    {}
};
