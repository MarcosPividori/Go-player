
#include "mcts.hpp"
#include <cstddef>
#include <cassert>
#include <random>

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
//        clock_t counter1,counter2,counter3;    
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
//          std::cout<<"SIZE OF V: "<<v.size()<<std::endl;
//          state->show();
#endif
            return nod;
        }
        for(int i=0;i<v.size();i++)
            nod->create_child(_val,v[i]);
        //TODO: Make this selection randomly!
        return nod->children[0];
    }
    return nod;
}

template <class Value,class Data,class State>
SimulationTotallyRandom<Value,Data,State>::SimulationTotallyRandom()
{
    std::random_device rd;
    mt.seed(rd());
//    counter1=0,counter2=0,counter3=0;    
}

template <class Value,class Data,class State>
Value SimulationTotallyRandom<Value,Data,State>::simulate(State *state)
{
    std::vector<Data> v;
    state->get_possible_moves(v);
//    clock_t time_init;    
    while(!v.empty()){
//        time_init=clock();    
        std::uniform_int_distribution<int> dist(0, v.size()-1);
//        counter1+=clock()-time_init;
//        time_init=clock();    
        state->apply(v[dist(mt)]);
//        counter2+=clock()-time_init;
        v.clear();
//        time_init=clock();    
        state->get_possible_moves(v);
//        counter3+=clock()-time_init;
    }
//    std::cout<<"STATS SIMULATION: random: "<<counter1<<" apply: "<<counter2<<" possible_moves: "<<counter3<<std::endl;
    return state->get_final_value();
}

template <class Value,class Data,class Node>
Data SelectResMostRobust<Value,Data,Node>::select_res(Node *node)
{
    assert(!node->children.empty());
    unsigned long max_visits = node->children[0]->visits;
    Node *max_node = node->children[0];
    for(int i=1;i<node->children.size();i++)
        if(node->children[i]->visits > max_visits){
            max_node = node->children[i];
            max_visits = node->children[i]->visits;
        }
    return max_node->data;
}
