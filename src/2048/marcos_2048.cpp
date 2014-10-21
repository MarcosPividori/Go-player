
#include "mcts_utils.hpp"
#include <iostream>
#include <cassert>
#include <random>
#include <iomanip>

#define EMPTY 0

typedef long Data2048;
typedef double Val2048;

class State2048: public State<Val2048,Data2048>{
    private:
        int number_empty();
    public:
        int A[4][4];
        State2048();
        State2048 *copy();
        virtual void get_possible_moves(std::vector<Data2048>& v);
        virtual void apply(Data2048);
        virtual Val2048 get_final_value();
        bool valid_move(Data2048);
        void show();
};

State2048::State2048()
{
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            A[i][j]=EMPTY;
}

State2048 *State2048::copy()
{
    State2048 *p= new State2048();
    *p=*this;
    return p;
}

void State2048::get_possible_moves(std::vector<Data2048>& v)
{
    if(get_final_value() ==1)
        return;
    int mov[4]={0,0,0,0},k,l,pos;
    for(int i=0;i<4;i++)
      for(int j=0;j<4;j++)
        for(int h=0;h<4;h++){
          switch(h){
            case 0 : k=i+1;l=j;break;  
            case 1 : k=i;l=j-1;break;  
            case 2 : k=i-1;l=j;break;  
            case 3 : k=i;l=j+1;break;  
          }
          if(k>=0 && k<4 && l>=0 && l<4)
            if((A[i][j]!=EMPTY && A[k][l]==EMPTY) || A[i][j]==A[k][l])
              mov[h]=1;
        }
    State2048 cop;
    for(int i=0;i<4;i++)
      if(mov[i]!=0){
        for(int j=0;j<4;j++)
          for(int k=0;k<4;k++)
            cop.A[j][k]=A[j][k];
        cop.apply(i);
        int num=number_empty()+1;
        for(int j=0;j<num;j++)
            v.push_back(j*4+i);
      }
}

int State2048::number_empty()
{
    int count=0;
    for(int i=0;i<4;i++)
      for(int j=0;j<4;j++)
        if(A[i][j]==EMPTY)
          count++;
    return count;
}

Val2048 State2048::get_final_value()
{
    bool _1024=false,_512=false,_256=false,res;
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++){
            if(A[i][j]==2048)
                return 1;
            if(A[i][j]==1024)
                _1024=true;
            if(A[i][j]==512)
                _512=true;
            if(A[i][j]==256)
                _256=true;
        }
    if(_1024)
        return 0.75;
    if(_512)
        return 0.5;
    if(_256)
        return 0.25;
    return 0;
}

void State2048::apply(Data2048 d)
{
    int pos = d/4;
    d=d%4;
    if(d==0)
    {
        for(int j=0;j<4;j++)
          for(int i=3;i>0;i--){
            if(A[i][j]==EMPTY)
              continue;
            for(int k=i-1;k>=0;k--){
              if(A[i][j] == A[k][j]){
                A[i][j]*=2;
                A[k][j]=EMPTY;
                break;
              }
              if(A[k][j] != EMPTY)
                break;
            }
          }
        for(int j=0;j<4;j++)
          for(int i=3;i>0;i--){
            if(A[i][j]!=EMPTY)
              continue;
            for(int k=i-1;k>=0;k--){
              if(A[k][j] != EMPTY){
                A[i][j]=A[k][j];
                A[k][j]=EMPTY;
                break;
              }
            }
          }
    }
    if(d==1)
    {
        for(int i=0;i<4;i++)
          for(int j=0;j<3;j++){
            if(A[i][j]==EMPTY)
              continue;
            for(int k=j+1;k<4;k++){
              if(A[i][j] == A[i][k]){
                A[i][j]*=2;
                A[i][k]=EMPTY;
                break;
              }
              if(A[i][k] != EMPTY)
                break;
            }
          }
        for(int i=0;i<4;i++)
          for(int j=0;j<3;j++){
            if(A[i][j]!=EMPTY)
              continue;
            for(int k=j+1;k<4;k++){
              if(A[i][k] != EMPTY){
                A[i][j]=A[i][k];
                A[i][k]=EMPTY;
                break;
              }
            }
          }
    }
    if(d==2)
    {
        for(int j=0;j<4;j++)
          for(int i=0;i<3;i++){
            if(A[i][j]==EMPTY)
              continue;
            for(int k=i+1;k<4;k++){
              if(A[i][j] == A[k][j]){
                A[i][j]*=2;
                A[k][j]=EMPTY;
                break;
              }
              if(A[k][j] != EMPTY)
                break;
            }
          }
        for(int j=0;j<4;j++)
          for(int i=0;i<3;i++){
            if(A[i][j]!=EMPTY)
              continue;
            for(int k=i+1;k<4;k++){
              if(A[k][j] != EMPTY){
                A[i][j]=A[k][j];
                A[k][j]=EMPTY;
                break;
              }
            }
          }
    }
    if(d==3)
    {
        for(int i=3;i>=0;i--)
          for(int j=3;j>0;j--){
            if(A[i][j]==EMPTY)
              continue;
            for(int k=j-1;k>=0;k--){
              if(A[i][j] == A[i][k]){
                A[i][j]*=2;
                A[i][k]=EMPTY;
                break;
              }
              if(A[i][k] != EMPTY)
                break;
            }
          }
        for(int i=3;i>=0;i--)
          for(int j=3;j>0;j--){
            if(A[i][j]!=EMPTY)
              continue;
            for(int k=j-1;k>=0;k--){
              if(A[i][k] != EMPTY){
                A[i][j]=A[i][k];
                A[i][k]=EMPTY;
                break;
              }
            }
          }
    }
    
    std::vector<int> v;
    for(int i=0;i<4;i++)
        for(int j=0;j<4;j++)
            if(A[i][j]==EMPTY)
                v.push_back(i*4+j);
    if(v.size()<=0){
        show();
        std::cout<<"Posicion: "<<d<<std::endl;
    }
    assert(v.size()>0);
    pos = pos % v.size();
    A[v[pos]/4][v[pos]%4]=2;
}

