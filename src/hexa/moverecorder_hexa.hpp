
#include "defines.h"
#include "mcts_rave.hpp"

class MoveRecorderHexa: public MoveRecorder<DataHexa>{
    private:
        Cell A[11][11];
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
    A[move.i][move.j]=PlayerToCell(move.player);
}

inline void MoveRecorderHexa::prevMove(DataHexa move)
{
    A[move.i][move.j]=PlayerToCell(move.player);
}

inline bool MoveRecorderHexa::isMove(DataHexa move)
{
    return A[move.i][move.j]==PlayerToCell(move.player);
}
