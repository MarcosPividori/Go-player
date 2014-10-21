
#include "mcts_utils.hpp"
#ifdef RAVE
 #include "mcts_rave.hpp"
 #include "moverecorder_hexa.hpp"
#else
 #include "mcts_uct.hpp"
#endif
#include <iostream>
#include <cassert>
#include <thread>
#include <iomanip>

#define MOVE(i,j,player) ((i)*22+(j)*2+(player))
#define I(m) (m)/22
#define J(m) ((m)%22)/2
#define Player(m) (m)%2
#define PlayerToCell(p) (p==Circle ? CIRCLE : CROSS)
#define CellToPlayer(p) (p==CIRCLE ? Circle : (p==CROSS ? Cross : assert(0)))
#define ChangePlayer(p) (p==Circle ? Cross  : Circle)
#define NUM_THREADS 5
#define NUM_CYCLES 50000

typedef long DataHexa;
typedef double ValHexa;
typedef NodeUCT<ValHexa,DataHexa> Nod;

typedef enum {
    Circle = 0,
    Cross  = 1,
} Player;

typedef enum {
    EMPTY  =  0,
    CIRCLE = -1,
    CROSS  =  1,
} CELL;

class StateHexa: public States<ValHexa,DataHexa>{
    private:
        CELL A[11][11];
        bool check_vertical(int i,int j,CELL p,bool visited[][11]);
        bool check_horizontal(int i,int j,CELL p,bool visited[][11]);
    public:
        Player turn;
        StateHexa();
        virtual StateHexa *copy();
        virtual void get_possible_moves(std::vector<DataHexa>& v);
        virtual void apply(DataHexa);
        virtual ValHexa get_final_value();
        bool valid_move(DataHexa);
        void show();
};

StateHexa::StateHexa()
{
    for(int i=0;i<11;i++)
        for(int j=0;j<11;j++)
            A[i][j]=EMPTY;
    turn=Cross;
}

StateHexa *StateHexa::copy()
{
    StateHexa *s = new StateHexa();
    *s = *this;
    return s;
}

void StateHexa::get_possible_moves(std::vector<DataHexa>& v)
{
    if(get_final_value() !=0)
        return;
    bool empty=true;
    for(int i=0;i<11;i++){
        for(int j=0;j<11;j++)
            if(A[i][j]==PlayerToCell(turn)){
                empty=false;
                break;
            }
        if(!empty)break;    
    }
    for(int i=0;i<11;i++)
        for(int j=0;j<11;j++)
            if(A[i][j]==EMPTY && (empty || (i>0 && A[i-1][j]!=EMPTY)
                                        || (i<10 && A[i+1][j]!=EMPTY)
                                        || (j>0 && A[i][j-1]!=EMPTY)
                                        || (j<10 && A[i][j+1]!=EMPTY)
                                        || (i>0 && j<10 && A[i-1][j+1]!=EMPTY)
                                        || (i<10 && j>0 && A[i+1][j-1]!=EMPTY)))
                v.push_back(MOVE(i,j,turn));
}

bool StateHexa::check_vertical(int i,int j,CELL p,bool visited[][11])
{
    visited[i][j]=true;
    if(i==10)
        return true;
    if(j>0 && A[i][j-1]==p && !visited[i][j-1])
        if(check_vertical(i,j-1,p,visited))
            return true;
    if(j<10 && A[i][j+1]==p && !visited[i][j+1])
        if(check_vertical(i,j+1,p,visited))
            return true;
    if(i<10 && A[i+1][j]==p && !visited[i+1][j])
        if(check_vertical(i+1,j,p,visited))
            return true;
    if(i<10 && j>0 && A[i+1][j-1]==p && !visited[i+1][j-1])
        if(check_vertical(i+1,j-1,p,visited))
            return true;
    return false;
}

bool StateHexa::check_horizontal(int i,int j,CELL p,bool visited[][11])
{
    visited[i][j]=true;
    if(j==10)
        return true;
    if(i>0 && A[i-1][j]==p && !visited[i-1][j])
        if(check_horizontal(i-1,j,p,visited))
            return true;
    if(i<10 && A[i+1][j]==p && !visited[i+1][j])
        if(check_horizontal(i+1,j,p,visited))
            return true;
    if(j<10 && A[i][j+1]==p && !visited[i][j+1])
        if(check_horizontal(i,j+1,p,visited))
            return true;
    if(j<10 && i>0 && A[i-1][j+1]==p && !visited[i-1][j+1])
        if(check_horizontal(i-1,j+1,p,visited))
            return true;
    return false;
}

