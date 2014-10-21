
#include "mcts_utils.hpp"
#include "mcts_uct.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include "state_connect4.hpp"

struct EvalNode{
    ValConnect4 operator()(ValConnect4 v_nodo,ValConnect4 v_final,DataConnect4 dat_nodo)
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

DataConnect4 insert_mov(Player player,StateConnect4 *state)
{
    int j;
    while(1){
        std::cout<<"Insert mov (Column num 0-6): ";
        if((std::cin>>j) && (j>=0 && j<7) && state->valid_move(MOVE(j,player)))
            break;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
    std::cout<<std::endl;
    return MOVE(j,player);
}

int main()
{
    Nod *nod=new Nod(0,MOVE(Circle,0)),*next;
    StateConnect4 state;
    DataConnect4 res;
    std::vector<DataConnect4> v;
    
    std::mutex mutex;
    SelectionUCT<ValConnect4,DataConnect4> sel(1);
    ExpansionAllChildren<ValConnect4,DataConnect4,StateConnect4,Nod> exp(2,0);
    SimulationTotallyRandom<ValConnect4,DataConnect4,StateConnect4> sim;
    RetropropagationSimple<ValConnect4,DataConnect4,EvalNode> ret;
    SelectResMostRobust<ValConnect4,DataConnect4,Nod> sel_res;
    Mcts<ValConnect4,DataConnect4,Nod,StateConnect4> m(&sel,&exp,&sim,&ret,&sel_res,&mutex);
    
    std::cout<< "CONNNECT4:"<<std::endl;
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
                threads[i] = std::thread(&Mcts<ValConnect4,DataConnect4,Nod,StateConnect4>::run_cycles,m,NUM_CYCLES/NUM_THREADS,nod,&state);
            for(std::thread& th : threads)
                th.join();
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

