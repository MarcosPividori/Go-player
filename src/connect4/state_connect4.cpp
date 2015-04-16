
#include "state_connect4.hpp"

StateConnect4::StateConnect4()
{
    for(int i=0;i<6;i++)
        for(int j=0;j<7;j++)
            A[i][j]=EMPTY;
    turn=Cross;
}

StateConnect4::StateConnect4(StateConnect4 *src)
{
    *this = *src;
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
    for(int j=0;j<7;j++)
        std::cout<<" "<<j;
    std::cout<<std::endl;
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

