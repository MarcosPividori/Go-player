
#include "mcts_go.hpp"

ValGo SimulationWithDomainKnowledge::simulate(StateGo *state)
{
    uniform_int_distribution<int> mov_dist(0, state->size()-1);
    SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>
        ::recorder.clear();
    vector<DataGo> v;
    DataGo mov;
    mov_counter=0;
    mov_limit= state->size() * state->size() * _long_game_coeff;
    int n;
    get_possible_moves(state,v,mov_dist);
    while(1){
        if(v.empty())
            if((n=state->possible_moves_size()) &&  mov_counter<mov_limit){
                uniform_int_distribution<int> dist(0,n-1);
                mov = state->get_possible_moves_by_index(dist(
                    SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,
                    EvalNod,MoveRecorderGo>::mt));
            }
            else
                break;
        else{
            uniform_int_distribution<int> dist(0,v.size()-1);
            mov = v[dist(SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,
                EvalNod,MoveRecorderGo>::mt)];
        }
        state->apply(mov);
        SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,
            MoveRecorderGo>::recorder.postMove(mov);
        v.clear();
        get_possible_moves(state,v,mov_dist);
    }
    return state->get_final_value();
}

inline void SimulationWithDomainKnowledge::get_possible_moves(
    StateGo *state,
    vector<DataGo> &v,
    uniform_int_distribution<int> &mov_dist)
{
    mov_counter++;
    INDEX i,j;
    if(mov_counter>=mov_limit)
        return;
    if(state->get_atari_escape_moves(v),!v.empty())
        if(v.size()>_limit_atari)
            return;
    for(int c=0;c<_fill_board_n;c++){
        i=mov_dist(SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,
            EvalNod,MoveRecorderGo>::mt);
        j=mov_dist(SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,
            EvalNod,MoveRecorderGo>::mt);
        if(state->is_completely_empty(i,j)){
            v.push_back(DataGo(i,j,state->turn));
            return;
        }
    }
    state->get_pattern_moves(v);
    state->get_capture_moves(v);
}

ValGo SimulationAndRetropropagationRaveGo::simulate(StateGo *state)
{
    SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,
        EvalNod,MoveRecorderGo>::recorder.clear();
    DataGo mov;
    int mov_counter=0;
    int mov_limit= state->size() * state->size() * _long_game_coeff;
    int size;
    while((size=state->possible_moves_size()) && mov_counter<mov_limit){
        mov_counter++;
        uniform_int_distribution<int> dist(0, size-1);
        mov = state->get_possible_moves_by_index(dist(
            SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,
            EvalNod,MoveRecorderGo>::mt));
        state->apply(mov);
        SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,
            MoveRecorderGo>::recorder.postMove(mov);
    }
    return state->get_final_value();
}

SimulationTotallyRandomGo::SimulationTotallyRandomGo(double long_game_coeff) :
    _long_game_coeff(long_game_coeff)
{
    random_device rd;
    mt.seed(rd());
}

ValGo SimulationTotallyRandomGo::simulate(StateGo *state)
{
    int mov_counter=0;
    int mov_limit= state->size() * state->size() * _long_game_coeff;
    int size;
    while((size=state->possible_moves_size()) && mov_counter<mov_limit){
        mov_counter++;
        uniform_int_distribution<int> dist(0, size-1);
        state->apply(state->get_possible_moves_by_index(dist(mt)));
    }
    return state->get_final_value();
}

