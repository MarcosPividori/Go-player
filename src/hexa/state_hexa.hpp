
#include "defines.h"

using namespace std;

class StateHexa
{
    private:
        int _size;
        CELL **A;
        bool check_vertical(int i,int j,CELL p,bool **visited);
        bool check_horizontal(int i,int j,CELL p,bool **visited);
    public:
        Player turn;
        StateHexa(int size);
        StateHexa(StateHexa *src);
        ~StateHexa();
        void get_possible_moves(vector<DataHexa>& v);
        void apply(DataHexa);
        ValHexa get_final_value();
        bool valid_move(DataHexa);
        void show();
};
