
#include "defines.h"
#include "states.hpp"

class StateTateti: public States<ValTateti,DataTateti>{
    private:
        CELL A[3][3];
    public:
        Player turn;
        StateTateti();
        virtual StateTateti *copy();
        virtual void get_possible_moves(std::vector<DataTateti>& v);
        virtual void apply(DataTateti);
        virtual ValTateti get_final_value();
        bool valid_move(DataTateti);
        void show();
};

