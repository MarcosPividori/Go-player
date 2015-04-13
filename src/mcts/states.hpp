#ifndef __STATES__
#define __STATES__

#include <vector>
//Unused class to represent minimal necessary interface for the State class template parameter.
template <class Value,class Data>
class States{
    public:
        virtual ~States()=0;
        virtual States *copy()=0;
        virtual void get_possible_moves(std::vector<Data>& v)=0;
        virtual void apply(Data)=0;
        virtual Value get_final_value()=0;
#ifdef DEBUG
        virtual void show()=0;
#endif
};

template <class Value,class Data>
States<Value,Data>::~States(){}

#endif // __STATES__
