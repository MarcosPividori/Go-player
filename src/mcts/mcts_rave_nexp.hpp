
#include "mcts.hpp"
#include "node_rave_nexp.hpp"
#include "mcts_rave.hpp"
#include <cstddef>
#include <cmath>
#include <cassert>
#include <climits>

template <class Value,class Data>
class SelectionUCTRaveNExp: public Selection<Value,Data,NodeUCTRaveNExp<Value,Data> >{
    public:
        NodeUCTRaveNExp<Value,Data>* select(const NodeUCTRaveNExp<Value,Data> *nod);
};

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
class SimulationAndRetropropagationRaveNExp: public Simulation<Value,Data,State>,
                                             public Retropropagation<Value,Data, NodeUCTRaveNExp<Value,Data> >
{
    protected:
        std::mt19937 mt;
        MoveRecorderT recorder;
        EvalNode _eval_fun;
    public:
        SimulationAndRetropropagationRaveNExp();
        virtual ~SimulationAndRetropropagationRaveNExp(){};
        virtual Value simulate(State *state);
        void retro(NodeUCTRaveNExp<Value,Data> *node,Value value);
};

////////////////////////////////////////////////////////////////////

template <class Value,class Data>
NodeUCTRaveNExp<Value,Data>* SelectionUCTRaveNExp<Value,Data>::select(const NodeUCTRaveNExp<Value,Data> *nod)
{
    if(nod->children.empty())
        return NULL;
    return *(nod->children.begin());
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>::SimulationAndRetropropagationRaveNExp()
{
    std::random_device rd;
    mt.seed(rd());
}

template <class Value,class Data,class State,class EvalNode,class MoveRecorderT>
Value SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>::simulate(State *state)
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
void SimulationAndRetropropagationRaveNExp<Value,Data,State,EvalNode,MoveRecorderT>::retro(NodeUCTRaveNExp<Value,Data> *node,
                                   Value value)
{
    do{
        node->value=_eval_fun(node->value,value,node->data);
        assert(node->visits<ULONG_MAX);
        if(node->parent)
          node->parent->children.erase(node);
        node->set_visits(node->visits+1);
        node->update_rate();
        if(node->parent)
          node->parent->children.insert(node);
        NodeUCTRaveNExp<Value,Data>* nod;
        std::vector<NodeUCTRaveNExp<Value,Data>*> changed;
        for(auto it=node->children.begin();it!=node->children.end();){
            nod=*it;
            if(recorder.isMove(nod->data)){
                assert(nod->amaf_visits<ULONG_MAX);
                node->children.erase(it++);
                changed.push_back(nod);
                nod->amaf_value=_eval_fun(nod->amaf_value,value,nod->data);
                nod->amaf_visits+=1;
                nod->update_rate();
            }
            else
              ++it;
        }
        for(int i=0;i<changed.size();i++)
            node->children.insert(changed[i]);
        recorder.prevMove(node->data);
    }while((node=node->parent)!=NULL);
}

