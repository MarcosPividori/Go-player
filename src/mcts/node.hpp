#ifndef __NODE__
#define __NODE__

#include <vector>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <functional>
#include <iostream>

using namespace std;

template <class Value,class Data> class NodeUCT{
    protected:
        NodeUCT<Value,Data> *parent;
    private:
        vector< NodeUCT<Value,Data>* > children;
        unsigned long visits;
        double sqrt_visits;
        void count_nodes(int level,long counter[],int max_level) const;
        NodeUCT(Value init_value,Data init_data,NodeUCT<Value,Data> *father);
    public:
        typedef typename vector< NodeUCT<Value,Data>* >::const_iterator const_iterator;
        Value value;
        Data data;
        NodeUCT(Value init_value,Data init_data);
        const_iterator children_begin() const {return children.cbegin();};
        const_iterator children_end() const {return children.cend();};
        void set_visits(unsigned long v);
        unsigned long get_visits() const {return visits;};
        double get_sqrt_visits() const {return sqrt_visits;};
        NodeUCT<Value,Data> *get_parent() const {return parent;};
        NodeUCT<Value,Data> *move_root_to_child(Data child_data);
        void create_child(Value init_value,Data init_data);
        void delete_tree();
        void join(NodeUCT<Value,Data>* nod);
        void show() const;
        void debug() const;
};

template <class Value,class Data>
NodeUCT<Value,Data>::NodeUCT(Value init_value,Data init_data) : 
        value(init_value),data(init_data),parent(NULL),visits(0),sqrt_visits(0)
{}

template <class Value,class Data>
NodeUCT<Value,Data>::NodeUCT(Value init_value,
                             Data init_data,
                             NodeUCT<Value,Data> *father) : 
    value(init_value),data(init_data),parent(father),visits(0),sqrt_visits(0)
{}

template <class Value,class Data>
inline void NodeUCT<Value,Data>::set_visits(unsigned long v)
{
    visits = v;
    sqrt_visits = sqrt((double) v);
}

template <class Value,class Data>
inline void NodeUCT<Value,Data>::join(NodeUCT<Value,Data>* nod)
{
    value+=nod->value;
    set_visits(visits+nod->visits);
}

template <class Value,class Data>
inline void NodeUCT<Value,Data>::create_child(Value init_value,Data init_data)
{
    NodeUCT<Value,Data> *n= new NodeUCT<Value,Data>(init_value,init_data,this);
    children.push_back(n);
}

template <class Value,class Data>
void NodeUCT<Value,Data>::delete_tree(){
    for_each(children.begin(), children.end(),
        mem_fun(&NodeUCT<Value,Data>::delete_tree));
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
void NodeUCT<Value,Data>::count_nodes(int level,long counter[],int max_level) const {
    if(level >= max_level)
        return;
    counter[level]+=children.size();
    for(int n=0;n<children.size();n++)
        children[n]->count_nodes(level+1,counter,max_level);
}

template <class Value,class Data>
void NodeUCT<Value,Data>::show() const {
    long size[20],total=0;
    for(int i=0;i<20;i++)
        size[i]=0;
    count_nodes(0,size,20);
    for(int i=0;i<20;i++)
        total+=size[i];
    cout<<"==Show node=="<<endl;
    cout<<"TOTAL: "<<total<<endl;
    for(int k=0;k<20 && size[k]!=0;k++)
        cout<<"Level "<<k<<": "<<size[k]<<endl;
    cout<<"============="<<endl;
}

template <class Value,class Data>
void NodeUCT<Value,Data>::debug() const {
    cout << "DEBUG STATS NODE: vis=" << visits << " win=" << value << endl;
    cout << "                  rate=" << (value / visits) << endl;
}

#endif //__NODE__
