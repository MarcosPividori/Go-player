
#include "defines.h"

class StateConnect4
{
    private:
        CELL A[6][7];
    public:
        Player turn;
        StateConnect4();
        StateConnect4(StateConnect4* src);
        void get_possible_moves(std::vector<DataConnect4>& v);
        void apply(DataConnect4);
        ValConnect4 get_final_value();
        bool valid_move(DataConnect4);
        void show();
};

