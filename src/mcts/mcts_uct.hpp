#ifndef __MCTS_UCT__
#define __MCTS_UCT__

#include "mcts.hpp"
#include "node.hpp"
#include <cassert>
#include <climits>
#include <cmath>
#include <cstddef>

template<class Value, class Data>
class SelectionUCT : public Selection<NodeUCT<Value, Data>> {
 private:
  double _coeff;
#ifdef DEBUG
 public:
#endif
  double get_uct_val(const NodeUCT<Value, Data> *nod, double log_parent);
 public:
  SelectionUCT(double coeff);
  NodeUCT<Value, Data> *select(const NodeUCT<Value, Data> *nod);
};

template<class Value, class Data>
struct EvalNode {
  virtual Value operator() (Value v_nodo, Value v_final, Data dat_nodo) = 0;
};

template<class Value, class Data, class EvalNode>
class RetropropagationSimple : public Retropropagation<Value, NodeUCT<Value, Data>> {
 private:
  EvalNode _eval_fun;
 public:
  void retro(NodeUCT<Value, Data> *node, Value value);
};

////////////////////////////////////////////////////////////////////

template<class Value, class Data>
SelectionUCT<Value, Data>::SelectionUCT(double coeff) : _coeff(coeff) {}

template<class Value, class Data>
inline double SelectionUCT<Value, Data>::
get_uct_val(const NodeUCT<Value, Data>* nod,
            double sqrt_log_parent) {
  if (nod->get_visits() == 0)
    return -1;
  return (((double) nod->value) / (double) nod->get_visits()) +
      _coeff * sqrt_log_parent / nod->get_sqrt_visits();
}

template<class Value, class Data>
NodeUCT<Value, Data>* SelectionUCT<Value, Data>::
select(const NodeUCT<Value, Data> *nod) {
  typename NodeUCT<Value, Data>::const_iterator iter = nod->children_begin();
  if (iter == nod->children_end())
    return NULL;
  NodeUCT<Value, Data> *max_nod = *iter;
  double max_val, val, sqrt_log_parent = sqrt(log((double) nod->get_visits()));
  if ((max_val = get_uct_val(*iter, sqrt_log_parent)) == -1)
    return max_nod;
  for (iter++; iter != nod->children_end(); iter++) {
    if ((val = get_uct_val(*iter, sqrt_log_parent)) == -1)
      return *iter;
    if (val > max_val) {
      max_val = val;
      max_nod = *iter;
    }
  }
  return max_nod;
}

template<class Value, class Data, class EvalNode>
void RetropropagationSimple<Value, Data, EvalNode>::
retro(NodeUCT<Value, Data>* node, Value value) {
  do {
    node->value = _eval_fun(node->value, value, node->data);
    assert(node->get_visits() < ULONG_MAX);
    node->set_visits(node->get_visits() + 1);
  } while (node = node->get_parent());
}

#endif                          // __MCTS_UCT__
