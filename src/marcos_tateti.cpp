
#include "mcts_utils.hpp"
#include "mcts_uct.hpp"
#include <iostream>
#include <cassert>
#include <thread>

#define MOVE(i,j,player) ((i)*6+(j)*2+(player))
#define I(m) (m)/6
#define J(m) ((m)%6)/2
#define Player(m) (m)%2
#define PlayerToCell(p) (p==Circle ? CIRCLE : CROSS)
#define CellToPlayer(p) (p==CIRCLE ? Circle : (p==CROSS ? Cross : assert(0)))
#define ChangePlayer(p) (p==Circle ? Cross  : Circle)
#define NUM_THREADS 5
#define NUM_CYCLES 100000

typedef long DataTateti;
typedef double ValTateti;
typedef NodeUCT<ValTateti,DataTateti> Nod;

typedef enum {
    Circle = 0,
    Cross  = 1,
} Player;

typedef enum {
    EMPTY  =  0,
    CIRCLE = -1,
    CROSS  =  1,
} CELL;

class StateTateti: public States<ValTateti,DataTateti>{
    private:
        CELL A[3][3];
    public:
        Player turn;
        StateTateti();
        virtual StateTateti *copy();
        virtual void get_possible_moves(std::vector<DataTateti>& v);
        virtual void apply(DataTateti);
        virtual ValTateti get_final_value();
        bool valid_move(DataTateti);
        void show();
};

StateTateti::StateTateti()
{
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            A[i][j]=EMPTY;
    turn=Cross;
}

StateTateti *StateTateti::copy()
{
    StateTateti *s = new StateTateti();
    *s = *this;
    return s;
}

void StateTateti::get_possible_moves(std::vector<DataTateti>& v)
{
    if(get_final_value() !=0)
        return;
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            if(A[i][j]==EMPTY)
                v.push_back(MOVE(i,j,turn));
}

ValTateti StateTateti::get_final_value()
{
    int sum;
    for(int i=0;i<3;i++){
        sum=0;
        for(int j=0;j<3;j++)
            sum+=A[i][j];
        if(sum==CROSS*3)
            return CROSS;
        if(sum==CIRCLE*3)
            return CIRCLE;
    }
    for(int j=0;j<3;j++){
        sum=0;
        for(int i=0;i<3;i++)
            sum+=A[i][j];
        if(sum== CROSS*3)
            return CROSS;
        if(sum== CIRCLE*3)
            return CIRCLE;
    }
    sum = A[0][0] + A[1][1] + A[2][2];
    if(sum==CROSS*3)
        return CROSS;
    if(sum==CIRCLE*3)
        return CIRCLE;
    sum = A[0][2] + A[1][1] + A[2][0];
    if(sum==CROSS*3)
        return CROSS;
    if(sum==CIRCLE*3)
        return CIRCLE;
    return EMPTY;
}

void StateTateti::apply(DataTateti d)
{
    assert(Player(d) == turn);
    assert(A[I(d)][J(d)] == EMPTY);
    A[I(d)][J(d)]=PlayerToCell(turn);
    turn=ChangePlayer(turn);
}

void StateTateti::show()
{
    std::cout<<"  1 2 3"<<std::endl;
    for(int i=0;i<3;i++){
        std::cout<<i+1;
        for(int j=0;j<3;j++)
            std::cout<<(A[i][j] == EMPTY ? " -" : (A[i][j]==CROSS ? " X" : " O"));
        std::cout<<std::endl;
    }
}

bool StateTateti::valid_move(DataTateti d)
{
    return A[I(d)][J(d)]==EMPTY;
}

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
        std::cout<<"Insert mov: ";
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

