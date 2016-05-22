
#include "state_connect4.hpp"

StateConnect4::StateConnect4()
{
    for(int i=0;i<6;i++)
        for(int j=0;j<7;j++)
            A[i][j]=EMPTY;
    turn=CROSS_P;
}

StateConnect4::StateConnect4(StateConnect4 *src)
{
    *this = *src;
}

void StateConnect4::get_possible_moves(vector<DataConnect4>& v) const
{
    if(get_final_value() !=EMPTY)
        return;
    for(int j=0;j<7;j++)
        if(A[0][j]==EMPTY)
            v.push_back(DataConnect4(j,turn));
}

ValConnect4 StateConnect4::get_final_value() const
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
    assert(d.player == turn);
    assert(A[0][d.pos] == EMPTY);
    for(int i=0;i<6;i++){
        if(A[i][d.pos] != EMPTY){
            A[i-1][d.pos]=PlayerToCell(turn);
            break;
        }
        if(i==5)
            A[5][d.pos]=PlayerToCell(turn);
    }
    turn=ChangePlayer(turn);
}

void StateConnect4::show() const
{
    for(int j=0;j<7;j++)
        cout<<" "<<j;
    cout<<endl;
    for(int i=0;i<6;i++){
        for(int j=0;j<7;j++)
            cout<<(A[i][j] == EMPTY ? " -" : (A[i][j]==CROSS ? " X" : " O"));
        cout<<endl;
    }
}

bool StateConnect4::valid_move(DataConnect4 d) const
{
    return A[0][d.pos]==EMPTY;
}

