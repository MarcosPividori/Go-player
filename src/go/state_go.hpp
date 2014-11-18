#ifndef __STATE_GO__
#define __STATE_GO__

#include <iostream>
#include "defines.h"
#include "states.hpp"
#include "pattern_list.hpp"
#include "avl_tree.hpp"

class PatternList;

typedef double ValGo;

class Block{
    public:
      int adj;
      int size;
      DataGo atari;
      inline bool is_atari() {return !IS_PASS(atari);};
      inline void no_atari() {atari=PASS(Empty);};
      inline void join(Block *b) {adj+=b->adj;size+=b->size;};
      Block() : adj(0),size(1),atari(PASS(Empty)) {};
};

class StateGo : public States<ValGo,DataGo>
{
    friend class PatternList;
    private:
        PatternList *patterns;
//#ifdef DEBUG
    public:
        Player **Stones;
        Block ***Blocks;
    private:
/*#else
    private:
        Player **Stones;
        Block ***Blocks;
#endif*/
        const int _size;
        const float _komi;
        DataGo ko;
        DataGo last_mov;
        unsigned char pass;
        std::vector<POS> b_atari;
        std::vector<POS> w_atari;
        AVLTree<POS> b_mov;
        AVLTree<POS> w_mov;
        void remove_atari_block(INDEX i,INDEX j);
        void add_atari_block(INDEX i,INDEX j);
        void eliminate_block(Block *block,INDEX i,INDEX j);
        void update_block(Block *block,Block *new_block,INDEX i,INDEX j);
        unsigned int count_area(bool **visited,INDEX i,INDEX j);
        void update_mov(INDEX i,INDEX j);
        bool no_suicide(INDEX i,INDEX j,Player p);
        bool no_ko_nor_suicide(INDEX i,INDEX j,Player p);
        bool no_self_atari_nor_suicide(INDEX i,INDEX j,Player p);
        bool remove_opponent_block_and_no_ko(INDEX i,INDEX j);
        unsigned int get_liberty_block(Block *block,Block *flag,INDEX i,INDEX j,INDEX &lib_i,INDEX &lib_j);
        bool is_block_in_atari(INDEX i,INDEX j,INDEX &i_atari,INDEX &j_atari);
        DataGo look_for_delete_atari(Block *block,Block *flag,INDEX i,INDEX j,int &max_size);
        DataGo get_delete_atari(INDEX i,INDEX j,int &b_size);
        float final_value();
#ifdef JAPANESE
        int captured_b;
        int captured_w;
#endif
        StateGo(StateGo *src);
    public:
        int num_movs;
        Player turn;
        StateGo(int size,float komi,PatternList *p);
        ~StateGo();
        StateGo *copy();
        void get_possible_moves(std::vector<DataGo>& v);
#ifdef KNOWLEDGE
        int possible_moves_size();
        DataGo get_possible_moves_by_index(int i);
        void get_atari_escape_moves(std::vector<DataGo>& v);
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
