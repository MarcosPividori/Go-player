
#include "mcts_parallel.hpp"
#include "mcts_utils.hpp"
#include "mcts_uct.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include "state_connect4.hpp"

using namespace std;

struct EvalNod : public EvalNode<ValConnect4,DataConnect4> {
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
        cout<< "First(f) or Second(s) player?: ";
        if((cin>>c) && (c=='f' || c=='s'))
            break;
        cin.clear();
        cin.ignore(10000, '\n');
    }
    return (c=='f') ? Cross : Circle;
}

DataConnect4 insert_mov(Player player,StateConnect4 *state)
{
    int j;
    while(1){
        cout<<"Insert mov (Column num 0-6): ";
        if((cin>>j) && (j>=0 && j<7) && state->valid_move(MOVE(j,player)))
            break;
        cin.clear();
        cin.ignore(10000, '\n');
    }
    cout<<endl;
    return MOVE(j,player);
}

int main()
{
    StateConnect4 state;
    DataConnect4 res;
    vector<DataConnect4> v;
    
    SelectionUCT<ValConnect4,DataConnect4> sel(1);
    ExpansionAllChildren<ValConnect4,DataConnect4,StateConnect4,Nod> exp(2,0);
    SimulationTotallyRandom<ValConnect4,DataConnect4,StateConnect4> sim;
    RetropropagationSimple<ValConnect4,DataConnect4,EvalNod> ret;
    SelectResMostRobust<ValConnect4,DataConnect4,Nod> sel_res;
    Mcts<ValConnect4,DataConnect4,Nod,StateConnect4> m(&sel,&exp,&sim,&ret,&sel_res);
    vector<Mcts<ValConnect4,DataConnect4,Nod,StateConnect4> *> m_vector(NUM_THREADS,&m);
    MctsParallel_GlobalMutex<ValConnect4,DataConnect4,Nod,StateConnect4> mcts(m_vector,&state,MOVE(Circle,0));
    
    cout<< "CONNNECT4:"<<endl;
    Player us_player=insert_player();
    
    cout<<"------------------"<<endl<<endl;
    state.show();

    while((v.clear(),state.get_possible_moves(v),!v.empty())){
        if(state.turn== us_player){
            cout<<endl;
            cout<<"-You play---------"<<endl<<endl;
            res=insert_mov(us_player,&state);
        }
        else{
            cout<<endl;
            cout<<"-Computer plays---"<<endl<<endl;
            mcts.run_time(MAX_SECONDS);
            //mcts.run_cycles(NUM_CYCLES);
            res=mcts.get_resultant_move();
        }

        mcts.apply_move(res);

        state.show();
    }

    cout<<endl<<"------------------"<<endl<<endl;
    cout << "RESULT: " << (state.get_final_value()==1  ? "X wins." :
                               (state.get_final_value()==-1 ? "O wins." : "Tie.")) <<endl<<endl;
    return 0;
}

