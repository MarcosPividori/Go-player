
#include <iostream>
#include "mcts_utils.hpp"
#include "defines.h"

typedef double ValGo;

class StateGo : public States<ValGo,DataGo>
{
#ifdef DEBUG
    public:
        Player **Stones;
        int ***Blocks;
    private:
#else
    private:
        Player **Stones;
        int ***Blocks;
#endif
        const int _size;
        const float _komi;
        INDEX koi;
        INDEX koj;
        bool ko_flag;
        bool ko_unique;
        char pass;
        void eliminate_block(int *block,INDEX i,INDEX j);
        void update_block(int *block,int *new_block,INDEX i,INDEX j);
        unsigned int count_area(bool **visited,INDEX i,INDEX j);
        bool no_ko_nor_suicide(INDEX i,INDEX j,Player p);
        float final_value();
#ifdef JAPANESE
        int captured_b;
        int captured_w;
#endif
    public:
        Player turn;
        StateGo(int size,float komi=0);
        ~StateGo();
        StateGo *copy();
        void get_possible_moves(std::vector<DataGo>& v);
        void apply(DataGo);
        ValGo get_final_value();
        float get_final_score();
        bool valid_move(DataGo);
        void show(FILE *output);
        void show();
};

