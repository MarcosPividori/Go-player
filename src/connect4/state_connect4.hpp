
#include "defines.h"
#include "states.hpp"

class StateConnect4: public States<ValConnect4,DataConnect4>{
    private:
        CELL A[6][7];
    public:
        Player turn;
        StateConnect4();
        virtual StateConnect4 *copy();
        virtual void get_possible_moves(std::vector<DataConnect4>& v);
        virtual void apply(DataConnect4);
        virtual ValConnect4 get_final_value();
        bool valid_move(DataConnect4);
        virtual void show();
};

