#include "defines.h"

class StateTateti {
 private:
  Cell A[3][3];
 public:
  Player turn;
  StateTateti();
  StateTateti(StateTateti* src);
  void get_possible_moves(std::vector< DataTateti> &v) const;
  void apply(DataTateti);
  ValTateti get_final_value() const;
  bool valid_move(DataTateti) const;
  void show() const;
};
