
#include "mcts_parallel.hpp"
#include "mcts_utils.hpp"
#include "defines.h"
#include "state_hexa.hpp"
#ifdef RAVE
 #include "moverecorder_hexa.hpp"
#endif
#include "mcts_uct.hpp"
#include <iostream>
#include <iomanip>
#include <cassert>
#include <thread>

#define NUM_THREADS 5
#define NUM_CYCLES 100000
#define BANDIT_COEFF 0
#define K_RAVE 5000
#define SIZE 7

using namespace std;

struct EvalNod : public EvalNode<ValHexa,DataHexa> {
    ValHexa operator()(ValHexa v_nodo,ValHexa v_final,DataHexa dat_nodo)
    {
        if(v_final == EMPTY)//Tie
            return v_nodo+0.9999;
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

DataHexa insert_mov(Player player,StateHexa *state)
{
    int i,j;
    while(1){
        cout<<"Insert mov (row and column): ";
        if((cin>>i>>j) && (i>=1 && i<12) && (j>=1 && j<12) && state->valid_move(DataHexa(i-1,j-1,player)))
            break;
        cin.clear();
        cin.ignore(10000, '\n');
    }
    cout<<endl;
    return DataHexa(i-1,j-1,player);
}

void debug(MctsParallel_GlobalMutex<ValHexa,DataHexa,Nod,StateHexa> &mcts);

int main()
{
    StateHexa state(SIZE);
    DataHexa res;
    vector<DataHexa> v;
   
    ExpansionAllChildren<ValHexa,DataHexa,StateHexa,Nod> exp(2,0);
    SelectResMostRobust<ValHexa,DataHexa,Nod> sel_res;
#ifdef RAVE
    NodeUCTRave<ValHexa,DataHexa>::k_rave = K_RAVE;
    SelectionUCTRave<ValHexa,DataHexa> sel(BANDIT_COEFF,K_RAVE);
    SimulationAndRetropropagationRave<ValHexa,DataHexa,StateHexa,EvalNod,MoveRecorderHexa> sim_and_retro[NUM_THREADS];
    vector<Mcts<ValHexa,DataHexa,Nod,StateHexa> *> m_vector;
    for(int i=0;i<NUM_THREADS;i++)
      m_vector.push_back(new Mcts<ValHexa,DataHexa,Nod,StateHexa>(&sel,&exp,&sim_and_retro[i],&sim_and_retro[i],&sel_res));
#else
    SelectionUCT<ValHexa,DataHexa> sel(1);
    SimulationTotallyRandom<ValHexa,DataHexa,StateHexa> sim;
    RetropropagationSimple<ValHexa,DataHexa,EvalNod> ret;
    Mcts<ValHexa,DataHexa,Nod,StateHexa> m(&sel,&exp,&sim,&ret,&sel_res);
    vector<Mcts<ValHexa,DataHexa,Nod,StateHexa> *> m_vector(NUM_THREADS,&m);
#endif
    MctsParallel_GlobalMutex<ValHexa,DataHexa,Nod,StateHexa> mcts(m_vector,&state,0);

    cout<< "HEXA:"<<endl;
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
            mcts.run_cycles(NUM_CYCLES);
            res=mcts.get_resultant_move();
        }

#ifdef DEBUG
        debug(mcts);
#endif
        mcts.apply_move(res);
        
        state.show();    
    }

    cout<<endl<<"------------------"<<endl<<endl;
    cout << "RESULT: " << (state.get_final_value()==CROSS  ? "++ wins." :
                               (state.get_final_value()==CIRCLE ? "oo wins." : "Tie."))
                            << endl<<endl;

#ifdef RAVE
    for(int i=0;i<NUM_THREADS;i++)
      delete m_vector[i];
#endif

    return 0;
}

#ifdef DEBUG
void debug(MctsParallel_GlobalMutex<ValHexa,DataHexa,Nod,StateHexa> &mcts)
{
#ifdef RAVE
    NodeUCTRave<ValHexa,DataHexa> *root = (NodeUCTRave<ValHexa,DataHexa>*) mcts.get_root();
    NodeUCTRave<ValHexa,DataHexa>::const_iterator iter=root->children_begin();
#else
    NodeUCT<ValHexa,DataHexa> *root = (NodeUCT<ValHexa,DataHexa>*) mcts.get_root();
    NodeUCT<ValHexa,DataHexa>::const_iterator iter=root->children_begin();
#endif
    root->debug();
    long visits[SIZE][SIZE];
    double coeffs[SIZE][SIZE];
    for(int i=0;i<SIZE;i++)
        for(int j=0;j<SIZE;j++)
            visits[i][j]=0;
    if(!root)
        return;
    cout<<endl<<"CELL MCTS VISITS: (num of visits,move value)"<<endl<<endl;
    double sqrt_log_parent = sqrt(log((double) root->get_visits()));
    for(;iter != root->children_end();iter++)
    {
        visits[(*iter)->data.i][(*iter)->data.j]=(*iter)->get_visits();
#ifdef RAVE
        coeffs[(*iter)->data.i][(*iter)->data.j]=
                SelectionUCTRave<ValHexa,DataHexa>(BANDIT_COEFF,K_RAVE)
                .get_uct_amaf_val((*iter),sqrt_log_parent);
#else
        coeffs[(*iter)->data.i][(*iter)->data.j]=
                SelectionUCT<ValHexa,DataHexa>(BANDIT_COEFF)
                .get_uct_val((*iter),sqrt_log_parent);
#endif
    }
    cout<<setw(5)<<" ";
    for(int j=0;j<SIZE;j++)
        cout<<setw(8)<<j+1;
    cout<<endl<<endl;
    for(int i=0;i<SIZE;i++){
        cout<<setw(3)<<i+1<<"  ";
        for(int j=0;j<SIZE;j++)
            if(visits[i][j] == 0)
              cout<<setw(8)<<"-------";
            else
              cout<<setw(8)<<visits[i][j];
        cout<<endl<<setw(5)<<" ";
        for(int j=0;j<SIZE;j++)
            if(visits[i][j] == 0)
              cout<<setw(8)<<" ";
            else
              cout<<setw(8)<<setprecision(5)<<coeffs[i][j];
        cout<<endl;
        cout<<" "<<endl;
    }
}
#endif
