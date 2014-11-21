
#include "defines.h"
#include "states.hpp"

class StateHexa: public States<ValHexa,DataHexa>{
    private:
        int _size;
        CELL **A;
        bool check_vertical(int i,int j,CELL p,bool **visited);
        bool check_horizontal(int i,int j,CELL p,bool **visited);
    public:
        Player turn;
        StateHexa(int size);
        ~StateHexa();
        virtual StateHexa *copy();
        virtual void get_possible_moves(std::vector<DataHexa>& v);
        virtual void apply(DataHexa);
        virtual ValHexa get_final_value();
        bool valid_move(DataHexa);
        void show();
};
