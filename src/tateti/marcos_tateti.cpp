
#include "mcts_utils.hpp"
#include "mcts_uct.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include "state_tateti.hpp"

struct EvalNode{
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
    Nod *nod=new Nod(0,0),*next;
    StateTateti state;
    DataTateti res;
    std::vector<DataTateti> v;
    std::mutex mutex;
    SelectionUCT<ValTateti,DataTateti> sel(1);
    ExpansionAllChildren<ValTateti,DataTateti,StateTateti,Nod> exp(2,0);
    SimulationTotallyRandom<ValTateti,DataTateti,StateTateti> sim;
    RetropropagationSimple<ValTateti,DataTateti,EvalNode> ret;
    SelectResMostRobust<ValTateti,DataTateti,Nod> sel_res;
    Mcts<ValTateti,DataTateti,Nod,StateTateti> m(&sel,&exp,&sim,&ret,&sel_res,&mutex);
    
    std::cout<< "TATETI:"<<std::endl;
    Player us_player=insert_player();
    
    std::cout<<"------------------"<<std::endl<<std::endl;
    state.show();

    while((v.clear(),state.get_possible_moves(v),!v.empty())){
        if(state.turn== us_player){
            std::cout<<std::endl;
            std::cout<<"-You play---------"<<std::endl<<std::endl;
            res=insert_mov(us_player,&state);
#ifdef DEBUG
            nod->show();
#endif
        }
        else{
            std::cout<<std::endl;
            std::cout<<"-Computer plays---"<<std::endl<<std::endl;
#ifdef DEBUG
            nod->show();
            exp.counter=0;
#endif

            std::thread threads[NUM_THREADS];
            for(int i=0; i<NUM_THREADS; i++)
                threads[i] = std::thread(&Mcts<ValTateti,DataTateti,Nod,StateTateti>::run_cycles,m,NUM_CYCLES/NUM_THREADS,nod,&state);
            for(int i=0; i<NUM_THREADS; i++)
                threads[i].join();

#ifdef DEBUG
            std::cout<<"Expansion counter: "<<exp.counter<<std::endl;
            nod->show();
#endif
            res = m.get_resultant_move(nod);
#ifdef DEBUG
            std::cout << "Resultado: " << res << " i=" << res/6 << " j=" << (res%6)/2
                      << " vis=" << nod->visits << " win=" << nod->value << std::endl;
#endif
        }
        state.apply(res);
        
        if((next=nod->move_root_to_child(res))){
            delete nod;
            nod=next;
        }else{
            nod->delete_tree();
            nod= new Nod(0,res);
        }

        state.show();    
    }

    std::cout<<std::endl<<"------------------"<<std::endl<<std::endl;
    std::cout << "RESULT: " << (state.get_final_value()==1  ? "X wins." :
                               (state.get_final_value()==-1 ? "O wins." : "Tie.")) <<std::endl<<std::endl;
    return 0;
}

