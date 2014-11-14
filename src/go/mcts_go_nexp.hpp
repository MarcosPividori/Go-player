
#include <random>
#include "mcts_rave_nexp.hpp"
#include "state_go.hpp"

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
class SimulationWithDomainKnowledgeNExp: public SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT> {
    protected:
        int mov_counter;
        int _fill_board_n;
        double _long_game_coeff;
        void get_possible_moves(State *state,std::vector<Data> &v,std::uniform_int_distribution<int> &mov_dist);
    public:
        SimulationWithDomainKnowledgeNExp(int number_fill_board_attemps,double long_game_coeff): SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>(),_fill_board_n(number_fill_board_attemps), _long_game_coeff(long_game_coeff) {};
        Value simulate(State *state);
};

/////////////////////////////////////////////////////////////////////////////////////////

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
Value SimulationWithDomainKnowledgeNExp<Value,Data,State,EvalNode,MoveRecorderT>::simulate(State *state)
{
    std::uniform_int_distribution<int> mov_dist(0, state->size()-1);
    SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>::recorder.clear();
    std::vector<Data> v;
    Data mov;
    mov_counter=0;
    int n;
    get_possible_moves(state,v,mov_dist);
    while(1){
        if(v.empty())
          if(n=state->possible_moves_size()){
            std::uniform_int_distribution<int> dist(0,n-1);
            mov = state->get_possible_moves_by_index(dist(SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>::mt));
          }
          else
            break;
        else{
          std::uniform_int_distribution<int> dist(0,v.size()-1);
          mov = v[dist(SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>::mt)];
        }
        state->apply(mov);
        SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>::recorder.postMove(mov);
        v.clear();
        get_possible_moves(state,v,mov_dist);
    }
    return state->get_final_value();
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
inline void SimulationWithDomainKnowledgeNExp<Value,Data,State,EvalNode,MoveRecorderT>::get_possible_moves(State *state,std::vector<Data> &v,std::uniform_int_distribution<int> &mov_dist)
{
    mov_counter++;
    INDEX i,j;
    if(mov_counter>(state->size() * state->size() * _long_game_coeff))
        return;
    if(state->get_atari_escape_moves(v),!v.empty())
        if(v.size()>16)
            return;
    for(int c=0;c<_fill_board_n;c++){
        i=mov_dist(SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>::mt);
        j=mov_dist(SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>::mt);
        if(state->is_completely_empty(i,j)){
            v.push_back(DataGo(i,j,state->turn));
            return;
        }
    }
    state->get_pattern_moves(v);
    state->get_capture_moves(v);
}

