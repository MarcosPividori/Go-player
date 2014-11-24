
#include "mcts_parallel.hpp"
#include "mcts_utils.hpp"
#include "mcts_uct.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include "state_tateti.hpp"

struct EvalNod: public EvalNode<ValTateti,DataTateti> {
    ValTateti operator()(ValTateti v_nodo,ValTateti v_final,DataTateti dat_nodo)
    {
        if(v_final == EMPTY)//Tie
            return v_nodo+0.9999;
        if(v_final == PlayerToCell(Player(dat_nodo)))
            return v_nodo+1;
        return v_nodo;
    }
};

Player insert_player()
{
    char c;
    while(1){
        std::cout<< "First(f) or Second(s) player?: ";
        if((std::cin>>c) && (c=='f' || c=='s'))
            break;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
    return (c=='f') ? Cross : Circle;
}

DataTateti insert_mov(Player player,StateTateti *state)
{
    int i,j;
    while(1){
        std::cout<<"Insert mov (row and column): ";
        if((std::cin>>i>>j) && (i>=1 && i<4) && (j>=1 && j<4) && state->valid_move(MOVE(i-1,j-1,player)))
            break;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
    std::cout<<std::endl;
    return MOVE(i-1,j-1,player);
}

int main()
{
    StateTateti state;
    DataTateti res;
    std::vector<DataTateti> v;
    SelectionUCT<ValTateti,DataTateti> sel(1);
    ExpansionAllChildren<ValTateti,DataTateti,StateTateti,Nod> exp(2,0);
    SimulationTotallyRandom<ValTateti,DataTateti,StateTateti> sim;
    RetropropagationSimple<ValTateti,DataTateti,EvalNod> ret;
    SelectResMostRobust<ValTateti,DataTateti,Nod> sel_res;
    Mcts<ValTateti,DataTateti,Nod,StateTateti> m(&sel,&exp,&sim,&ret,&sel_res);
    std::vector<Mcts<ValTateti,DataTateti,Nod,StateTateti> *> m_vector(NUM_THREADS,&m);
    MctsParallel_GlobalMutex<ValTateti,DataTateti,Nod,StateTateti> mcts(m_vector,&state,0);
    
    std::cout<< "TATETI:"<<std::endl;
    Player us_player=insert_player();
    
    std::cout<<"------------------"<<std::endl<<std::endl;
    state.show();

    while((v.clear(),state.get_possible_moves(v),!v.empty())){
        if(state.turn== us_player){
            std::cout<<std::endl;
            std::cout<<"-You play---------"<<std::endl<<std::endl;
            res=insert_mov(us_player,&state);
        }
        else{
            std::cout<<std::endl;
            std::cout<<"-Computer plays---"<<std::endl<<std::endl;
            mcts.run_cycles(NUM_CYCLES);
            res=mcts.get_resultant_move();
        }
        
        mcts.apply_move(res);
        
        state.show();    
    }

    std::cout<<std::endl<<"------------------"<<std::endl<<std::endl;
    std::cout << "RESULT: " << (state.get_final_value()==1  ? "X wins." :
                               (state.get_final_value()==-1 ? "O wins." : "Tie.")) <<std::endl<<std::endl;
    return 0;
}

