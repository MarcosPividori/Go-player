
#include "state_go.hpp"
#include "mcts_utils.hpp"
#include "config.hpp"

#define NUM_THREADS 5

#ifdef RAVE
 #ifdef KNOWLEDGE
  #include "mcts_go.hpp"
 #else
  #include "mcts_rave.hpp"
 #endif
 #include "moverecorder_go.hpp"
 typedef NodeUCTRave<ValGo,DataGo> Nod;
#else
 #include "mcts_uct.hpp"
 typedef NodeUCT<ValGo,DataGo> Nod;
#endif

struct EvalNod : EvalNode<ValGo,DataGo> {
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
        Nod *_root;
        Config _cfg;
        std::mutex _mutex;
        PatternList *_patterns;
        ExpansionAllChildren<ValGo,DataGo,StateGo,Nod> _exp;
        SelectResMostRobustOverLimit<Nod> _sel_res;
        Mcts<ValGo,DataGo,Nod,StateGo> **_m;
#ifdef RAVE
        SelectionUCTRave<ValGo,DataGo> _sel;
        SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo> **_sim_and_retro;
#else
        SelectionUCT<ValGo,DataGo> _sel;
        SimulationTotallyRandom<ValGo,DataGo,StateGo> _sim;
        RetropropagationSimple<ValGo,DataGo,EvalNod> _ret;
#endif
    public:
        Game(int size,Config &cfg_input);
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
        void match_patterns();
#endif
};

