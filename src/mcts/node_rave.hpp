#ifndef __NODE_RAVE__
#define __NODE_RAVE__

#include <vector>
#include <cstddef>
#include <cassert>
#include <algorithm>
#include <functional>
#include <iostream>

using namespace std;

template <class Value,class Data> class NodeUCTRave{
    protected:
        NodeUCTRave<Value,Data> *parent;
    private:
        vector< NodeUCTRave<Value,Data>* > children;
        unsigned long visits;
        double sqrt_visits;
        unsigned long amaf_visits;
        double sqrt_for_amaf;
        void count_nodes(int level,long counter[],int max_level);
        NodeUCTRave(Value init_value,Data init_data,NodeUCTRave<Value,Data> *father);
    public:
        typedef typename vector< NodeUCTRave<Value,Data>* >::const_iterator const_iterator;
        static double k_rave;
        Value value;
        Value amaf_value;
        Data data;
        NodeUCTRave(Value init_value,Data init_data);
        const_iterator children_begin() const {return children.cbegin();};
        const_iterator children_end() const {return children.cend();};
        void set_visits(unsigned long v);
        unsigned long get_visits() const {return visits;};
        double get_sqrt_visits() const {return sqrt_visits;};
        void set_amaf_visits(unsigned long v) {amaf_visits = v;};
        unsigned long get_amaf_visits() const {return amaf_visits;};
        double get_sqrt_for_amaf() const {return sqrt_for_amaf;};
        NodeUCTRave<Value,Data> *get_parent() const {return parent;};
        NodeUCTRave<Value,Data> *move_root_to_child(Data child_data);
        void create_child(Value init_value,Data init_data);
        void delete_tree();
        void join(NodeUCTRave<Value,Data>* nod);
        void show();
        void debug();
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
    for_each(children.begin(), children.end(), mem_fun(&NodeUCTRave<Value,Data>::delete_tree));
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
    cout<<"==Show node=="<<endl;
    cout<<"TOTAL: "<<total<<endl;
    for(int k=0;k<20 && size[k]!=0;k++)
        cout<<"Level "<<k<<": "<<size[k]<<endl;
    cout<<"============="<<endl;
}

template <class Value,class Data>
void NodeUCTRave<Value,Data>::debug(){
    cout << "DEBUG STATS NODE: vis=" << visits << " win=" << value<< " vis_amaf=" << amaf_visits << " win_amaf=" << amaf_value << endl;
    cout << "                  rate=" << (value / visits) << "  amaf_rate=" << (amaf_value / amaf_visits) << "  amaf_coeff="<< (sqrt(k_rave)/sqrt_for_amaf) << endl;
}

#endif //__NODE_RAVE__
