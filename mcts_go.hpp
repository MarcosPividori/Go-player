
#include "mcts_rave.hpp"
#include <random>
#include "state_go.hpp"

#define FILL_BOARD_N     6
#define COEFF_LONG_GAME  1.4

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
class SimulationWithDomainKnowledge: public SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT> {
    protected:
        int mov_counter;
        void get_possible_moves(State *state,std::vector<Data> &v,std::uniform_int_distribution<int> &mov_dist);
    public:
        SimulationWithDomainKnowledge();
        Value simulate(State *state);
};

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
SimulationWithDomainKnowledge<Value,Data,State,EvalNode,MoveRecorderT>::SimulationWithDomainKnowledge() : SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>()
{}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
Value SimulationWithDomainKnowledge<Value,Data,State,EvalNode,MoveRecorderT>::simulate(State *state)
{
    std::uniform_int_distribution<int> mov_dist(0, state->size()-1);
    SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::recorder.clear();
    std::vector<Data> v;
    Data mov;
    mov_counter=0;
    get_possible_moves(state,v,mov_dist);
    while(!v.empty()){
        std::uniform_int_distribution<int> dist(0, v.size()-1);
        mov = v[dist(SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::mt)];
        state->apply(mov);
        SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::recorder.postMove(mov);
        v.clear();
        get_possible_moves(state,v,mov_dist);
    }
    return state->get_final_value();
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
inline void SimulationWithDomainKnowledge<Value,Data,State,EvalNode,MoveRecorderT>::get_possible_moves(State *state,std::vector<Data> &v,std::uniform_int_distribution<int> &mov_dist)
{
    mov_counter++;
    INDEX i,j;
    for(int c=0;c<FILL_BOARD_N;c++){
        i=mov_dist(SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::mt);
        j=mov_dist(SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::mt);
        if(state->is_completely_empty(i,j)){
            v.push_back({state->turn,i,j});
            return;
        }
    }
    if(state->get_pattern_moves(v),v.empty())
      if(mov_counter>(state->size() * state->size() * COEFF_LONG_GAME))
        state->get_possible_moves(v);
      else
        if(state->get_capture_moves(v),v.empty())
          state->get_possible_moves(v);
}

