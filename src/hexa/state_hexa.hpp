
#include "defines.h"
#include "states.hpp"

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
        ~StateHexa();
        StateHexa *copy();
        void get_possible_moves(std::vector<DataHexa>& v);
        void apply(DataHexa);
        ValHexa get_final_value();
        bool valid_move(DataHexa);
        void show();
};
