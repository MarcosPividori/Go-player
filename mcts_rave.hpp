
#include "mcts.hpp"
#include "node_rave.hpp"
#include <cstddef>
#include <cmath>
#include <cassert>
#include <climits>

template <class Value,class Data>
class SelectionUCTRave: public Selection<Value,Data,NodeUCTRave<Value,Data> >{
    private:
        const double _coeff;
        const double _amaf_coeff;
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
class SimulationAndRetropropagationRave: public Simulation<Value,Data,State>,
                                         public Retropropagation<Value,Data, NodeUCTRave<Value,Data> >
{
    protected:
        std::mt19937 mt;
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
SelectionUCTRave<Value,Data>::SelectionUCTRave(double coeff, double amaf_coeff) : _coeff(coeff),_amaf_coeff(sqrt(amaf_coeff))
{}

template <class Value,class Data>
inline double SelectionUCTRave<Value,Data>::get_uct_amaf_val(const NodeUCTRave<Value,Data> *nod, double sqrt_log_parent)
{
    if(nod->visits==0)
        return -1;
    if(nod->amaf_visits==0)
        return (((double) nod->value) / (double) nod->visits) +
                 _coeff * sqrt_log_parent / nod->sqrt_visits;
    double amaf_coeff = _amaf_coeff / nod->sqrt_for_amaf;
    return (((double) nod->value) / (double) nod->visits) * (1-amaf_coeff) +
           (((double) nod->amaf_value) / (double) nod->amaf_visits) * amaf_coeff +
             _coeff * sqrt_log_parent / nod->sqrt_visits;
}

template <class Value,class Data>
NodeUCTRave<Value,Data>* SelectionUCTRave<Value,Data>::select(const NodeUCTRave<Value,Data> *nod)
{
    //TODO: Make the selection of no visited nodes randomly!
    if(nod->children.empty())
        return NULL;
    NodeUCTRave<Value,Data> *max_nod= nod->children[0];
    double max_val,val,sqrt_log_parent = sqrt(log((double) nod->visits));
    if((max_val = get_uct_amaf_val(nod->children[0],sqrt_log_parent))==-1)
        return max_nod;
    for(int i=1;i<nod->children.size();i++){
        if((val=get_uct_amaf_val(nod->children[i],sqrt_log_parent))==-1)
            return nod->children[i];
        if(val>max_val){
            max_val = val;
            max_nod = nod->children[i];
        }
    }
    return max_nod;
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::SimulationAndRetropropagationRave()
{
    std::random_device rd;
    mt.seed(rd());
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
Value SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::simulate(State *state)
{
    recorder.clear();
    std::vector<Data> v;
    Data mov;
    state->get_possible_moves(v);
    while(!v.empty()){
        std::uniform_int_distribution<int> dist(0, v.size()-1);
        mov = v[dist(mt)];
        state->apply(mov);
        recorder.postMove(mov);
        v.clear();
        state->get_possible_moves(v);
    }
    return state->get_final_value();
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
void SimulationAndRetropropagationRave<Value,Data,State,EvalNode,MoveRecorderT>::retro(NodeUCTRave<Value,Data> *node,
                                   Value value)
{
    do{
        node->value=_eval_fun(node->value,value,node->data);
        assert(node->visits<ULONG_MAX);
        node->set_visits(node->visits+1);
        NodeUCTRave<Value,Data>* nod;
        for(int i=0;i<node->children.size();i++){
            nod=node->children[i];
            if(recorder.isMove(nod->data)){
                assert(nod->amaf_visits<ULONG_MAX);
                nod->amaf_value=_eval_fun(nod->amaf_value,value,nod->data);
                nod->amaf_visits+=1;
            }
        }
        recorder.prevMove(node->data);
    }while((node=node->parent)!=NULL);
}

