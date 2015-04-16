
#include "state_tateti.hpp"

StateTateti::StateTateti()
{
    for(int i=0;i<3;i++)
        for(int j=0;j<3;j++)
            A[i][j]=EMPTY;
    turn=Cross;
}

StateTateti::StateTateti(StateTateti *src)
{
    *this = *src;
}

void StateTateti::get_possible_moves(vector<DataTateti>& v)
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
    cout<<"  1 2 3"<<endl;
    for(int i=0;i<3;i++){
        cout<<i+1;
        for(int j=0;j<3;j++)
            cout<<(A[i][j] == EMPTY ? " -" : (A[i][j]==CROSS ? " X" : " O"));
        cout<<endl;
    }
}

bool StateTateti::valid_move(DataTateti d)
{
    return A[I(d)][J(d)]==EMPTY;
}

