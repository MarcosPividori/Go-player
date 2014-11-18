
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
#ifdef RAVE
    _sim_and_retro= new SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>*[_cfg.num_threads_mcts];
    for(int i=0;i<_cfg.num_threads_mcts;i++){
    #ifdef KNOWLEDGE
        _sim_and_retro[i]=new SimulationWithDomainKnowledge<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>(
                                           cfg_input.number_fill_board_attemps,cfg_input.long_game_coeff);
    #else
        _sim_and_retro[i]=new SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>();
    #endif
        _m.push_back(new Mcts<ValGo,DataGo,Nod,StateGo>(&_sel,&_exp,_sim_and_retro[i],_sim_and_retro[i],&_sel_res));
    }
    NodeUCTRave<ValGo,DataGo>::k_rave = cfg_input.amaf_coeff;
#else
    for(int i=0;i<_cfg.num_threads_mcts;i++)
        _m.push_back(new Mcts<ValGo,DataGo,Nod,StateGo>(&_sel,&_exp,&_sim,&_ret,&_sel_res));
#endif

    _patterns=NULL;
    if(cfg_input.pattern_file){
        _patterns= new PatternList();
        _patterns->read_file(cfg_input.pattern_file);
    }
    
    _state = new StateGo(_size,_komi,_patterns);
   
    StateGo::japanese_rules=_cfg.japanese_rules;
 
    if(cfg_input.root_parallel)
        _mcts = new MctsParallel_Root<ValGo,DataGo,Nod,StateGo>(_m,_state,INIT_DATA(CHANGE_PLAYER(_state->turn)));
    else
        _mcts = new MctsParallel_GlobalMutex<ValGo,DataGo,Nod,StateGo>(_m,_state,INIT_DATA(CHANGE_PLAYER(_state->turn)));
}

Game::~Game(){
    for(int i=0;i<_m.size();i++){
        delete _m[i];
        delete _sim_and_retro[i];
    }
    delete _state;
    delete _mcts;
    delete[] _sim_and_retro;
    if(_patterns)
        delete _patterns;
}

void Game::set_boardsize(int size){
    if(size!= _size){
        delete _state;
        _size = size;
        _state = new StateGo(_size,_komi,_patterns);
        _mcts->reinit(_state,INIT_DATA(CHANGE_PLAYER(_state->turn)));
    }
}

void Game::clear_board(){
    delete _state;
    _state = new StateGo(_size,_komi,_patterns);
    _mcts->reinit(_state,INIT_DATA(CHANGE_PLAYER(_state->turn)));
}

void Game::set_komi(float komi){
    if(komi!= _komi){
        delete _state;
        _komi = komi;
        _state = new StateGo(_size,_komi,_patterns);
        _mcts->reinit(_state,INIT_DATA(CHANGE_PLAYER(_state->turn)));
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
    _mcts->apply_move(pos);

#ifdef DEBUG
        std::cout << "STATS PLAYED NODE: vis=" << _mcts->get_root()->visits << " win=" << _mcts->get_root()->value
                  << " vis_amaf=" << _mcts->get_root()->amaf_visits << " win_amaf=" << _mcts->get_root()->amaf_value << std::endl;
        std::cout << "                   rate=" << (_mcts->get_root()->value / _mcts->get_root()->visits) 
                  << "  amaf_rate=" << (_mcts->get_root()->amaf_value / _mcts->get_root()->amaf_visits)
                  << "  amaf_coeff="<< (sqrt(NodeUCTRave<ValGo,DataGo>::k_rave)/_mcts->get_root()->sqrt_for_amaf) << std::endl;
#endif

    return true;
}

DataGo Game::gen_move(Player p){
    assert(p==_state->turn);

    _mcts->run_cycles(_cfg.number_cycles_mcts);
    DataGo pos = _mcts->get_resultant_move();
/*
#ifdef DEBUG
    _mcts->get_root()->show();
    std::cout << "Resultado: " << " i=" << (int)pos.i << " j=" << (int)pos.j
              << " vis=" << _mcts->get_root()->visits << " win=" << _mcts->get_root()->value
              << " vis_amaf=" << _mcts->get_root()->amaf_visits << " win_amaf=" << _mcts->get_root()->amaf_value << std::endl;
    debug();
#endif
*/
    return pos;
}

void Game::show_board(FILE *output){
    _state->show(output);
}

#ifdef DEBUG
void Game::debug(){
    std::cout<<"VALUE: "<<_state->get_final_value()<<std::endl;
    std::cout<<"BLOCKS:"<<std::endl;
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(_state->Blocks[i][j]==NULL)
              std::cout<<std::setw(5)<<"----";
            else
              std::cout<<std::setw(5)<<(((long) _state->Blocks[i][j])%10000);
        std::cout<<std::endl;
    }
    std::cout<<"BLOCKS'S VALUES:"<<std::endl;
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(_state->Blocks[i][j] == NULL)
              std::cout<<std::setw(3)<<"--";
            else
              std::cout<<std::setw(3)<<(_state->Blocks[i][j]->adj);
        std::cout<<std::endl;
    }
    std::cout<<"CELL MCTS VISITS:"<<std::endl;
    long visits[MAX_BOARD][MAX_BOARD];
    double coeffs[MAX_BOARD][MAX_BOARD];
    double sqrt_log_parent = sqrt(log((double) _mcts->get_root()->visits));
    for(int i = _size-1;i>=0;i--)
        for(int j=0;j<_size;j++)
            visits[i][j]=0;
    for(int i=0;i<_mcts->get_root()->children.size();i++)
        if(!IS_PASS(_mcts->get_root()->children[i]->data)){
          visits[_mcts->get_root()->children[i]->data.i][_mcts->get_root()->children[i]->data.j]=_mcts->get_root()->children[i]->visits;
          coeffs[_mcts->get_root()->children[i]->data.i][_mcts->get_root()->children[i]->data.j]=_sel.get_uct_amaf_val(_mcts->get_root()->children[i],sqrt_log_parent);
        }
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(visits[i][j] == 0){
              std::cout<<std::setw(4)<<"---";
              switch(_state->Stones[i][j]){
                case Black: std::cout<<"X";break;
                case White: std::cout<<"O";break;
                default: std::cout<<"-";break;
              }
              std::cout<<"---";
            }
            else
              std::cout<<std::setw(8)<<visits[i][j];
        std::cout<<std::endl;
        for(int j=0;j<_size;j++)
            if(visits[i][j] == 0)
              std::cout<<std::setw(8)<<" ";
            else
              std::cout<<std::setw(8)<<std::setprecision(5)<<coeffs[i][j];
        std::cout<<std::endl;
        std::cout<<" "<<std::endl;
    }
}

void Game::match_patterns(){
    std::vector<DataGo> v;
    std::cout<<"ESCAPE ATARI: "<<std::endl;
    _state->get_atari_escape_moves(v);
    for(int i=0;i<v.size();i++)
        std::cout<<"Position: "<<(int)v[i].i<<" "<<(int)v[i].j<<std::endl;
    std::cout<<"PATTERNS: "<<std::endl;
    v.clear();
    _state->get_pattern_moves(v);
    for(int i=0;i<v.size();i++)
        std::cout<<"Position: "<<(int)v[i].i<<" "<<(int)v[i].j<<std::endl;
    v.clear();
    std::cout<<"CAPTURES: "<<std::endl;
    _state->get_capture_moves(v);
    for(int i=0;i<v.size();i++)
        std::cout<<"Position: "<<(int)v[i].i<<" "<<(int)v[i].j<<std::endl;
}
#endif
