
#include "defines.h"
#include "mcts_rave.hpp"

class MoveRecorderHexa: public MoveRecorder<DataHexa>{
    private:
        CELL A[11][11];
    public:
        void clear();
        void postMove(DataHexa move);
        void prevMove(DataHexa move); 
        bool isMove(DataHexa move);
};

inline void MoveRecorderHexa::clear()
{
    for(int i=0;i<11;i++)
        for(int j=0;j<11;j++)
            A[i][j]=EMPTY;
}

inline void MoveRecorderHexa::postMove(DataHexa move)
{
    A[I(move)][J(move)]=PlayerToCell(Player(move));
}

inline void MoveRecorderHexa::prevMove(DataHexa move)
{
    A[I(move)][J(move)]=PlayerToCell(Player(move));
}

inline bool MoveRecorderHexa::isMove(DataHexa move)
{
    return A[I(move)][J(move)]==PlayerToCell(Player(move));
}