void State2048::show()
{
    for(int i=0;i<4;i++){
        for(int j=0;j<4;j++)
            if(A[i][j] == EMPTY)
                std::cout<<"  -- ";
            else{
                std::cout<<std::setw(5)<<A[i][j];
            }
        std::cout<<std::endl;
    }
}

bool State2048::valid_move(Data2048 d)
{
    std::vector<Data2048> v;
    get_possible_moves(v);
    for(int i=0;i<v.size();i++)
        if(v[i]==d)
            return true;
    return false;
}

Val2048 fun(Val2048 v_nodo,Val2048 v_final,Data2048 dat_nodo)
{
    //if(v_final == 1)
    //    return v_nodo+1;
    return v_nodo+v_final;
}

Data2048 insert_mov(State2048 *state)
{
    int j;
    while(1){
        std::cout<<"Insert mov (-1 computer decide): ";
        if((std::cin>>j) && (j==-1 || ((j>=0 && j<4) && state->valid_move(j))))
            break;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
    }
    std::cout<<std::endl;
    return j;
}

int main()
{
    NodeUCT<Val2048,Data2048> *nod=new NodeUCT<Val2048,Data2048>(0,0,NULL),*next;
    Data2048 res;
    std::vector<Data2048> v;
    
    SelectionUCT<Val2048,Data2048> sel(1);
    ExpansionAllChildren<Val2048,Data2048> exp(2,0);
    SimulationTotallyRandom<Val2048,Data2048> sim;
    RetropropagationSimple<Val2048,Data2048> ret(fun);
    SelectResMostRobust<Val2048,Data2048> sel_res;
    Mcts<Val2048,Data2048,NodeUCT<Val2048,Data2048>> m(&sel,&exp,&sim,&ret,&sel_res);
    
    std::cout<< "2048:"<<std::endl;
  
    State2048 state;
    state.A[1][0]=512;
    state.A[2][1]=256;
    state.A[2][2]=128;
    state.A[2][3]=1024;
 
    std::cout<<"------------------"<<std::endl<<std::endl;
    state.show();

    while((v.clear(),state.get_possible_moves(v),!v.empty())){
        std::cout<<std::endl;
        std::cout<<"-You play---------"<<std::endl<<std::endl;
        res=-1;//res=insert_mov(&state);
        if(res ==-1){
            std::cout<<"-Computer plays---"<<std::endl<<std::endl;
#ifdef DEBUG
            nod->show();
            exp.counter=0;
#endif
            m.run_cycles(2000,nod,&state);
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
            nod= new NodeUCT<Val2048,Data2048>(0,res,NULL);
        }

        state.show();
    }

    std::cout<<std::endl<<"------------------"<<std::endl<<std::endl;
    return 0;
}

