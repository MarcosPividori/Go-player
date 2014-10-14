#ifndef __STATE_GO__
#define __STATE_GO__

#include <iostream>
#include "defines.h"
#include "states.hpp"
#include "pattern_list.hpp"

class PatternList;

typedef double ValGo;

class StateGo : public States<ValGo,DataGo>
{
    friend class PatternList;
    private:
        PatternList *patterns;
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
        DataGo last_mov;
        char pass;
        void eliminate_block(int *block,INDEX i,INDEX j);
        void update_block(int *block,int *new_block,INDEX i,INDEX j);
        unsigned int count_area(bool **visited,INDEX i,INDEX j);
        bool no_ko_nor_suicide(INDEX i,INDEX j,Player p);
        bool remove_opponent_block_and_no_ko(INDEX i,INDEX j,Player p);
        float final_value();
#ifdef JAPANESE
        int captured_b;
        int captured_w;
#endif
    public:
        Player turn;
        StateGo(int size,float komi,PatternList *p);
        ~StateGo();
        StateGo *copy();
        void get_possible_moves(std::vector<DataGo>& v);
#ifdef KNOWLEDGE
        void get_pattern_moves(std::vector<DataGo>& v);
        void get_capture_moves(std::vector<DataGo>& v);
        bool is_completely_empty(INDEX i,INDEX j);
#endif
        inline int size(){return _size;};
        void apply(DataGo);
        ValGo get_final_value();
        float get_final_score();
        bool valid_move(DataGo);
        void show(FILE *output);
        void show();
};

#endif // __STATE_GO__
