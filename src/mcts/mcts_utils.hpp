#ifndef __MCTS_UTILS__
#define __MCTS_UTILS__

#include "mcts.hpp"
#include <cstddef>
#include <cassert>
#include <random>
#include <iterator>

template <class Value,class Data,class State,class Node>
class ExpansionAllChildren: public Expansion<Value,Data,Node,State> {
    private:
        int _lim;
        Value _val;
    public:
#ifdef DEBUG
        long counter;
#endif
        ExpansionAllChildren(int lim,Value init_val);
        Node* expand(Node *nod,State *state);
};

template <class Value,class Data,class State>
class SimulationTotallyRandom: public Simulation<Value,Data,State> {
    private:
        std::mt19937 mt;
    public:
        SimulationTotallyRandom();
        Value simulate(State *state);
};

template <class Value,class Data,class Node>
class SelectResMostRobust: public SelectRes<Value,Data,Node> {
    public:
        Data select_res(Node *node);
};

////////////////////////////////////////////////////////////////////

template <class Value,class Data,class State,class Node>
ExpansionAllChildren<Value,Data,State,Node>::ExpansionAllChildren(int lim,Value init_val) : _lim(lim),_val(init_val)
#ifdef DEBUG
,counter(0)
#endif
{}

template <class Value,class Data,class State,class Node>
Node* ExpansionAllChildren<Value,Data,State,Node>::expand(Node *nod,
                                            State *state)
{
    assert(nod->children.empty());
    if(nod->parent == NULL //Root node.
     || nod->visits >= _lim){
        std::vector<Data> v;
        state->get_possible_moves(v);
        if(v.empty()){
#ifdef DEBUG
            counter++;
#endif
            return nod;
        }
        for(int i=0;i<v.size();i++)
            nod->create_child(_val,v[i]);
        //TODO: Make this selection randomly!
        return *(nod->children.begin());
    }
    return nod;
}

template <class Value,class Data,class State>
SimulationTotallyRandom<Value,Data,State>::SimulationTotallyRandom()
{
    std::random_device rd;
    mt.seed(rd());
}

template <class Value,class Data,class State>
Value SimulationTotallyRandom<Value,Data,State>::simulate(State *state)
{
    std::vector<Data> v;
    state->get_possible_moves(v);
    while(!v.empty()){
        std::uniform_int_distribution<int> dist(0, v.size()-1);
        state->apply(v[dist(mt)]);
        v.clear();
        state->get_possible_moves(v);
    }
    return state->get_final_value();
}

template <class Value,class Data,class Node>
Data SelectResMostRobust<Value,Data,Node>::select_res(Node *node)
{
    assert(!node->children.empty());
    Node *max_node = (*(node->children.begin()));
    unsigned long max_visits = max_node->visits;
    for(std::iterator<std::input_iterator_tag, Node* > it=node->children.begin();it!=node->children.end();++it)
        if((*it)->visits > max_visits){
            max_node = *it;
            max_visits = (*it)->visits;
        }
    return max_node->data;
}

#endif // __MCTS_UTILS__
