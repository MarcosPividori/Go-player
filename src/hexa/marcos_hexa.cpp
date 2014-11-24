
#include "mcts_parallel.hpp"
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
#define NUM_CYCLES 100000

struct EvalNod : public EvalNode<ValHexa,DataHexa> {
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
    StateHexa state(7);
    DataHexa res;
    std::vector<DataHexa> v;
   
    ExpansionAllChildren<ValHexa,DataHexa,StateHexa,Nod> exp(2,0);
    SelectResMostRobust<ValHexa,DataHexa,Nod> sel_res;
#ifdef RAVE
    SelectionUCTRave<ValHexa,DataHexa> sel(0,5000);
    SimulationAndRetropropagationRave<ValHexa,DataHexa,StateHexa,EvalNod,MoveRecorderHexa> sim_and_retro[NUM_THREADS];
    std::vector<Mcts<ValHexa,DataHexa,Nod,StateHexa> *> m_vector;
    for(int i=0;i<NUM_THREADS;i++)
      m_vector.push_back(Mcts<ValHexa,DataHexa,Nod,StateHexa>(&sel,&exp,&sim_and_retro[i],&sim_and_retro[i],&sel_res,&mutex));
#else
    SelectionUCT<ValHexa,DataHexa> sel(1);
    SimulationTotallyRandom<ValHexa,DataHexa,StateHexa> sim;
    RetropropagationSimple<ValHexa,DataHexa,EvalNod> ret;
    Mcts<ValHexa,DataHexa,Nod,StateHexa> m(&sel,&exp,&sim,&ret,&sel_res);
    std::vector<Mcts<ValHexa,DataHexa,Nod,StateHexa> *> m_vector(NUM_THREADS,&m);
#endif
    MctsParallel_GlobalMutex<ValHexa,DataHexa,Nod,StateHexa> mcts(m_vector,&state,0);

    std::cout<< "HEXA:"<<std::endl;
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
    std::cout << "RESULT: " << (state.get_final_value()==1  ? "++ wins." :
                               (state.get_final_value()==-1 ? "oo wins." : "Tie.")) <<std::endl<<std::endl;

#ifdef RAVE
    for(int i=0;i<NUM_THREADS;i++)
      delete m_vector[i];
#endif

    return 0;
}

