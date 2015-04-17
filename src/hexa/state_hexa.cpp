
#include "state_hexa.hpp"
#include <iomanip>

#define FOR_EACH_ADJ(i,j,k,l,Code)  ( \
            { \
              if(i>0){ \
                       { k=i-1;l=j;Code;} \
                       if(j<_size-1) { k=i-1;l=j+1;Code;} \
                     } \
              if(i<_size-1){ \
                             { k=i+1;l=j;Code;} \
                             if(j>0) { k=i+1;l=j-1;Code;} \
                           } \
              if(j>0)      { k=i;l=j-1;Code;} \
              if(j<_size-1){ k=i;l=j+1;Code;} \
            })

using namespace std;

StateHexa::StateHexa(int size) : _size(size)
{
    A = new CELL*[size];
    for(int i=0;i<size;i++){
        A[i] = new CELL[size];
        for(int j=0;j<size;j++)
            A[i][j]=EMPTY;
    }
    turn=Cross;
}

StateHexa::StateHexa(StateHexa *src) : _size(src->_size)
{
    A = new CELL*[_size];
    for(int i=0;i<_size;i++){
        A[i] = new CELL[_size];
        for(int j=0;j<_size;j++)
            A[i][j]=src->A[i][j];
    }
    turn = src->turn;
}

StateHexa::~StateHexa()
{
    for(int i=0;i<_size;i++)
        delete[] A[i];
    delete[] A;
}

void StateHexa::get_possible_moves(vector<DataHexa>& v)
{
    if(get_final_value() !=0)
        return;
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++)
            if(A[i][j]==EMPTY)
                v.push_back(MOVE(i,j,turn));
}

bool StateHexa::check_vertical(int i,int j,CELL p,bool **visited)
{
    visited[i][j]=true;
    if(i==_size-1)
        return true;
    int k,l;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(A[k][l]==p && !visited[k][l])
        if(check_vertical(k,l,p,visited))
            return true;
    }
    );
    return false;
}

bool StateHexa::check_horizontal(int i,int j,CELL p,bool **visited)
{
    visited[i][j]=true;
    if(j==_size-1)
        return true;
    int k,l;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(A[k][l]==p && !visited[k][l])
        if(check_horizontal(k,l,p,visited))
            return true;
    }
    );
    return false;
}

ValHexa StateHexa::get_final_value()
{
    bool **visited=new bool*[_size];
    for(int i=0;i<_size;i++){
        visited[i]=new bool[_size];
        for(int j=0;j<_size;j++)
            visited[i][j]=false;
    }
    ValHexa res= EMPTY;
    for(int i=0;i<_size;i++)
        if(A[i][0]==CROSS && !visited[i][0] && check_horizontal(i,0,CROSS,visited))
            {res=CROSS;break;}
    if(res==EMPTY){
        for(int i=0;i<_size;i++)
            for(int j=0;j<_size;j++)
                visited[i][j]=false;
        for(int j=0;j<_size;j++)
            if(A[0][j]==CIRCLE && !visited[0][j] && check_vertical(0,j,CIRCLE,visited))
                {res=CIRCLE;break;}
    }
    for(int i=0;i<_size;i++)
        delete[] visited[i];
    delete[] visited;
    return res;
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
    cout<<"    ";
    for(int j=0;j<_size;j++)
        cout<<setw(3)<<j+1<<" ";
    cout<<endl;
    cout<<"     ";
    for(int j=0;j<_size;j++)
        cout<<" oo ";
    cout<<endl;
    for(int i=0;i<_size;i++){
        if(i!=0)
            cout<<" ";
        if(i!=0)
            cout<<" ";
        for(int j=i;j>1;j--)
            cout<<"  ";
        if(i>=9)
            cout<<" ";
        cout<<setw(2)<<(i+1);
        if(i<9)
            cout<<" ";
        cout<<" ++";
        for(int j=0;j<_size;j++)
            cout<<"("<<(A[i][j] == EMPTY ? "  " : (A[i][j]==CROSS ? "++" : "oo"))<<")";
        cout<<"++"<<endl;
    }
    cout<<"       ";
    for(int j=1;j<_size;j++)
        cout<<"  ";
    for(int j=0;j<_size;j++)
        cout<<" oo ";
    cout<<endl;
}

bool StateHexa::valid_move(DataHexa d)
{
    return A[I(d)][J(d)]==EMPTY;
}

