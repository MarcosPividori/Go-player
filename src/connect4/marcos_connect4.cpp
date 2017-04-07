#include "mcts_parallel.hpp"
#include "mcts_uct.hpp"
#include "mcts_utils.hpp"
#include "state_connect4.hpp"
#include <iostream>

struct EvalNod : public EvalNode<ValConnect4, DataConnect4> {
  ValConnect4 operator() (ValConnect4 v_nodo,
                          ValConnect4 v_final, DataConnect4 dat_nodo) {
    if (v_final == EMPTY)       //Tie
      return v_nodo + 0.9999;
    if (v_final == PlayerToCell(dat_nodo.player))
      return v_nodo + 1;
    return v_nodo;
  }
};

Player insert_player() {
  char c;
  while (1) {
    std::cout << "First(f) or Second(s) player?: ";
    if ((std::cin >> c) && (c == 'f' || c == 's'))
      break;
    std::cin.clear();
    std::cin.ignore(10000, '\n');
  }
  return (c == 'f') ? CROSS_P : CIRCLE_P;
}

DataConnect4 insert_mov(Player player, StateConnect4* state) {
  int j;
  while (1) {
    std::cout << "Insert mov (Column num 0-6): ";
    if ((std::cin >> j) && (j >= 0 && j < 7)
        && state->valid_move(DataConnect4(j, player)))
      break;
    std::cin.clear();
    std::cin.ignore(10000, '\n');
  }
  std::cout << std::endl;
  return DataConnect4(j, player);
}

int main() {
  StateConnect4 state;
  DataConnect4 res;
  std::vector<DataConnect4> v;

  SelectionUCT<ValConnect4, DataConnect4> sel(1);
  ExpansionAllChildren<ValConnect4, DataConnect4, StateConnect4, Nod> exp(2,
                                                                           0);
  SimulationTotallyRandom<ValConnect4, DataConnect4, StateConnect4> sim;
  RetropropagationSimple<ValConnect4, DataConnect4, EvalNod> ret;
  SelectResMostRobust<ValConnect4, DataConnect4, Nod> sel_res;
  Mcts<ValConnect4, DataConnect4, Nod, StateConnect4>
      m(&sel, &exp, &sim, &ret, &sel_res);
  std::vector<Mcts<ValConnect4, DataConnect4, Nod, StateConnect4> *>
      m_vector(NUM_THREADS, &m);
  MctsParallel_GlobalMutex<ValConnect4, DataConnect4, Nod, StateConnect4>
      mcts(m_vector, &state, DataConnect4(0, CIRCLE_P));

  std::cout << "CONNNECT4:" << std::endl;
  Player us_player = insert_player();

  std::cout << "------------------" << std::endl << std::endl;
  state.show();

  while ((v.clear(), state.get_possible_moves(v), !v.empty())) {
    if (state.turn == us_player) {
      std::cout << std::endl;
      std::cout << "-You play---------" << std::endl << std::endl;
      res = insert_mov(us_player, &state);
    } else {
      std::cout << std::endl;
      std::cout << "-Computer plays---" << std::endl << std::endl;
      mcts.run_time(MAX_SECONDS);
      //mcts.run_cycles(NUM_CYCLES);
      res = mcts.get_resultant_move();
    }

    mcts.apply_move(res);

    state.show();
  }

  std::cout << std::endl << "------------------" << std::endl << std::endl;
  std::cout << "RESULT: "
      << (state.get_final_value() == CROSS ? "X wins." :
          (state.get_final_value() == CIRCLE ? "O wins." : "Tie."))
      << std::endl << std::endl;
  return 0;
}
