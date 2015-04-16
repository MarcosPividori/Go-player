#ifndef __MCTS__
#define __MCTS__

#include <vector>
#include <cstddef>
#include <ctime>
#include <mutex>

using namespace std;

template <class Nod> class Selection{
    public:
        virtual Nod *select(const Nod *nod)=0;
        virtual ~Selection() {};
};

template <class Nod,class State> class Expansion{
    public:
        virtual Nod *expand(Nod *nod,State *state)=0;
        virtual ~Expansion() {};
};

template <class Value,class State> class Simulation{
    public:
        virtual Value simulate(State *state)=0;
        virtual ~Simulation() {};
};

template <class Value,class Nod> class Retropropagation{
    public:
        virtual void retro(Nod *node,Value value)=0;
        virtual ~Retropropagation() {};
};

template <class Data,class Nod> class SelectRes{
    public:
        virtual Data select_res(Nod *node)=0;
        virtual ~SelectRes() {};
};

template <class Value,class Data,class Nod,class State>
class Mcts{
    private:
        Selection<Nod> *_sel;
        Expansion<Nod,State> *_exp;
        Simulation<Value,State> *_sim;
        Retropropagation<Value,Nod> *_ret;
        SelectRes<Data,Nod> *_sel_res;
        mutex *_mutex;
    public:
        Mcts(Selection<Nod> *sel,
             Expansion<Nod,State> *exp,
             Simulation<Value,State> *sim,
             Retropropagation<Value,Nod> *ret,
             SelectRes<Data,Nod> *sel_res,
             mutex *mutex=NULL);
        void run_time(double time_limit,
                 Nod *root,
                 State *init_state);
        void run_cycles(unsigned long cycles_limit,
                 Nod *root,
                 State *init_state);
        Data get_resultant_move(Nod *root);
        void set_mutex(mutex *mutex) {_mutex=mutex;};
    private:
        void run_one_cycle(Nod *root,
                           State *init_state);
};

///////////////////////////////////////////////////////////////////

template <class Value,class Data,class Nod,class State>
Mcts<Value,Data,Nod,State>::Mcts(Selection<Nod> *sel,
                           Expansion<Nod,State> *exp,
                           Simulation<Value,State> *sim,
                           Retropropagation<Value,Nod> *ret,
                           SelectRes<Data,Nod> *sel_res,
                           mutex *mutex)
                         : _sel(sel),_exp(exp),_sim(sim),_ret(ret),_sel_res(sel_res),_mutex(mutex)
{}

template <class Value,class Data,class Nod,class State>
inline void Mcts<Value,Data,Nod,State>::run_one_cycle(Nod *root,
                                                      State *init_state)
{
    Nod *node,*after,*before;
    Value res;
    State actual_state(init_state);
    node=root;
    if(_mutex){
        //Selection
        _mutex->lock();
        while((after=_sel->select(node))){
            node=after;
            actual_state.apply(node->data);
        }
        //Expansion
        before=node;
        if((node=_exp->expand(node,&actual_state)) != before)
            actual_state.apply(node->data);
        _mutex->unlock();
        //Simulation
        res= _sim->simulate(&actual_state);
        //Retropropagation
        _mutex->lock();
        _ret->retro(node,res);
        _mutex->unlock();
    }
    else
    {
        //Selection
        while((after=_sel->select(node))){
            node=after;
            actual_state.apply(node->data);
        }
        //Expansion
        before=node;
        if((node=_exp->expand(node,&actual_state)) != before)
            actual_state.apply(node->data);
        //Simulation
        res= _sim->simulate(&actual_state);
        //Retropropagation
        _ret->retro(node,res);
    }
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
