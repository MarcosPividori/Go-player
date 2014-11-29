
#include "game.hpp"
#include <iomanip>
#include <thread>
#include <mutex>

RaveEnv::RaveEnv(Config &cfg_input,StateGo *state)
{
    _exp= new ExpansionAllChildren<ValGo,DataGo,StateGo,Nod>(cfg_input.limit_expansion,0);
    _sel_res= new SelectResMostRobustOverLimit<Nod>(cfg_input.resign_limit);
    _sim_and_retro= new SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo>*[cfg_input.num_threads_mcts];
    _sel= new SelectionUCTRave<ValGo,DataGo>(cfg_input.bandit_coeff,cfg_input.amaf_coeff);
    for(int i=0;i<cfg_input.num_threads_mcts;i++){
        if(cfg_input.knowledge)
            _sim_and_retro[i]= new SimulationWithDomainKnowledge(
                                           cfg_input.number_fill_board_attemps,cfg_input.long_game_coeff,cfg_input.limit_atari);
        else
            _sim_and_retro[i]= new SimulationAndRetropropagationRaveGo(cfg_input.long_game_coeff);
        _m.push_back(new Mcts<ValGo,DataGo,Nod,StateGo>(_sel,_exp,_sim_and_retro[i],_sim_and_retro[i],_sel_res));
    }
    NodeUCTRave<ValGo,DataGo>::k_rave = cfg_input.amaf_coeff;
    if(cfg_input.root_parallel)
        _mcts= new MctsParallel_Root<ValGo,DataGo,Nod,StateGo>(_m,state,INIT_DATA(CHANGE_PLAYER(state->turn)));
    else
        _mcts= new MctsParallel_GlobalMutex<ValGo,DataGo,Nod,StateGo>(_m,state,INIT_DATA(CHANGE_PLAYER(state->turn)));
}

RaveEnv::~RaveEnv()
{
    for(int i=0;i<_m.size();i++){
        delete _m[i];
        delete _sim_and_retro[i];
    }
    delete[] _sim_and_retro;
    delete _exp;
    delete _sel_res;
    delete _sel;
    delete _mcts;
}

UCTEnv::UCTEnv(Config &cfg_input,StateGo *state)
{
    _exp= new ExpansionAllChildren<ValGo,DataGo,StateGo,Nod>(cfg_input.limit_expansion,0);
    _sel_res= new SelectResMostRobustOverLimit<Nod>(cfg_input.resign_limit);
    _sel= new SelectionUCT<ValGo,DataGo>(cfg_input.bandit_coeff);
    _sim= new Simulation<ValGo,StateGo>*[cfg_input.num_threads_mcts];
    _ret= new RetropropagationSimple<ValGo,DataGo,EvalNod>();
    for(int i=0;i<cfg_input.num_threads_mcts;i++){
        if(cfg_input.knowledge)
            _sim[i]= new SimulationWithDomainKnowledge(cfg_input.number_fill_board_attemps,cfg_input.long_game_coeff,cfg_input.limit_atari);
        else
            _sim[i]= new SimulationTotallyRandomGo(cfg_input.long_game_coeff);
        _m.push_back(new Mcts<ValGo,DataGo,Nod,StateGo>(_sel,_exp,_sim[i],_ret,_sel_res));
    }
    if(cfg_input.root_parallel)
        _mcts= new MctsParallel_Root<ValGo,DataGo,Nod,StateGo>(_m,state,INIT_DATA(CHANGE_PLAYER(state->turn)));
    else
        _mcts= new MctsParallel_GlobalMutex<ValGo,DataGo,Nod,StateGo>(_m,state,INIT_DATA(CHANGE_PLAYER(state->turn)));
}

UCTEnv::~UCTEnv()
{
    for(int i=0;i<_m.size();i++){
        delete _m[i];
        delete _sim[i];
    }
    delete[] _sim;
    delete _ret;
    delete _exp;
    delete _sel_res;
    delete _sel;
    delete _mcts;
}

