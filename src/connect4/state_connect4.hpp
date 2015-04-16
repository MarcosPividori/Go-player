
#include "defines.h"

using namespace std;

class StateConnect4
{
    private:
        CELL A[6][7];
    public:
        Player turn;
        StateConnect4();
        StateConnect4(StateConnect4* src);
        void get_possible_moves(vector<DataConnect4>& v);
        void apply(DataConnect4);
        ValConnect4 get_final_value();
        bool valid_move(DataConnect4);
        void show();
};

