
struct Config
{
    const char *pattern_file; 
    double bandit_coeff;
    double amaf_coeff;
    double number_fill_board_attemps;
    double long_game_coeff;
    int limit_expansion;
    Config() : pattern_file(NULL),
               bandit_coeff(0.5),
               amaf_coeff(1000),
               number_fill_board_attemps(6),
               long_game_coeff(1.4),
               limit_expansion(2)
    {}
};
