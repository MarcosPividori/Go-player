
#include "mcts.hpp"
#include <cstddef>
#include <cmath>
#include <cassert>
#include <random>
#include <climits>

template <class Value,class Data>
class SelectionUCT: public Selection<Value,Data, NodeUCT<Value,Data> >{
    private:
        double _coeff;
        double get_uct_val(NodeUCT<Value,Data> *nod,double log_parent);
    public:
        SelectionUCT(double coeff);
        NodeUCT<Value,Data>* select(NodeUCT<Value,Data> *nod);
};

template <class Value,class Data,class State>
class ExpansionAllChildren: public Expansion<Value,Data, NodeUCT<Value,Data>,State> {
    private:
        int _lim;
        Value _val;
    public:
#ifdef DEBUG
        long counter;
#endif
        ExpansionAllChildren(int lim,Value init_val);
        NodeUCT<Value,Data>* expand(NodeUCT<Value,Data> *nod,State *state);
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

template <class Value,class Data,class EvalNode>
class RetropropagationSimple: public Retropropagation<Value,Data, NodeUCT<Value,Data> > {
    private:
        EvalNode _eval_fun;
    public:
        RetropropagationSimple(EvalNode fun);
        void retro(NodeUCT<Value,Data> *node,Value value);
};

template <class Value,class Data>
class SelectResMostRobust: public SelectRes<Value,Data, NodeUCT<Value,Data> > {
    public:
        Data select_res(NodeUCT<Value,Data> *node);
};

////////////////////////////////////////////////////////////////////

template <class Value,class Data>
SelectionUCT<Value,Data>::SelectionUCT(double coeff) : _coeff(coeff)
{}

template <class Value,class Data>
inline double SelectionUCT<Value,Data>::get_uct_val(NodeUCT<Value,Data> *nod, double sqrt_log_parent)
{
    double val;
    nod->mutex.lock();
    if(nod->visits==0)
        val=-1;
    else
        val=(((double) nod->value) / (double) nod->visits) +
               _coeff * sqrt_log_parent / nod->sqrt_visits;
    nod->mutex.unlock();
    return val;
}

template <class Value,class Data>
NodeUCT<Value,Data>* SelectionUCT<Value,Data>::select(NodeUCT<Value,Data> *nod)
{
    nod->mutex.lock();
    //TODO: Make the selection of no visited nodes randomly!
    if(nod->children.empty())
        return NULL;
    NodeUCT<Value,Data> *max_nod= nod->children[0];
    double max_val,val,sqrt_log_parent = sqrt(log((double) nod->visits));
    nod->mutex.unlock();
    if((max_val = get_uct_val(nod->children[0],sqrt_log_parent))==-1)
        return max_nod;
    for(int i=1;i<nod->children.size();i++){
        if((val=get_uct_val(nod->children[i],sqrt_log_parent))==-1)
            return nod->children[i];
        if(val>max_val){
            max_val = val;
            max_nod = nod->children[i];
        }
    }
    return max_nod;
}

template <class Value,class Data,class State>
ExpansionAllChildren<Value,Data,State>::ExpansionAllChildren(int lim,Value init_val) : _lim(lim),_val(init_val)
#ifdef DEBUG
,counter(0)
#endif
{}

template <class Value,class Data,class State>
NodeUCT<Value,Data>* ExpansionAllChildren<Value,Data,State>::expand(NodeUCT<Value,Data> *nod,
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

template <class Value,class Data,class EvalNode>
RetropropagationSimple<Value,Data,EvalNode>::RetropropagationSimple(EvalNode fun) : _eval_fun(fun)
{}

template <class Value,class Data,class EvalNode>
void RetropropagationSimple<Value,Data,EvalNode>::retro(NodeUCT<Value,Data> *node,
                                   Value value)
{
    do{
        node->mutex.lock();
        node->value=_eval_fun(node->value,value,node->data);
        assert(node->visits<ULONG_MAX);
        node->set_visits(node->visits+1);
        node->mutex.unlock();
    }while((node=node->parent)!=NULL);
}

template <class Value,class Data>
Data SelectResMostRobust<Value,Data>::select_res(NodeUCT<Value,Data> *node)
{
    assert(!node->children.empty());
    unsigned long max_visits = node->children[0]->visits;
    NodeUCT<Value,Data> *max_node = node->children[0];
    for(int i=1;i<node->children.size();i++)
        if(node->children[i]->visits > max_visits){
            max_node = node->children[i];
            max_visits = node->children[i]->visits;
        }
    return max_node->data;
}
