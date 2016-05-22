
#include "state_go.hpp"
#include "mcts_utils.hpp"
#include "mcts_uct.hpp"
#include "mcts_parallel.hpp"
#include "config.hpp"
#include "mcts_go.hpp"

using namespace std;

class Game{
    private:
        StateGo *_state;
        float _komi;
        int _size;
        Config _cfg;
        PatternList *_patterns;
        MctsParallel<ValGo,DataGo,StateGo> *_mcts;
    public:
        Game(int size,Config &cfg_input);
        ~Game();
        void set_boardsize(int size);
        int get_boardsize(){return _size;}
        void clear_board();
        void set_komi(float komi);
        float get_final_score() const;
        bool play_move(DataGo pos);
        DataGo gen_move(Player p);
        void show_board(FILE *output) const;
#ifdef DEBUG
        void debug() const;
        void match_patterns() const;
#endif
};

class RaveEnv : public MctsParallel<ValGo,DataGo,StateGo>
{
    private:
        typedef NodeUCTRave<ValGo,DataGo> Nod;
        Selection<Nod> *_sel;
        Expansion<Nod,StateGo> *_exp;
        SimulationAndRetropropagationRave<ValGo,DataGo,StateGo,EvalNod,MoveRecorderGo> **_sim_and_retro;
        Retropropagation<ValGo,Nod> **_ret;
        SelectRes<DataGo,Nod> *_sel_res;
        vector<Mcts<ValGo,DataGo,Nod,StateGo> *> _m;
        MctsParallel<ValGo,DataGo,StateGo> *_mcts;
    public:
        RaveEnv(Config &cfg_input,StateGo *state);
        ~RaveEnv();
        void run_time(double time_limit) {_mcts->run_time(time_limit);};
        void run_cycles(unsigned long cycles_limit) {_mcts->run_cycles(cycles_limit);};
        DataGo get_resultant_move() {return _mcts->get_resultant_move();};
        void apply_move(DataGo mov) {_mcts->apply_move(mov);};
        void reinit(StateGo *init_state,DataGo init_data) {_mcts->reinit(init_state,init_data);};
#ifdef DEBUG
        void *get_root() {_mcts->get_root();};
#endif
};

class UCTEnv : public MctsParallel<ValGo,DataGo,StateGo>
{
    private:
        typedef NodeUCT<ValGo,DataGo> Nod;
        Selection<Nod> *_sel;
        Expansion<Nod,StateGo> *_exp;
        Simulation<ValGo,StateGo> **_sim;
        Retropropagation<ValGo,Nod> *_ret;
        SelectRes<DataGo,Nod> *_sel_res;
        vector<Mcts<ValGo,DataGo,Nod,StateGo> *> _m;
        MctsParallel<ValGo,DataGo,StateGo> *_mcts;
    public:
        UCTEnv(Config &cfg_input,StateGo *state);
        ~UCTEnv();
        void run_time(double time_limit) {_mcts->run_time(time_limit);};
        void run_cycles(unsigned long cycles_limit) {_mcts->run_cycles(cycles_limit);};
        DataGo get_resultant_move() {return _mcts->get_resultant_move();};
        void apply_move(DataGo mov) {_mcts->apply_move(mov);};
        void reinit(StateGo *init_state,DataGo init_data) {_mcts->reinit(init_state,init_data);};
#ifdef DEBUG
        void *get_root() {_mcts->get_root();};
#endif
};