Game::Game(int size,Config &cfg_input) : _komi(0),_size(size),_cfg(cfg_input),_patterns(NULL)
{
    if(cfg_input.knowledge && cfg_input.pattern_file){
        _patterns= new PatternList();
        _patterns->read_file(cfg_input.pattern_file);
    }

    _state= new StateGo(_size,_komi,_patterns);
   
    if(cfg_input.rave)
        _mcts = new RaveEnv(cfg_input,_state);
    else
        _mcts = new UCTEnv(cfg_input,_state);

    StateGo::japanese_rules=_cfg.japanese_rules;
    StateGo::pattern_coeff=_cfg.pattern_coeff;
    StateGo::capture_coeff=_cfg.capture_coeff;
    StateGo::atari_delete_coeff=_cfg.atari_delete_coeff;
    StateGo::atari_escape_coeff=_cfg.atari_escape_coeff;
}

Game::~Game()
{
    delete _state;
    delete _mcts;
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
    if(_cfg.rave)
        ((NodeUCTRave<ValGo,DataGo>*) (_mcts->get_root()))->debug();
    else
        ((NodeUCT<ValGo,DataGo>*) (_mcts->get_root()))->debug();
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
    _state->debug();
    long visits[MAX_BOARD][MAX_BOARD];
    double coeffs[MAX_BOARD][MAX_BOARD];
    for(int i = _size-1;i>=0;i--)
        for(int j=0;j<_size;j++)
            visits[i][j]=0;
    if(_cfg.rave)
    {
        NodeUCTRave<ValGo,DataGo> *root = (NodeUCTRave<ValGo,DataGo>*) _mcts->get_root();
        if(!root) return;
        std::cout<<"CELL MCTS VISITS:"<<std::endl;
        double sqrt_log_parent = sqrt(log((double) root->visits));
        for(int i=0;i<root->children.size();i++)
            if(!IS_PASS(root->children[i]->data)){
              visits[root->children[i]->data.i][root->children[i]->data.j]=root->children[i]->visits;
              coeffs[root->children[i]->data.i][root->children[i]->data.j]=SelectionUCTRave<ValGo,DataGo>(_cfg.bandit_coeff,_cfg.amaf_coeff).get_uct_amaf_val(root->children[i],sqrt_log_parent);
            }
    }
    else
    {
        NodeUCT<ValGo,DataGo> *root = (NodeUCT<ValGo,DataGo>*) _mcts->get_root();
        if(!root) return;
        std::cout<<"CELL MCTS VISITS:"<<std::endl;
        double sqrt_log_parent = sqrt(log((double) root->visits));
        for(int i=0;i<root->children.size();i++)
            if(!IS_PASS(root->children[i]->data)){
              visits[root->children[i]->data.i][root->children[i]->data.j]=root->children[i]->visits;
              coeffs[root->children[i]->data.i][root->children[i]->data.j]=SelectionUCT<ValGo,DataGo>(_cfg.bandit_coeff).get_uct_val(root->children[i],sqrt_log_parent);
            }
    }
    for(int i=_size-1;i>=0;i--){
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
        std::cout<<"Position: "<<POS_TO_LETTER(v[i].j)<<" "<<int(1+v[i].i)<<std::endl;
    std::cout<<"PATTERNS: "<<std::endl;
    v.clear();
    _state->get_pattern_moves(v);
    for(int i=0;i<v.size();i++)
        std::cout<<"Position: "<<POS_TO_LETTER(v[i].j)<<" "<<int(1+v[i].i)<<std::endl;
    v.clear();
    std::cout<<"CAPTURES: "<<std::endl;
    _state->get_capture_moves(v);
    for(int i=0;i<v.size();i++)
        std::cout<<"Position: "<<POS_TO_LETTER(v[i].j)<<" "<<int(1+v[i].i)<<std::endl;
}
#endif
