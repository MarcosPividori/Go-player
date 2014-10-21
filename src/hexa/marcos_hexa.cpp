
#include "mcts_utils.hpp"
#include "defines.h"
#include "state_hexa.hpp"
#ifdef RAVE
 #include "moverecorder_hexa.hpp"
#else
 #include "mcts_uct.hpp"
#endif
#include <iostream>
#include <cassert>
#include <thread>

#define NUM_THREADS 5
#define NUM_CYCLES 30000

struct EvalNode{
    ValHexa operator()(ValHexa v_nodo,ValHexa v_final,DataHexa dat_nodo)
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

DataHexa insert_mov(Player player,StateHexa *state)
{
    int i,j;
    while(1){
        std::cout<<"Insert mov (row and column): ";
        if((std::cin>>i>>j) && (i>=1 && i<12) && (j>=1 && j<12) && state->valid_move(MOVE(i-1,j-1,player)))
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
    StateHexa state;
    DataHexa res;
    std::vector<DataHexa> v;
    
    std::mutex mutex;    
    ExpansionAllChildren<ValHexa,DataHexa,StateHexa,Nod> exp(2,0);
    SelectResMostRobust<ValHexa,DataHexa,Nod> sel_res;
#ifdef RAVE
    SelectionUCTRave<ValHexa,DataHexa> sel(1,100);
    Mcts<ValHexa,DataHexa,Nod,StateHexa> *m[NUM_THREADS];
    SimulationAndRetropropagationRave<ValHexa,DataHexa,StateHexa,EvalNode,MoveRecorderHexa> sim_and_retro[NUM_THREADS];
    for(int i=0;i<NUM_THREADS;i++)
      m[i]= new Mcts<ValHexa,DataHexa,Nod,StateHexa>(&sel,&exp,&sim_and_retro[i],&sim_and_retro[i],&sel_res,&mutex);
#else
    SelectionUCT<ValHexa,DataHexa> sel(1);
    SimulationTotallyRandom<ValHexa,DataHexa,StateHexa> sim;
    RetropropagationSimple<ValHexa,DataHexa,EvalNode> ret;
    Mcts<ValHexa,DataHexa,Nod,StateHexa> m(&sel,&exp,&sim,&ret,&sel_res,&mutex);
#endif
    
    std::cout<< "HEXA:"<<std::endl;
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
#ifdef RAVE
                threads[i] = std::thread(&Mcts<ValHexa,DataHexa,Nod,StateHexa>::run_cycles,m[i],NUM_CYCLES/NUM_THREADS,nod,&state);
#else
                threads[i] = std::thread(&Mcts<ValHexa,DataHexa,Nod,StateHexa>::run_cycles,m,NUM_CYCLES/NUM_THREADS,nod,&state);
#endif
            for(int i=0; i<NUM_THREADS; i++)
                threads[i].join();
#ifdef DEBUG
            std::cout<<"Expansion counter: "<<exp.counter<<std::endl;
            nod->show();
#endif
#ifdef RAVE
            res = m[0]->get_resultant_move(nod);
#else
            res = m.get_resultant_move(nod);
#endif
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
    std::cout << "RESULT: " << (state.get_final_value()==1  ? "++ wins." :
                               (state.get_final_value()==-1 ? "oo wins." : "Tie.")) <<std::endl<<std::endl;

#ifdef RAVE
    for(int i=0;i<NUM_THREADS;i++){
      delete m[i];
    }
#endif
    return 0;
}

