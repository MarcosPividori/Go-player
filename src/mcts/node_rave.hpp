#ifndef __NODE_RAVE__
#define __NODE_RAVE__

#include <vector>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <functional>
#include <iostream>

template <class Value,class Data> class NodeUCTRave{
    private:
        void count_nodes(int level,long counter[],int max_level);
        NodeUCTRave(Value init_value,Data init_data,NodeUCTRave<Value,Data> *father);
    public:
        static double k_rave;
        unsigned long visits;
        unsigned long amaf_visits;
        double sqrt_visits;
        double sqrt_for_amaf;
        Value value;
        Value amaf_value;
        Data data;
        std::vector< NodeUCTRave<Value,Data>* > children;
        NodeUCTRave<Value,Data> *parent;
        NodeUCTRave(Value init_value,Data init_data);
        void create_child(Value init_value,Data init_data);
        void delete_tree();
        NodeUCTRave<Value,Data> *move_root_to_child(Data child_data);
        void set_visits(unsigned long v);
        void join(NodeUCTRave<Value,Data>* nod);
        void show();
};

template <class Value,class Data>
double NodeUCTRave<Value,Data>::k_rave;

template <class Value,class Data>
NodeUCTRave<Value,Data>::NodeUCTRave(Value init_value,Data init_data) : 
        value(init_value),data(init_data),parent(NULL),visits(0),sqrt_visits(0),amaf_visits(0),amaf_value(0),sqrt_for_amaf(0)
{}

template <class Value,class Data>
NodeUCTRave<Value,Data>::NodeUCTRave(Value init_value,Data init_data,NodeUCTRave<Value,Data> *father) : 
        value(init_value),data(init_data),parent(father),visits(0),sqrt_visits(0),amaf_visits(0),amaf_value(0),sqrt_for_amaf(0)
{}

template <class Value,class Data>
inline void NodeUCTRave<Value,Data>::set_visits(unsigned long v)
{
    visits = v;
    sqrt_visits = sqrt((double) v);
    sqrt_for_amaf = sqrt(3 * ((double) v)+k_rave);
}

template <class Value,class Data>
inline void NodeUCTRave<Value,Data>::join(NodeUCTRave<Value,Data>* nod)
{
    value+=nod->value;
    amaf_value+=nod->amaf_value;
    amaf_visits+=nod->amaf_visits;
    set_visits(visits+nod->visits);
}

template <class Value,class Data>
inline void NodeUCTRave<Value,Data>::create_child(Value init_value,Data init_data)
{
    NodeUCTRave<Value,Data> *n= new NodeUCTRave<Value,Data>(init_value,init_data,this);
    children.push_back(n);
}

template <class Value,class Data>
void NodeUCTRave<Value,Data>::delete_tree(){
    std::for_each(children.begin(), children.end(), std::mem_fun(&NodeUCTRave<Value,Data>::delete_tree));
    delete this;
}

template <class Value,class Data>
NodeUCTRave<Value,Data> *NodeUCTRave<Value,Data>::move_root_to_child(Data child_data){
    NodeUCTRave<Value,Data> *child=NULL;
    for(int i=0;i<children.size();i++)
        if(children[i]->data == child_data && !child)
            child = children[i];
        else
            children[i]->delete_tree();
    children.clear();
    if(child)
      child->parent=NULL;
    return child;
}

template <class Value,class Data>
void NodeUCTRave<Value,Data>::count_nodes(int level,long counter[],int max_level){
    if(level >= max_level)
        return;
    counter[level]+=children.size();
    for(int n=0;n<children.size();n++)
        children[n]->count_nodes(level+1,counter,max_level);
}

template <class Value,class Data>
void NodeUCTRave<Value,Data>::show(){
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

#endif //__NODE_RAVE__
