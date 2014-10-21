
#include "state_hexa.hpp"
#include <iomanip>

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

