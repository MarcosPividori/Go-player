
#include "defines.h"

using namespace std;

class StateTateti
{
    private:
        Cell A[3][3];
    public:
        Player turn;
        StateTateti();
        StateTateti(StateTateti *src);
        void get_possible_moves(vector<DataTateti>& v) const;
        void apply(DataTateti);
        ValTateti get_final_value() const;
        bool valid_move(DataTateti) const;
        void show() const;
};