ValHexa StateHexa::get_final_value()
{
    bool visited[11][11];
    for(int i=0;i<11;i++)
        for(int j=0;j<11;j++)
            visited[i][j]=false;
    for(int i=0;i<11;i++)
        if(A[i][0]==CROSS && !visited[i][0] && check_horizontal(i,0,CROSS,visited))
            return CROSS;
    for(int i=0;i<11;i++)
        for(int j=0;j<11;j++)
            visited[i][j]=false;
    for(int j=0;j<11;j++)
        if(A[0][j]==CIRCLE && !visited[0][j] && check_vertical(0,j,CIRCLE,visited))
            return CIRCLE;
    return EMPTY;
}

void StateHexa::apply(DataHexa d)
{
    assert(Player(d) == turn);
    assert(A[I(d)][J(d)] == EMPTY);
    A[I(d)][J(d)]=PlayerToCell(turn);
    turn=ChangePlayer(turn);
}

void StateHexa::show()
{
    std::cout<<"    ";
    for(int j=0;j<9;j++)
        std::cout<<" "<<(j+1)<<"  ";
    std::cout<<" 10  11 "<<std::endl;
    std::cout<<"     ";
    for(int j=0;j<11;j++)
        std::cout<<" oo ";
    std::cout<<std::endl;
    for(int i=0;i<11;i++){
        if(i!=0)
            std::cout<<" ";
        if(i!=0)
            std::cout<<" ";
        for(int j=i;j>1;j--)
            std::cout<<"  ";
        if(i>=9)
            std::cout<<" ";
        std::cout<<std::setw(2)<<(i+1);
        if(i<9)
            std::cout<<" ";
        std::cout<<" ++";
        for(int j=0;j<11;j++)
            std::cout<<"("<<(A[i][j] == EMPTY ? "  " : (A[i][j]==CROSS ? "++" : "oo"))<<")";
        std::cout<<"++"<<std::endl;
    }
    std::cout<<"       ";
    for(int j=1;j<11;j++)
        std::cout<<"  ";
    for(int j=0;j<11;j++)
        std::cout<<" oo ";
    std::cout<<std::endl;
}

bool StateHexa::valid_move(DataHexa d)
{
    return A[I(d)][J(d)]==EMPTY;
}

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
    SelectionUCT<ValHexa,DataHexa> sel(1);
    ExpansionAllChildren<ValHexa,DataHexa,StateHexa,Nod> exp(2,0);
#ifdef RAVE
    Mcts<ValHexa,DataHexa,Nod,StateHexa> *m[NUM_THREADS];
    SimulationAndRetropropagationRave<ValHexa,DataHexa,StateHexa,EvalNode,MoveRecorderHexa> *sim_and_retro[NUM_THREADS];
    for(int i=0;i<NUM_THREADS,i++){
      sim_and_retro[i]=new SimulationAndRetropropagationRave<ValHexa,DataHexa,StateHexa,EvalNode,MoveRecorderHexa>();
      m[i]=new Mcts<ValHexa,DataHexa,Nod,StateHexa>(&sel,&exp,sim_and_retro[i],sim_and_retro[i],&sel_res,&mutex);
    }
#else
    SimulationTotallyRandom<ValHexa,DataHexa,StateHexa> sim;
    RetropropagationSimple<ValHexa,DataHexa,EvalNode> ret;
    Mcts<ValHexa,DataHexa,Nod,StateHexa> m(&sel,&exp,&sim,&ret,&sel_res,&mutex);
#endif
    SelectResMostRobust<ValHexa,DataHexa,Nod> sel_res;
    
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
    std::cout << "RESULT: " << (state.get_final_value()==1  ? "++ wins." :
                               (state.get_final_value()==-1 ? "oo wins." : "Tie.")) <<std::endl<<std::endl;

#ifdef RAVE
    for(int i=0;i<NUM_THREADS,i++){
      delete sim_and_retro[i];
      delete m[i];
    }
#endif
    return 0;
}

