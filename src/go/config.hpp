struct Config {
  const char *pattern_file;
  double bandit_coeff;
  double amaf_coeff;
  double number_fill_board_attemps;
  double long_game_coeff;
  double pattern_coeff;
  double capture_coeff;
  double atari_delete_coeff;
  double atari_escape_coeff;
  int limit_atari;
  int limit_expansion;
  int number_cycles_mcts;
  int num_threads_mcts;
  double resign_limit;
  bool root_parallel;
  bool japanese_rules;
  bool rave;
  bool knowledge;
  Config() :
      pattern_file(nullptr),
      bandit_coeff(0),
      amaf_coeff(2500),
      number_fill_board_attemps(1),
      long_game_coeff(3),
      pattern_coeff(6),
      capture_coeff(4),
      atari_delete_coeff(4),
      atari_escape_coeff(1),
      limit_atari(16),
      limit_expansion(1),
      number_cycles_mcts(30000),
      num_threads_mcts(5),
      resign_limit(0.1),
      root_parallel(false),
      japanese_rules(false),
      rave(true),
      knowledge(true) {}
};
