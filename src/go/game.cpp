
#include "game.hpp"
#include <iomanip>
#include <thread>
#include <mutex>

Game::Game(int size,Config &cfg_input) : _komi(0),_size(size),_exp(cfg_input.limit_expansion,0),_cfg(cfg_input),_sel_res(cfg_input.resign_limit),
#ifdef RAVE
_sel(cfg_input.bandit_coeff,cfg_input.amaf_coeff)
#else
_sel(cfg_input.bandit_coeff)
#endif
{
    _m= new Mcts<ValGo,DataGo,Nod,StateGo>*[_cfg.num_threads_mcts];
#ifdef RAVE
    _sim_and_retro= new SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNode,MoveRecorderGo>*[_cfg.num_threads_mcts];
#endif
    for(int i=0;i<_cfg.num_threads_mcts;i++){
#ifdef RAVE
#ifdef KNOWLEDGE
        _sim_and_retro[i]=new SimulationWithDomainKnowledge<ValGo,DataGo,StateGo,EvalNode,MoveRecorderGo>(cfg_input.number_fill_board_attemps,cfg_input.long_game_coeff);
#else
        _sim_and_retro[i]=new SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNode,MoveRecorderGo>();
#endif
        _m[i]=new Mcts<ValGo,DataGo,Nod,StateGo>(&_sel,&_exp,_sim_and_retro[i],_sim_and_retro[i],&_sel_res,&_mutex);
#else
        _m[i]=new Mcts<ValGo,DataGo,Nod,StateGo>(&_sel,&_exp,&_sim,&_ret,&_sel_res,&_mutex);
#endif
    }
    _patterns=NULL;
    if(cfg_input.pattern_file){
        _patterns= new PatternList();
        _patterns->read_file(cfg_input.pattern_file);
    }
#ifdef RAVE
    NodeUCTRave<ValGo,DataGo>::k_rave = cfg_input.amaf_coeff;
#endif
    _state = new StateGo(_size,_komi,_patterns);
    _root= new Nod(0,{CHANGE_PLAYER(_state->turn),0,0});
}

Game::~Game(){
    delete _state;
    _root->delete_tree();
    for(int i=0;i<_cfg.num_threads_mcts;i++){
        delete _m[i];
        delete _sim_and_retro[i];
    }
    delete[] _m;
    delete[] _sim_and_retro;
    if(_patterns)
        delete _patterns;
}

void Game::set_boardsize(int size){
    if(size!= _size){
        delete _state;
        _root->delete_tree();
        _size = size;
        _state = new StateGo(_size,_komi,_patterns);
        _root= new Nod(0,{CHANGE_PLAYER(_state->turn),0,0});
    }
}

void Game::clear_board(){
    delete _state;
    _root->delete_tree();
    _state = new StateGo(_size,_komi,_patterns);
    _root= new Nod(0,{CHANGE_PLAYER(_state->turn),0,0});
}

void Game::set_komi(float komi){
    if(komi!= _komi){
        delete _state;
        _root->delete_tree();
        _komi = komi;
        _state = new StateGo(_size,_komi,_patterns);
        _root= new Nod(0,{CHANGE_PLAYER(_state->turn),0,0});
    }
}

float Game::get_final_score(){
    return _state->get_final_score();
}

bool Game::play_move(DataGo pos){
    if(IS_RESIGN(pos))
        return true;
    if(!_state->valid_move(pos))
        return false;
    _state->apply(pos);
    Nod *next;
    if(next=_root->move_root_to_child(pos)){
        delete _root;
        _root=next;
#ifdef DEBUG
        std::cerr << "STATS PLAYED NODE: vis=" << _root->visits << " win=" << _root->value
                  << " vis_amaf=" << _root->amaf_visits << " win_amaf=" << _root->amaf_value << std::endl;
        std::cerr << "                   rate=" << (_root->value / _root->visits) 
                  << "  amaf_rate=" << (_root->amaf_value / _root->amaf_visits)
                  << "  amaf_coeff="<< (sqrt(NodeUCTRave<ValGo,DataGo>::k_rave)/_root->sqrt_for_amaf) << std::endl;
#endif
    }else{
        _root->delete_tree();
        _root= new Nod(0,pos);
    }
    return true;
}

DataGo Game::gen_move(Player p){
    assert(p==_state->turn);
    std::thread *threads= new std::thread[_cfg.num_threads_mcts];
    for(int i=0; i<_cfg.num_threads_mcts; i++)
        threads[i] = std::thread(&Mcts<ValGo,DataGo,Nod,StateGo>::run_cycles,_m[i],_cfg.number_cycles_mcts/_cfg.num_threads_mcts,_root,_state);
    for(int i=0;i<_cfg.num_threads_mcts;i++)
        threads[i].join();
    delete[] threads;
    DataGo pos = _m[0]->get_resultant_move(_root);
#ifdef DEBUG
    _root->show();
    std::cerr << "Resultado: " << " i=" << (int)pos.i << " j=" << (int)pos.j
              << " vis=" << _root->visits << " win=" << _root->value
              << " vis_amaf=" << _root->amaf_visits << " win_amaf=" << _root->amaf_value << std::endl;
#endif
    return pos;
}

void Game::show_board(FILE *output){
    _state->show(output);
}

#ifdef DEBUG
void Game::debug(){
    std::cerr<<"VALUE: "<<_state->get_final_value()<<std::endl;
    std::cerr<<"BLOCKS:"<<std::endl;
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(_state->Blocks[i][j]==NULL)
              std::cerr<<std::setw(5)<<"----";
            else
              std::cerr<<std::setw(5)<<(((long) _state->Blocks[i][j])%10000);
        std::cerr<<std::endl;
    }
    std::cerr<<std::endl;
    std::cerr<<"BLOCKS'S VALUES:"<<std::endl;
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(_state->Blocks[i][j] == NULL)
              std::cerr<<std::setw(3)<<"--";
            else
              std::cerr<<std::setw(3)<<(*_state->Blocks[i][j]);
        std::cerr<<std::endl;
    }
}

void Game::match_patterns(){
    std::vector<DataGo> v;
    std::cerr<<"ESCAPE ATARI: "<<std::endl;
    _state->get_atari_escape_moves(v);
    for(int i=0;i<v.size();i++)
        std::cout<<"Position: "<<(int)v[i].i<<" "<<(int)v[i].j<<std::endl;
    std::cerr<<"PATTERNS: "<<std::endl;
    v.clear();
    _state->get_pattern_moves(v);
    for(int i=0;i<v.size();i++)
        std::cout<<"Position: "<<(int)v[i].i<<" "<<(int)v[i].j<<std::endl;
    v.clear();
    std::cerr<<"CAPTURES: "<<std::endl;
    _state->get_capture_moves(v);
    for(int i=0;i<v.size();i++)
        std::cout<<"Position: "<<(int)v[i].i<<" "<<(int)v[i].j<<std::endl;
    std::cerr<<"SIMULATION POSSIBLE MOVES: "<<std::endl;
    v.clear();
    _state->get_simulation_possible_moves(v);
    for(int i=0;i<v.size();i++)
        std::cout<<"Position: "<<(int)v[i].i<<" "<<(int)v[i].j<<std::endl;
}
#endif
