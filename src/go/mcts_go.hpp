
#include "mcts_rave.hpp"
#include <random>
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
        std::uniform_int_distribution<int> dist(0,v.size()-1);
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
    if(state->get_pattern_moves(v),!v.empty()){
      state->get_capture_moves(v);
      return;
    }
    state->get_capture_moves(v);
    state->get_possible_moves(v);
}


template <class Node>
class SelectResMostRobustOverLimit: public SelectRes<ValGo,DataGo,Node> {
    private:
        double _limit;
    public:
        SelectResMostRobustOverLimit(double limit):_limit(limit){};
        DataGo select_res(Node *node);
};

template <class Node>
DataGo SelectResMostRobustOverLimit<Node>::select_res(Node *node)
{
    assert(!node->children.empty());
    unsigned long max_visits = node->children[0]->visits;
    Node *max_node = node->children[0];
    for(int i=1;i<node->children.size();i++)
        if(node->children[i]->visits > max_visits){
            max_node = node->children[i];
            max_visits = node->children[i]->visits;
        }
    if(max_node->visits!=0 && (max_node->value / max_node->visits) < _limit)
        return RESIGN(max_node->data.player);
    return max_node->data;
}
