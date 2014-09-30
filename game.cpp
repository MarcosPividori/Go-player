
#include "game.hpp"
#include <iomanip>
#include <thread>

Game::Game(int size) : _komi(0),_size(size),_sel(1),_exp(2,0),_ret(EvalNode()),_m(&_sel,&_exp,&_sim,&_ret,&_sel_res){
    _state = new StateGo(_size,_komi);
    _root= new NodeUCT<ValGo,DataGo>(0,{CHANGE_PLAYER(_state->turn),0,0},NULL);
}

Game::~Game(){
    delete _state;
    _root->delete_tree();
}

void Game::set_boardsize(int size){
    if(size!= _size){
        delete _state;
        _root->delete_tree();
        _size = size;
        _state = new StateGo(_size,_komi);
        _root= new NodeUCT<ValGo,DataGo>(0,{CHANGE_PLAYER(_state->turn),0,0},NULL);
    }
}

void Game::clear_board(){
    delete _state;
    _root->delete_tree();
    _state = new StateGo(_size,_komi);
    _root= new NodeUCT<ValGo,DataGo>(0,{CHANGE_PLAYER(_state->turn),0,0},NULL);
}

void Game::set_komi(float komi){
    if(komi!= _komi){
        delete _state;
        _root->delete_tree();
        _komi = komi;
        _state = new StateGo(_size,_komi);
        _root= new NodeUCT<ValGo,DataGo>(0,{CHANGE_PLAYER(_state->turn),0,0},NULL);
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
    NodeUCT<ValGo,DataGo> *next;
    if(next=_root->move_root_to_child(pos)){
        delete _root;
        _root=next;
    }else{
        _root->delete_tree();
        _root= new NodeUCT<ValGo,DataGo>(0,pos,NULL);
    }
    return true;
}

DataGo Game::gen_move(Player p){
    assert(p==_state->turn);
    std::thread threads[5];
    for(int i=0; i<5; i++)
        threads[i] = std::thread(&Mcts<ValGo,DataGo,NodeUCT<ValGo,DataGo>,StateGo>::run_time,&_m,5,_root,_state);
    for(std::thread& th : threads) th.join();
    DataGo pos = _m.get_resultant_move(_root);
#ifdef DEBUG
    _root->show();
    std::cerr << "Resultado: " << " i=" << (int)pos.i << " j=" << (int)pos.j
              << " vis=" << _root->visits << " win=" << _root->value << std::endl;
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
#endif
