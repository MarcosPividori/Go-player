#ifndef __MCTS_GO__
#define __MCTS_GO__

#include <random>
#include "mcts_rave.hpp"
#include "state_go.hpp"

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
class SimulationWithDomainKnowledge: public SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT> {
    protected:
        int mov_counter;
        int _fill_board_n;
        double _long_game_coeff;
        void get_possible_moves(State *state,std::vector<Data> &v,std::uniform_int_distribution<int> &mov_dist);
    public:
        SimulationWithDomainKnowledge(int number_fill_board_attemps,double long_game_coeff): SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>(),_fill_board_n(number_fill_board_attemps), _long_game_coeff(long_game_coeff) {};
        Value simulate(State *state);
};

template <class Node>
class SelectResMostRobustOverLimit: public SelectRes<ValGo,DataGo,Node> {
    private:
        double _limit;
    public:
        SelectResMostRobustOverLimit(double limit):_limit(limit){};
        DataGo select_res(Node *node);
};

/////////////////////////////////////////////////////////////////////////////////////////

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
Value SimulationWithDomainKnowledge<Value,Data,State,EvalNode,MoveRecorderT>::simulate(State *state)
{
    std::uniform_int_distribution<int> mov_dist(0, state->size()-1);
    SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::recorder.clear();
    std::vector<Data> v;
    Data mov;
    mov_counter=0;
    int n;
    get_possible_moves(state,v,mov_dist);
    while(1){
        if(v.empty())
          if(n=state->possible_moves_size()){
            std::uniform_int_distribution<int> dist(0,n-1);
            mov = state->get_possible_moves_by_index(dist(SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::mt));
          }
          else
            break;
        else{
          std::uniform_int_distribution<int> dist(0,v.size()-1);
          mov = v[dist(SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::mt)];
        }
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
    if(mov_counter>(state->size() * state->size() * _long_game_coeff))
        return;
    if(state->get_atari_escape_moves(v),!v.empty())
        if(v.size()>16)
            return;
    for(int c=0;c<_fill_board_n;c++){
        i=mov_dist(SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::mt);
        j=mov_dist(SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::mt);
        if(state->is_completely_empty(i,j)){
            v.push_back(DataGo(i,j,state->turn));
            return;
        }
    }
    state->get_pattern_moves(v);
    state->get_capture_moves(v);
}


template <class Node>
DataGo SelectResMostRobustOverLimit<Node>::select_res(Node *node)
{
    assert(!node->children.empty());
    Node *max_node = *(node->children.begin());
    unsigned long max_visits = max_node->visits;
    for(auto it=node->children.begin();it!=node->children.end();++it)
        if((*it)->visits > max_visits){
            max_node = *it;
            max_visits = (*it)->visits;
        }
    if(max_node->visits!=0 && (max_node->value / max_node->visits) < _limit)
        return RESIGN(max_node->data.player);
    return max_node->data;
}

#endif // __MCTS_GO__
