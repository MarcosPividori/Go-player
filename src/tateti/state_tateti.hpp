
#include "defines.h"

using namespace std;

class StateTateti
{
    private:
        CELL A[3][3];
    public:
        Player turn;
        StateTateti();
        StateTateti(StateTateti *src);
        void get_possible_moves(vector<DataTateti>& v);
        void apply(DataTateti);
        ValTateti get_final_value();
        bool valid_move(DataTateti);
        void show();
};

