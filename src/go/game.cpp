
#include "game.hpp"
#include <iomanip>
#include <thread>
#include <mutex>

Game::Game(int size,Config &cfg_input) : _komi(0),_size(size),_exp(cfg_input.limit_expansion,0),_cfg(cfg_input),_sel_res(cfg_input.resign_limit),
#ifdef RAVE
 #ifndef NEXP
_sel(cfg_input.bandit_coeff,cfg_input.amaf_coeff)
 #else
_sel()
 #endif
#else
_sel(cfg_input.bandit_coeff)
#endif

{

#ifdef RAVE
  #ifdef NEXP
    _sim_and_retro= new SimulationAndRetropropagationRaveNExp<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>*[_cfg.num_threads_mcts];
  #else
    _sim_and_retro= new SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>*[_cfg.num_threads_mcts];
  #endif
    for(int i=0;i<_cfg.num_threads_mcts;i++){
    #ifdef KNOWLEDGE
      #ifdef NEXP
        _sim_and_retro[i]=new SimulationWithDomainKnowledgeNExp<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>(
                                           cfg_input.number_fill_board_attemps,cfg_input.long_game_coeff);
      #else
        _sim_and_retro[i]=new SimulationWithDomainKnowledge<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>(
                                           cfg_input.number_fill_board_attemps,cfg_input.long_game_coeff);
      #endif
    #else
      #ifdef NEXP
        _sim_and_retro[i]=new SimulationAndRetropropagationRaveNExp<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>();
      #else
        _sim_and_retro[i]=new SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>();
      #endif
    #endif
        _m.push_back(new Mcts<ValGo,DataGo,Nod,StateGo>(&_sel,&_exp,_sim_and_retro[i],_sim_and_retro[i],&_sel_res));
    }
  #ifdef NEXP
    NodeUCTRaveNExp<ValGo,DataGo>::k_rave = cfg_input.amaf_coeff;
  #else
    NodeUCTRave<ValGo,DataGo>::k_rave = cfg_input.amaf_coeff;
  #endif
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
    delete &_m;
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
        std::cerr << "STATS PLAYED NODE: vis=" << _mcts->get_root()->visits << " win=" << _mcts->get_root()->value
                  << " vis_amaf=" << _mcts->get_root()->amaf_visits << " win_amaf=" << _mcts->get_root()->amaf_value << std::endl;
        std::cerr << "                   rate=" << (_mcts->get_root()->value / _mcts->get_root()->visits) 
                  << "  amaf_rate=" << (_mcts->get_root()->amaf_value / _mcts->get_root()->amaf_visits)
                  << "  amaf_coeff="<< (sqrt(NodeUCTRave<ValGo,DataGo>::k_rave)/_mcts->get_root()->sqrt_for_amaf) << std::endl;
#endif

    return true;
}

DataGo Game::gen_move(Player p){
    assert(p==_state->turn);

    _mcts->run_cycles(_cfg.number_cycles_mcts);
    DataGo pos = _mcts->get_resultant_move();

#ifdef DEBUG
    _mcts->get_root()->show();
    std::cerr << "Resultado: " << " i=" << (int)pos.i << " j=" << (int)pos.j
              << " vis=" << _mcts->get_root()->visits << " win=" << _mcts->get_root()->value
              << " vis_amaf=" << _mcts->get_root()->amaf_visits << " win_amaf=" << _mcts->get_root()->amaf_value << std::endl;
    debug();
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
              std::cerr<<std::setw(3)<<(_state->Blocks[i][j]->adj);
        std::cerr<<std::endl;
    }
    std::cerr<<std::endl;
    std::cerr<<"CELL MCTS VISITS:"<<std::endl;
    long visits[MAX_BOARD][MAX_BOARD];
    double coeffs[MAX_BOARD][MAX_BOARD];
    double sqrt_log_parent = sqrt(log((double) _mcts->get_root()->visits));
    for(int i = _size-1;i>=0;i--)
        for(int j=0;j<_size;j++)
            visits[i][j]=0;
    Nod *root_now=_mcts->get_root();
    for(auto it=root_now->children.begin();it!=root_now->children.end();++it)
        if(!IS_PASS((*it)->data)){
          visits[(*it)->data.i][(*it)->data.j]=(*it)->visits;
          #ifndef NEXP
          coeffs[(*it)->data.i][(*it)->data.j]=_sel.get_uct_amaf_val((*it),sqrt_log_parent);
          #else
          coeffs[(*it)->data.i][(*it)->data.j]=(*it)->rate;
          #endif
        }
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(visits[i][j] == 0){
              std::cerr<<std::setw(4)<<"---";
              switch(_state->Stones[i][j]){
                case Black: std::cerr<<"X";break;
                case White: std::cerr<<"O";break;
                default: std::cerr<<"-";break;
              }
              std::cerr<<"---";
            }
            else
              std::cerr<<std::setw(8)<<visits[i][j];
        std::cerr<<std::endl;
        for(int j=0;j<_size;j++)
            if(visits[i][j] == 0)
              std::cerr<<std::setw(8)<<" ";
            else
              std::cerr<<std::setw(8)<<std::setprecision(5)<<coeffs[i][j];
        std::cerr<<std::endl;
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
}
#endif
