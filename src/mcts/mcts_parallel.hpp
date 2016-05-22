#ifndef __MCTS_PARALLEL__
#define __MCTS_PARALLEL__

#include "mcts.hpp"
#include "mcts_utils.hpp"
#include <thread>
#include <mutex>

using namespace std;

template <class Value,class Data,class State>
class MctsParallel{
    public:
#ifdef DEBUG
        virtual void *get_root()=0;
#endif
        virtual void run_time(double time_limit)=0;
        virtual void run_cycles(unsigned long cycles_limit)=0;
        virtual Data get_resultant_move()=0;
        virtual void apply_move(Data mov)=0;
        virtual void reinit(State *init_state,Data init_data)=0;
        virtual ~MctsParallel();
};

template <class Value,class Data,class State>
MctsParallel<Value,Data,State>::~MctsParallel(){}

template <class Value,class Data,class Nod,class State>
class MctsParallel_GlobalMutex : public MctsParallel<Value,Data,State> {
    private:
        mutex _mutex;
        vector<Mcts<Value,Data,Nod,State>* >& _m;
        State *_state;
        Nod *_root;
    public:
        MctsParallel_GlobalMutex(vector<Mcts<Value,Data,Nod,State>* >& m,
             State *init_state,
             Data init_data);
        ~MctsParallel_GlobalMutex();
        void run_time(double time_limit);
        void run_cycles(unsigned long cycles_limit);
        Data get_resultant_move();
        void apply_move(Data mov);
        void reinit(State *init_state,Data init_data);
#ifdef DEBUG
        void *get_root();
#endif
};

template <class Value,class Data,class Nod,class State>
class MctsParallel_Root : public MctsParallel<Value,Data,State> {
    private:
        vector<Mcts<Value,Data,Nod,State>* >& _m;
        ExpansionAllChildren<Value,Data,State,Nod> _exp;
        void merge_trees();
        Nod **_roots_mcts;
        State *_state;
        Nod *_root;
    public:
        MctsParallel_Root(vector<Mcts<Value,Data,Nod,State>* >& m,
             State *init_state,
             Data init_data);
        ~MctsParallel_Root();
        void run_time(double time_limit);
        void run_cycles(unsigned long cycles_limit);
        Data get_resultant_move();
        void apply_move(Data mov);
        void reinit(State *init_state,Data init_data);
#ifdef DEBUG
        void *get_root();
#endif
};

///////////////////////////////////////////////////////////////////

template <class Value,class Data,class Nod,class State>
MctsParallel_GlobalMutex<Value,Data,Nod,State>::MctsParallel_GlobalMutex(
    vector<Mcts<Value,Data,Nod,State>* > &m,
    State *init_state,
    Data init_data) : _state(init_state), _m(m)
{
    assert(_m.size()>0);
    _root= new Nod(0,init_data);
    for(int i=0;i<_m.size();i++)
        _m[i]->set_mutex(&_mutex);
}

template <class Value,class Data,class Nod,class State>
void MctsParallel_GlobalMutex<Value,Data,Nod,State>::run_time(double time_limit)
{
    thread *threads= new thread[_m.size()];
    for(int i=0;i<_m.size();i++)
        threads[i] = thread(&Mcts<Value,Data,Nod,State>::run_time,_m[i],
            time_limit,_root,_state);
    for(int i=0;i<_m.size();i++)
        threads[i].join();
    delete[] threads;
}

template <class Value,class Data,class Nod,class State>
void MctsParallel_GlobalMutex<Value,Data,Nod,State>::run_cycles(unsigned long cycles_limit)
{
    thread *threads= new thread[_m.size()];
    for(int i=0;i<_m.size();i++)
        threads[i] = thread(&Mcts<Value,Data,Nod,State>::run_cycles,_m[i],
            cycles_limit/_m.size(),_root,_state);
    for(int i=0;i<_m.size();i++)
        threads[i].join();
    delete[] threads;
}

template <class Value,class Data,class Nod,class State>
Data MctsParallel_GlobalMutex<Value,Data,Nod,State>::get_resultant_move()
{
    return _m[0]->get_resultant_move(_root);
}

template <class Value,class Data,class Nod,class State>
void MctsParallel_GlobalMutex<Value,Data,Nod,State>::apply_move(Data move)
{
    _state->apply(move);
    Nod *next;
    if(next=_root->move_root_to_child(move)){
        delete _root;
        _root=next;
    }else{
        _root->delete_tree();
        _root= new Nod(0,move);
    }
}

