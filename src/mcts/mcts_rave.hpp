
#include "mcts.hpp"
#include "node_rave.hpp"
#include <cstddef>
#include <cmath>
#include <cassert>
#include <climits>

using namespace std;

template <class Value,class Data>
class SelectionUCTRave: public Selection<NodeUCTRave<Value,Data> >{
    private:
        const double _coeff;
        const double _amaf_coeff;
#ifdef DEBUG
    public:
#endif
        double get_uct_amaf_val(const NodeUCTRave<Value,Data> *nod,double log_parent);
    public:
        SelectionUCTRave(double coeff,double amaf_coeff);
        NodeUCTRave<Value,Data>* select(const NodeUCTRave<Value,Data> *nod);
};

template <class Data>
class MoveRecorder{
    public:
        virtual void clear()=0;
        virtual void postMove(Data move)=0;
        virtual void prevMove(Data move)=0; 
        virtual bool isMove(Data move)=0;
};

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
class SimulationAndRetropropagationRave: public Simulation<Value,State>,
    public Retropropagation<Value, NodeUCTRave<Value,Data> >
{
    protected:
        mt19937 mt;
        MoveRecorderT recorder;
        EvalNode _eval_fun;
    public:
        SimulationAndRetropropagationRave();
        virtual ~SimulationAndRetropropagationRave(){};
        virtual Value simulate(State *state);
        void retro(NodeUCTRave<Value,Data> *node,Value value);
};

////////////////////////////////////////////////////////////////////

template <class Value,class Data>
SelectionUCTRave<Value,Data>::SelectionUCTRave(double coeff, double amaf_coeff) :
    _coeff(coeff),_amaf_coeff(sqrt(amaf_coeff))
{}

template <class Value,class Data>
inline double SelectionUCTRave<Value,Data>::get_uct_amaf_val(
    const NodeUCTRave<Value,Data> *nod,
    double sqrt_log_parent)
{
    if(nod->get_visits()==0)
        return -1;
    if(nod->get_amaf_visits()==0)
        return (((double) nod->value) / (double) nod->get_visits()) +
                 _coeff * sqrt_log_parent / nod->get_sqrt_visits();
    double amaf_coeff = _amaf_coeff / nod->get_sqrt_for_amaf();
    return (((double) nod->value) / (double) nod->get_visits()) * (1-amaf_coeff) +
           (((double) nod->amaf_value) / (double) nod->get_amaf_visits()) * amaf_coeff +
             _coeff * sqrt_log_parent / nod->get_sqrt_visits();
}

template <class Value,class Data>
NodeUCTRave<Value,Data>* SelectionUCTRave<Value,Data>::select(
    const NodeUCTRave<Value,Data> *nod)
{
    typename NodeUCTRave<Value,Data>::const_iterator iter=nod->children_begin();
    if(iter==nod->children_end())
        return NULL;
    NodeUCTRave<Value,Data> *max_nod= *iter;
    double max_val,val,sqrt_log_parent = sqrt(log((double) (nod->get_visits())));
    if((max_val = get_uct_amaf_val(*iter,sqrt_log_parent))==-1)
        return max_nod;
    for(iter++;iter != nod->children_end();iter++){
        if((val=get_uct_amaf_val(*iter,sqrt_log_parent))==-1)
            return *iter;
        if(val>max_val){
            max_val = val;
            max_nod = *iter;
        }
    }
    return max_nod;
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>
::SimulationAndRetropropagationRave()
{
    random_device rd;
    mt.seed(rd());
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
Value SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>
::simulate(State *state)
{
    recorder.clear();
    vector<Data> v;
    Data mov;
    state->get_possible_moves(v);
    while(!v.empty()){
        uniform_int_distribution<int> dist(0, v.size()-1);
        mov = v[dist(mt)];
        state->apply(mov);
        recorder.postMove(mov);
        v.clear();
        state->get_possible_moves(v);
    }
    return state->get_final_value();
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
void SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>
::retro(NodeUCTRave<Value,Data> *node,
        Value value)
{
    do{
        node->value=_eval_fun(node->value,value,node->data);
        assert(node->get_visits()<ULONG_MAX);
        node->set_visits(node->get_visits()+1);
        NodeUCTRave<Value,Data>* nod;
        typename NodeUCTRave<Value,Data>::const_iterator iter=node->children_begin();
        for(;iter != node->children_end();iter++){
            nod=*iter;
            if(recorder.isMove(nod->data)){
                assert(nod->get_amaf_visits()<ULONG_MAX);
                nod->amaf_value=_eval_fun(nod->amaf_value,value,nod->data);
                nod->set_amaf_visits(nod->get_amaf_visits()+1);
            }
        }
        recorder.prevMove(node->data);
    }while(node=node->get_parent());
}

