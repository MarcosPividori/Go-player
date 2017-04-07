#include "defines.h"

class StateConnect4 {
 private:
  Cell A[6][7];
 public:
  Player turn;
  StateConnect4();
  StateConnect4(StateConnect4* src);
  void get_possible_moves(std::vector<DataConnect4> &v) const;
  void apply(DataConnect4 move);
  ValConnect4 get_final_value() const;
  bool valid_move(DataConnect4 move) const;
  void show() const;
};
