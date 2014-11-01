#ifndef __MCTS__
#define __MCTS__

#include <vector>
#include <cstddef>
#include <ctime>
#include <mutex>
#include "states.hpp"

template <class Value,class Data,class Nod> class Selection{
    public:
        virtual Nod *select(const Nod *nod)=0;
};

template <class Value,class Data,class Nod,class State> class Expansion{
    public:
        virtual Nod *expand(Nod *nod,State *state)=0;
};

template <class Value,class Data,class State> class Simulation{
    public:
        virtual Value simulate(State *state)=0;
};

template <class Value,class Data,class Nod> class Retropropagation{
    public:
        virtual void retro(Nod *node,Value value)=0;
};

template <class Value,class Data,class Nod> class SelectRes{
    public:
        virtual Data select_res(Nod *node)=0;
};

template <class Value,class Data,class Nod,class State>
class Mcts{
    private:
        Selection<Value,Data,Nod> *_sel;
        Expansion<Value,Data,Nod,State> *_exp;
        Simulation<Value,Data,State> *_sim;
        Retropropagation<Value,Data,Nod> *_ret;
        SelectRes<Value,Data,Nod> *_sel_res;
        std::mutex *_mutex;
    public:
        Mcts(Selection<Value,Data,Nod> *sel,
             Expansion<Value,Data,Nod,State> *exp,
             Simulation<Value,Data,State> *sim,
             Retropropagation<Value,Data,Nod> *ret,
             SelectRes<Value,Data,Nod> *sel_res,
             std::mutex *mutex);
        void run_time(double time_limit,
                 Nod *root,
                 State *init_state);
        void run_cycles(unsigned long cycles_limit,
                 Nod *root,
                 State *init_state);
        Data get_resultant_move(Nod *root);
    private:
        void run_one_cycle(Nod *root,
                           State *init_state);
};

///////////////////////////////////////////////////////////////////

template <class Value,class Data,class Nod,class State>
Mcts<Value,Data,Nod,State>::Mcts(Selection<Value,Data,Nod> *sel,
                           Expansion<Value,Data,Nod,State> *exp,
                           Simulation<Value,Data,State> *sim,
                           Retropropagation<Value,Data,Nod> *ret,
                           SelectRes<Value,Data,Nod> *sel_res,
                           std::mutex *mutex)
                         : _sel(sel),_exp(exp),_sim(sim),_ret(ret),_sel_res(sel_res),_mutex(mutex)
{}

template <class Value,class Data,class Nod,class State>
inline void Mcts<Value,Data,Nod,State>::run_one_cycle(Nod *root,
                                                      State *init_state)
{
    Nod *node,*after,*before;
    Value res;
    State *actual_state=init_state->copy();
    node=root;
    //Selection
    _mutex->lock();
    while((after=_sel->select(node))){
        node=after;
        actual_state->apply(node->data);
    }
    //Expansion
    before=node;
    if((node=_exp->expand(node,actual_state)) != before)
        actual_state->apply(node->data);
    _mutex->unlock();
    //Simulation
    res= _sim->simulate(actual_state);
    //Retropropagation
    _mutex->lock();
    _ret->retro(node,res);
    _mutex->unlock();
    delete actual_state;
}

template <class Value,class Data,class Nod,class State>
void Mcts<Value,Data,Nod,State>::run_time(double time_limit,
                                          Nod *root,
                                          State *init_state)
{
    time_t time_init=time(NULL);
    for(;difftime(time(NULL),time_init)<time_limit;)
        run_one_cycle(root,init_state);
}

template <class Value,class Data,class Nod,class State>
void Mcts<Value,Data,Nod,State>::run_cycles(unsigned long cycles_limit,
                                            Nod *root,
                                            State *init_state)
{
    for(unsigned long c=0;c<cycles_limit;c++)
        run_one_cycle(root,init_state);
}

template <class Value,class Data,class Nod,class State>
Data Mcts<Value,Data,Nod,State>::get_resultant_move(Nod *root)
{
    return _sel_res->select_res(root);
}

#endif // __MCTS__
