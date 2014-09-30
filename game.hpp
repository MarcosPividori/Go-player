
#include "state_go.hpp"
#include "mcts_utils.hpp"
#include "mcts_uct.hpp"

struct EvalNode{
    ValGo operator()(ValGo v_nodo,ValGo v_final,DataGo dat_nodo)
    {
        if(v_final == dat_nodo.player)
            return v_nodo+1;
        return v_nodo;
    }
};

class Game{
    private:
        StateGo *_state;
        float _komi;
        int _size;
        NodeUCT<ValGo,DataGo> *_root;
        SelectionUCT<ValGo,DataGo> _sel;
        ExpansionAllChildren<ValGo,DataGo,StateGo,NodeUCT<ValGo,DataGo> > _exp;
        SimulationTotallyRandom<ValGo,DataGo,StateGo> _sim;
        RetropropagationSimple<ValGo,DataGo,EvalNode> _ret;
        SelectResMostRobust<ValGo,DataGo> _sel_res;
        Mcts<ValGo,DataGo,NodeUCT<ValGo,DataGo>,StateGo> _m;
    public:
        Game(int size);
        ~Game();
        void set_boardsize(int size);
        int get_boardsize(){return _size;}
        void clear_board();
        void set_komi(float komi);
        float get_final_score();
        bool play_move(DataGo pos);
        DataGo gen_move(Player p);
        void show_board(FILE *output);
#ifdef DEBUG
        void debug();
#endif
};

