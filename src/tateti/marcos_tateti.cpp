
#include "mcts_parallel.hpp"
#include "mcts_utils.hpp"
#include "mcts_uct.hpp"
#include <iostream>
#include <cassert>
#include <thread>
#include "state_tateti.hpp"

using namespace std;

struct EvalNod: public EvalNode<ValTateti,DataTateti> {
    ValTateti operator()(ValTateti v_nodo,ValTateti v_final,DataTateti dat_nodo)
    {
        if(v_final == EMPTY)//Tie
            return v_nodo+0.9;
        if(v_final == PlayerToCell(dat_nodo.player))
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
    return (c=='f') ? CROSS_P : CIRCLE_P;
}

DataTateti insert_mov(Player player,StateTateti *state)
{
    int i,j;
    while(1){
        cout<<"Insert mov (row and column): ";
        if((cin>>i>>j) && (i>=1 && i<4) && (j>=1 && j<4) && state->valid_move(DataTateti(i-1,j-1,player)))
            break;
        cin.clear();
        cin.ignore(10000, '\n');
    }
    cout<<endl;
    return DataTateti(i-1,j-1,player);
}

int main()
{
    StateTateti state;
    DataTateti res;
    vector<DataTateti> v;
    SelectionUCT<ValTateti,DataTateti> sel(1);
    ExpansionAllChildren<ValTateti,DataTateti,StateTateti,Nod> exp(2,0);
    SimulationTotallyRandom<ValTateti,DataTateti,StateTateti> sim;
    RetropropagationSimple<ValTateti,DataTateti,EvalNod> ret;
    SelectResMostRobust<ValTateti,DataTateti,Nod> sel_res;
    Mcts<ValTateti,DataTateti,Nod,StateTateti> m(&sel,&exp,&sim,&ret,&sel_res);
    vector<Mcts<ValTateti,DataTateti,Nod,StateTateti> *> m_vector(NUM_THREADS,&m);
    MctsParallel_GlobalMutex<ValTateti,DataTateti,Nod,StateTateti> mcts(m_vector,&state,0);
    
    cout<< "TATETI:"<<endl;
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
    cout << "RESULT: " << (state.get_final_value()==CROSS  ? "X wins." :
                               (state.get_final_value()==CIRCLE ? "O wins." : "Tie.")) <<endl<<endl;
    return 0;
}

