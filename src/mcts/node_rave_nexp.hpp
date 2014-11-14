#ifndef __NODE_RAVE_NEXP__
#define __NODE_RAVE_NEXP__

#include <vector>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <functional>
#include <iostream>
#include <set>

template <class Value,class Data> class NodeUCTRaveNExp{
    private:
        void count_nodes(int level,long counter[],int max_level);
        NodeUCTRaveNExp(Value init_value,Data init_data,NodeUCTRaveNExp<Value,Data> *father);
        struct NodeComp
        {
          bool operator()(const NodeUCTRaveNExp<Value,Data>* lhs, const NodeUCTRaveNExp<Value,Data>* rhs) const 
          {
            if(lhs->rate==rhs->rate)
              return (lhs->data)>(rhs->data);
            else 
              return lhs->rate>rhs->rate;
          };
        };
    public:
        static double k_rave;
        static double sqrt_k_rave;
        unsigned long visits;
        unsigned long amaf_visits;
        double sqrt_visits;
        double sqrt_for_amaf;
        Value rate;
        Value value;
        Value amaf_value;
        Data data;
        std::set<NodeUCTRaveNExp<Value,Data>* ,NodeComp> children;
        NodeUCTRaveNExp<Value,Data> *parent;
        NodeUCTRaveNExp(Value init_value,Data init_data);
        void create_child(Value init_value,Data init_data);
        void delete_tree();
        NodeUCTRaveNExp<Value,Data> *move_root_to_child(Data child_data);
        void set_visits(unsigned long v);
        void update_rate();
        void join(NodeUCTRaveNExp<Value,Data>* nod);
        void show();
};

template <class Value,class Data>
double NodeUCTRaveNExp<Value,Data>::k_rave;

template <class Value,class Data>
double NodeUCTRaveNExp<Value,Data>::sqrt_k_rave;

template <class Value,class Data>
NodeUCTRaveNExp<Value,Data>::NodeUCTRaveNExp(Value init_value,Data init_data) : 
        rate(2),value(init_value),data(init_data),parent(NULL),visits(0),sqrt_visits(0),amaf_visits(0),amaf_value(0),sqrt_for_amaf(0)
{}

template <class Value,class Data>
NodeUCTRaveNExp<Value,Data>::NodeUCTRaveNExp(Value init_value,Data init_data,NodeUCTRaveNExp<Value,Data> *father) : 
        rate(2),value(init_value),data(init_data),parent(father),visits(0),sqrt_visits(0),amaf_visits(0),amaf_value(0),sqrt_for_amaf(0)
{}

template <class Value,class Data>
inline void NodeUCTRaveNExp<Value,Data>::set_visits(unsigned long v)
{
    visits = v;
    sqrt_visits = sqrt((double) v);
    sqrt_for_amaf = sqrt(3 * ((double) v)+k_rave);
}

template <class Value,class Data>
inline void NodeUCTRaveNExp<Value,Data>::update_rate()
{
    if(visits==0)
      rate=2;
    else
      if(amaf_visits==0)
        rate= double(value) / double(visits);
      else{
        double amaf_coeff = sqrt_k_rave / sqrt_for_amaf;
        rate= (double(value) / double(visits)) * (1-amaf_coeff) +
              (double(amaf_value) / double(amaf_visits)) * amaf_coeff;
      }
}

template <class Value,class Data>
inline void NodeUCTRaveNExp<Value,Data>::join(NodeUCTRaveNExp<Value,Data>* nod)
{
    value+=nod->value;
    amaf_value+=nod->amaf_value;
    amaf_visits+=nod->amaf_visits;
    set_visits(visits+nod->visits);
}

template <class Value,class Data>
inline void NodeUCTRaveNExp<Value,Data>::create_child(Value init_value,Data init_data)
{
    NodeUCTRaveNExp<Value,Data> *n= new NodeUCTRaveNExp<Value,Data>(init_value,init_data,this);
    children.insert(n);
}

template <class Value,class Data>
void NodeUCTRaveNExp<Value,Data>::delete_tree(){
    std::for_each(children.begin(), children.end(), std::mem_fun(&NodeUCTRaveNExp<Value,Data>::delete_tree));
    delete this;
}

template <class Value,class Data>
NodeUCTRaveNExp<Value,Data> *NodeUCTRaveNExp<Value,Data>::move_root_to_child(Data child_data){
    NodeUCTRaveNExp<Value,Data> *child=NULL;
    for(auto it=children.begin();it!=children.end();++it)
        if((*it)->data == child_data && !child)
            child = (*it);
        else
            (*it)->delete_tree();
    children.clear();
    if(child)
      child->parent=NULL;
    return child;
}

template <class Value,class Data>
void NodeUCTRaveNExp<Value,Data>::count_nodes(int level,long counter[],int max_level){
    if(level >= max_level)
        return;
    counter[level]+=children.size();
    for(auto it=children.begin();it!=children.end();++it)
        (*it)->count_nodes(level+1,counter,max_level);
}

template <class Value,class Data>
void NodeUCTRaveNExp<Value,Data>::show(){
    long size[20],total=0;
    for(int i=0;i<20;i++)
        size[i]=0;
    count_nodes(0,size,20);
    for(int i=0;i<20;i++)
        total+=size[i];
    std::cout<<"==Show node=="<<std::endl;
    std::cout<<"TOTAL: "<<total<<std::endl;
    for(int k=0;k<20 && size[k]!=0;k++)
        std::cout<<"Level "<<k<<": "<<size[k]<<std::endl;
    std::cout<<"============="<<std::endl;
}

#endif //__NODE_RAVE_NEXP__
