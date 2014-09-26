
#include "mcts_utils.hpp"
#include <iostream>
#include <cassert>
#include <thread>

#define MOVE(pos,player) (pos*2+player)
#define Pos(m) m/2
#define Player(m) m%2
#define PlayerToCell(p) (p==Circle ? CIRCLE : CROSS)
#define CellToPlayer(p) (p==CIRCLE ? Circle : (p==CROSS ? Cross : assert(0)))
#define ChangePlayer(p) (p==Circle ? Cross  : Circle)

typedef long DataConnect4;
typedef double ValConnect4;

typedef enum {
    Circle = 0,
    Cross  = 1,
} Player;

typedef enum {
    EMPTY  =  0,
    CIRCLE = -1,
    CROSS  =  1,
} CELL;

class StateConnect4: public State<ValConnect4,DataConnect4>{
    private:
        CELL A[6][7];
    public:
        Player turn;
        StateConnect4();
        virtual StateConnect4 *copy();
        virtual void get_possible_moves(std::vector<DataConnect4>& v);
        virtual void apply(DataConnect4);
        virtual ValConnect4 get_final_value();
        bool valid_move(DataConnect4);
        virtual void show();
};

StateConnect4::StateConnect4()
{
    for(int i=0;i<6;i++)
        for(int j=0;j<7;j++)
            A[i][j]=EMPTY;
    turn=Cross;
}

StateConnect4 *StateConnect4::copy()
{
    StateConnect4 *s = new StateConnect4();
    *s = *this;
    return s;
}

void StateConnect4::get_possible_moves(std::vector<DataConnect4>& v)
{
    if(get_final_value() !=EMPTY)
        return;
    for(int j=0;j<7;j++)
        if(A[0][j]==EMPTY)
            v.push_back(MOVE(j,turn));
}

ValConnect4 StateConnect4::get_final_value()
{
    int sum;
    for(int i=0;i<6;i++){
        sum=0;
        for(int j=0;j<7;j++){
            sum=A[i][j] + sum * (A[i][j]*sum > 0);
            if(sum==CROSS*4)
                return CROSS;
            if(sum==CIRCLE*4)
                return CIRCLE;
        }
    }
    for(int j=0;j<7;j++){
        sum=0;
        for(int i=0;i<6;i++){
            sum=A[i][j] + sum * (A[i][j]*sum > 0);
            if(sum== CROSS*4)
                return CROSS;
            if(sum== CIRCLE*4)
                return CIRCLE;
        }
    }
    for(int i=3;i<6;i++){
        sum=0;
        for(int j=0,k=i;j<7 && k>=0;j++,k--){
            sum=A[k][j] + sum * (A[k][j]*sum > 0);
            if(sum==CROSS*4)
                return CROSS;
            if(sum==CIRCLE*4)
                return CIRCLE;
        }
    }
    for(int j=1;j<4;j++){
        sum=0;
        for(int i=5,k=j;i>=0 && k<7;k++,i--){
            sum=A[i][k] + sum * (A[i][k]*sum > 0);
            if(sum==CROSS*4)
                return CROSS;
            if(sum==CIRCLE*4)
                return CIRCLE;
        }
    }
    for(int i=0;i<3;i++){
        sum=0;
        for(int j=0,k=i;j<7 && k<6;j++,k++){
            sum=A[k][j] + sum * (A[k][j]*sum > 0);
            if(sum==CROSS*4)
                return CROSS;
            if(sum==CIRCLE*4)
                return CIRCLE;
        }
    }
    for(int j=1;j<4;j++){
        sum=0;
        for(int i=0,k=j;i<6 && k<7;k++,i++){
            sum=A[i][k] + sum * (A[i][k]*sum > 0);
            if(sum==CROSS*4)
                return CROSS;
            if(sum==CIRCLE*4)
                return CIRCLE;
        }
    }
    return EMPTY;
}

void StateConnect4::apply(DataConnect4 d)
{
    assert(Player(d) == turn);
    assert(A[0][Pos(d)] == EMPTY);
    for(int i=0;i<6;i++){
        if(A[i][Pos(d)] != EMPTY){
            A[i-1][Pos(d)]=PlayerToCell(turn);
            break;
        }
        if(i==5)
            A[5][Pos(d)]=PlayerToCell(turn);
    }
    turn=ChangePlayer(turn);
}

void StateConnect4::show()
{
    for(int i=0;i<6;i++){
        for(int j=0;j<7;j++)
            std::cout<<(A[i][j] == EMPTY ? " -" : (A[i][j]==CROSS ? " X" : " O"));
        std::cout<<std::endl;
    }
}

bool StateConnect4::valid_move(DataConnect4 d)
{
    return A[0][Pos(d)]==EMPTY;
}

ValConnect4 fun(ValConnect4 v_nodo,ValConnect4 v_final,DataConnect4 dat_nodo)
{
    if(v_final == EMPTY)//Tie
        return v_nodo+0.9999;
    if(v_final == PlayerToCell(Player(dat_nodo)))
        return v_nodo+1;
    return v_nodo;
}

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
        std::cout<<"Insert mov: ";
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
    NodeUCT<ValConnect4,DataConnect4> *nod=new NodeUCT<ValConnect4,DataConnect4>(0,MOVE(Circle,0),NULL),*next;
    StateConnect4 state;
    DataConnect4 res;
    std::vector<DataConnect4> v;
    
    SelectionUCT<ValConnect4,DataConnect4> sel(1);
    ExpansionAllChildren<ValConnect4,DataConnect4> exp(2,0);
    SimulationTotallyRandom<ValConnect4,DataConnect4> sim;
    RetropropagationSimple<ValConnect4,DataConnect4> ret(fun);
    SelectResMostRobust<ValConnect4,DataConnect4> sel_res;
    Mcts<ValConnect4,DataConnect4,NodeUCT<ValConnect4,DataConnect4>> m(&sel,&exp,&sim,&ret,&sel_res);
    
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
            std::thread threads[5];
            for(int i=0;i<5;i++)
                threads[i] = std::thread(&Mcts<ValConnect4,DataConnect4,NodeUCT<ValConnect4,DataConnect4>>::run_time,&m,5,nod,&state);
            for(std::thread& th : threads)
                th.join();
            //m.run_cycles(100000,nod,&state);
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
            nod= new NodeUCT<ValConnect4,DataConnect4>(0,res,NULL);
        }

        state.show();
    }

    std::cout<<std::endl<<"------------------"<<std::endl<<std::endl;
    std::cout << "RESULT: " << (state.get_final_value()==1  ? "X wins." :
                               (state.get_final_value()==-1 ? "O wins." : "Tie.")) <<std::endl<<std::endl;
    return 0;
}