template <class Value,class Data,class Nod,class State>
void MctsParallel_GlobalMutex<Value,Data,Nod,State>::reinit(State *init_state,
                                                            Data init_data)
{   _state=init_state;
    _root->delete_tree();
    _root= new Nod(0,init_data);
}

template <class Value,class Data,class Nod,class State>
MctsParallel_GlobalMutex<Value,Data,Nod,State>::~MctsParallel_GlobalMutex()
{
    _root->delete_tree();
}

#ifdef DEBUG
template <class Value,class Data,class Nod,class State>
void *MctsParallel_GlobalMutex<Value,Data,Nod,State>::get_root()
{
    return _root;
}
#endif

///////////////////////////////////////////////////////////////////////////

template <class Value,class Data,class Nod,class State>
MctsParallel_Root<Value,Data,Nod,State>::MctsParallel_Root(
    vector<Mcts<Value,Data,Nod,State>* > &m,
    State *init_state,
    Data init_data) : _state(init_state), _m(m), _exp(0,0)
{
    assert(_m.size()>0);
    _root= new Nod(0,init_data);
    _roots_mcts = new Nod*[_m.size()];
    for(int i=0;i<_m.size();i++)
        _roots_mcts[i] = new Nod(0,init_data);
}

template <class Value,class Data,class Nod,class State>
void MctsParallel_Root<Value,Data,Nod,State>::run_time(double time_limit)
{
    thread *threads= new thread[_m.size()];
    for(int i=0;i<_m.size();i++)
        threads[i] = thread(&Mcts<Value,Data,Nod,State>::run_time,_m[i],
            time_limit,_roots_mcts[i],_state);
    for(int i=0;i<_m.size();i++)
        threads[i].join();
    delete[] threads;
    merge_trees();
}

template <class Value,class Data,class Nod,class State>
void MctsParallel_Root<Value,Data,Nod,State>::run_cycles(unsigned long cycles_limit)
{
    thread *threads= new thread[_m.size()];
    for(int i=0;i<_m.size();i++)
        threads[i] = thread(&Mcts<Value,Data,Nod,State>::run_cycles,_m[i],
            cycles_limit/_m.size(),_roots_mcts[i],_state);
    for(int i=0;i<_m.size();i++)
        threads[i].join();
    delete[] threads;
    merge_trees();
}

template <class Value,class Data,class Nod,class State>
inline void MctsParallel_Root<Value,Data,Nod,State>::merge_trees()
{
    _exp.expand(_root,_state);
    for(int i=0;i<_m.size();i++){
        _root->join(_roots_mcts[i]);
        typename Nod::const_iterator iter1=_root->children_begin(),
                            iter2=_roots_mcts[i]->children_begin();
        for(;iter2 != _roots_mcts[i]->children_end();iter1++,iter2++){
            assert((*iter1)->data==(*iter2)->data);
            (*iter1)->join(*iter2);
        }
    }
}

template <class Value,class Data,class Nod,class State>
Data MctsParallel_Root<Value,Data,Nod,State>::get_resultant_move()
{
    return _m[0]->get_resultant_move(_root);
}

template <class Value,class Data,class Nod,class State>
void MctsParallel_Root<Value,Data,Nod,State>::apply_move(Data move)
{
    _state->apply(move);
    Nod *next;
    for(int i=0;i<_m.size();i++){
        if(next=_roots_mcts[i]->move_root_to_child(move)){
            delete _roots_mcts[i];
            _roots_mcts[i]=next;
        }else{
            _roots_mcts[i]->delete_tree();
            _roots_mcts[i] = new Nod(0,move);
        }
    }
    if(next=_root->move_root_to_child(move)){
        delete _root;
        _root=next;
    }else{
        _root->delete_tree();
        _root = new Nod(0,move);
    }
}

template <class Value,class Data,class Nod,class State>
void MctsParallel_Root<Value,Data,Nod,State>::reinit(State *init_state,
                           Data init_data)
{   _state=init_state;
    _root->delete_tree();
    _root= new Nod(0,init_data);
    for(int i=0;i<_m.size();i++){
        _roots_mcts[i]->delete_tree();
        _roots_mcts[i] = new Nod(0,init_data);
    }
}

template <class Value,class Data,class Nod,class State>
MctsParallel_Root<Value,Data,Nod,State>::~MctsParallel_Root()
{
    _root->delete_tree();
    for(int i=0;i<_m.size();i++)
        _roots_mcts[i]->delete_tree();
    delete[] _roots_mcts; 
}

#ifdef DEBUG
template <class Value,class Data,class Nod,class State>
void *MctsParallel_Root<Value,Data,Nod,State>::get_root()
{
    return _root;
}
#endif

#endif // __MCTS_PARALLEL__
