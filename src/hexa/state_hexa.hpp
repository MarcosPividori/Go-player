#include "defines.h"

class StateHexa {
 private:
  int _size;
  Cell **A;
  bool check_vertical(int i, int j, Cell p, bool** visited) const;
  bool check_horizontal(int i, int j, Cell p, bool** visited) const;
 public:
  Player turn;
  StateHexa(int size);
  StateHexa(StateHexa* src);
  ~StateHexa();
  void get_possible_moves(std::vector<DataHexa> &v) const;
  void apply(DataHexa);
  ValHexa get_final_value() const;
  bool valid_move(DataHexa) const;
  void show() const;
};
