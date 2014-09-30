
#include "node.hpp"
#include "mcts.hpp"
#include <cstddef>
#include <cmath>
#include <cassert>
#include <climits>

template <class Value,class Data>
class SelectionUCT: public Selection<Value,Data, NodeUCT<Value,Data> >{
    private:
        double _coeff;
        double get_uct_val(const NodeUCT<Value,Data> *nod,double log_parent);
    public:
        SelectionUCT(double coeff);
        NodeUCT<Value,Data>* select(const NodeUCT<Value,Data> *nod);
};

template <class Value,class Data,class EvalNode>
class RetropropagationSimple: public Retropropagation<Value,Data, NodeUCT<Value,Data> > {
    private:
        EvalNode _eval_fun;
    public:
        RetropropagationSimple(EvalNode fun);
        void retro(NodeUCT<Value,Data> *node,Value value);
};

////////////////////////////////////////////////////////////////////

template <class Value,class Data>
SelectionUCT<Value,Data>::SelectionUCT(double coeff) : _coeff(coeff)
{}

template <class Value,class Data>
inline double SelectionUCT<Value,Data>::get_uct_val(const NodeUCT<Value,Data> *nod, double sqrt_log_parent)
{
    if(nod->visits==0)
        return -1;
    return (((double) nod->value) / (double) nod->visits) +
             _coeff * sqrt_log_parent / nod->sqrt_visits;
}

template <class Value,class Data>
NodeUCT<Value,Data>* SelectionUCT<Value,Data>::select(const NodeUCT<Value,Data> *nod)
{
    //TODO: Make the selection of no visited nodes randomly!
    if(nod->children.empty())
        return NULL;
    NodeUCT<Value,Data> *max_nod= nod->children[0];
    double max_val,val,sqrt_log_parent = sqrt(log((double) nod->visits));
    if((max_val = get_uct_val(nod->children[0],sqrt_log_parent))==-1)
        return max_nod;
    for(int i=1;i<nod->children.size();i++){
        if((val=get_uct_val(nod->children[i],sqrt_log_parent))==-1)
            return nod->children[i];
        if(val>max_val){
            max_val = val;
            max_nod = nod->children[i];
        }
    }
    return max_nod;
}

template <class Value,class Data,class EvalNode>
RetropropagationSimple<Value,Data,EvalNode>::RetropropagationSimple(EvalNode fun) : _eval_fun(fun)
{}

template <class Value,class Data,class EvalNode>
void RetropropagationSimple<Value,Data,EvalNode>::retro(NodeUCT<Value,Data> *node,
                                   Value value)
{
    do{
        node->value=_eval_fun(node->value,value,node->data);
        assert(node->visits<ULONG_MAX);
        node->set_visits(node->visits+1);
    }while((node=node->parent)!=NULL);
}

