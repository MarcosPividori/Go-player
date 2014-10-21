
#include "defines.h"
#include "states.hpp"

class StateHexa: public States<ValHexa,DataHexa>{
    private:
        CELL A[11][11];
        bool check_vertical(int i,int j,CELL p,bool visited[][11]);
        bool check_horizontal(int i,int j,CELL p,bool visited[][11]);
    public:
        Player turn;
        StateHexa();
        virtual StateHexa *copy();
        virtual void get_possible_moves(std::vector<DataHexa>& v);
        virtual void apply(DataHexa);
        virtual ValHexa get_final_value();
        bool valid_move(DataHexa);
        void show();
};
