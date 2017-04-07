#ifndef __MCTS_UTILS__
#define __MCTS_UTILS__

#include "mcts.hpp"
#include <cassert>
#include <cstddef>
#include <random>

template<class Value, class Data, class State, class Node>
class ExpansionAllChildren : public Expansion<Node, State> {
 private:
  int _lim;
  Value _val;
 public:
#ifdef DEBUG
  long counter;
#endif
  ExpansionAllChildren(int lim, Value init_val);
  Node* expand(Node* nod, State* state);
};

template<class Value, class Data, class State>
class SimulationTotallyRandom : public Simulation<Value, State> {
 private:
  std::mt19937 mt;
 public:
  SimulationTotallyRandom();
  Value simulate(State* state);
};

template<class Value, class Data, class Node>
class SelectResMostRobust : public SelectRes<Data, Node> {
 public:
  Data select_res(Node* node);
};

////////////////////////////////////////////////////////////////////

template<class Value, class Data, class State, class Node>
ExpansionAllChildren<Value, Data, State, Node>::
ExpansionAllChildren(int lim, Value init_val) : _lim(lim), _val(init_val)
#ifdef DEBUG
, counter(0)
#endif
{}

template<class Value, class Data, class State, class Node>
Node* ExpansionAllChildren<Value, Data, State, Node>::
expand(Node* nod, State* state) {
  assert(nod->children_begin() == nod->children_end());
  if (!nod->get_parent()        //Root node.
      || nod->get_visits() >= _lim) {
    std::vector<Data> v;
    state->get_possible_moves(v);
    if (v.empty()) {
#ifdef DEBUG
      counter++;
#endif
      return nod;
    }
    for (int i = 0; i < v.size(); i++)
      nod->create_child(_val, v[i]);
    return *(nod->children_begin());
  }
  return nod;
}

template<class Value, class Data, class State>
SimulationTotallyRandom<Value, Data, State>::SimulationTotallyRandom() {
  std::random_device rd;
  mt.seed(rd());
}

template<class Value, class Data, class State>
Value SimulationTotallyRandom<Value, Data, State>::simulate(State* state) {
  std::vector<Data> v;
  state->get_possible_moves(v);
  while (!v.empty()) {
    std::uniform_int_distribution<int> dist(0, v.size() - 1);
    state->apply(v[dist(mt)]);
    v.clear();
    state->get_possible_moves(v);
  }
  return state->get_final_value();
}

template<class Value, class Data, class Node>
Data SelectResMostRobust<Value, Data, Node>::select_res(Node* node) {
  assert(node->children_begin() != node->children_end());
  typename Node::const_iterator iter = node->children_begin();
  unsigned long max_visits = (*iter)->get_visits();
  Node *max_node = *iter;
  for (iter++; iter != node->children_end(); iter++)
    if ((*iter)->get_visits() > max_visits) {
      max_node = *iter;
      max_visits = (*iter)->get_visits();
    }
  return max_node->data;
}

#endif                          // __MCTS_UTILS__
