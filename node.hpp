
#include <vector>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <functional>
#include <iostream>

template <class Value,class Data> class NodeUCT{
    private:
        void count_nodes(int level,long counter[],int max_level);
    public:
        unsigned long visits;
        double sqrt_visits;
        Value value;
        Data data;
        std::vector< NodeUCT<Value,Data>* > children;
        NodeUCT<Value,Data> *parent;
        NodeUCT(Value init_value,Data init_data,NodeUCT<Value,Data> *father);
        void create_child(Value init_value,Data init_data);
        void delete_tree();
        NodeUCT<Value,Data> *move_root_to_child(Data child_data);
        void set_visits(unsigned long v);
        void show();
};

template <class Value,class Data>
NodeUCT<Value,Data>::NodeUCT(Value init_value,Data init_data,NodeUCT<Value,Data> *father) : 
        value(init_value),data(init_data),parent(father),visits(0),sqrt_visits(0)
{}

template <class Value,class Data>
inline void NodeUCT<Value,Data>::set_visits(unsigned long v)
{
    visits = v;
    sqrt_visits = sqrt((double) v);
}

template <class Value,class Data>
inline void NodeUCT<Value,Data>::create_child(Value init_value,Data init_data)
{
    NodeUCT<Value,Data> *n= new NodeUCT<Value,Data>(init_value,init_data,this);
    children.push_back(n);
}

template <class Value,class Data>
void NodeUCT<Value,Data>::delete_tree(){
    std::for_each(children.begin(), children.end(), std::mem_fun(&NodeUCT<Value,Data>::delete_tree));
    delete this;
}

template <class Value,class Data>
NodeUCT<Value,Data> *NodeUCT<Value,Data>::move_root_to_child(Data child_data){
    NodeUCT<Value,Data> *child=NULL;
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
void NodeUCT<Value,Data>::count_nodes(int level,long counter[],int max_level){
    if(level >= max_level)
        return;
    counter[level]+=children.size();
    for(int n=0;n<children.size();n++)
        children[n]->count_nodes(level+1,counter,max_level);
}

template <class Value,class Data>
void NodeUCT<Value,Data>::show(){
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

