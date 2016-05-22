#ifndef __STATES__
#define __STATES__

#include <vector>
// Unused class to represent minimal necessary interface for the
// State class template parameter.
template <class Value,class Data>
class States{
    public:
        ~States()=0;
        States States(States *src)=0;
        void get_possible_moves(std::vector<Data>& v)=0;
        void apply(Data)=0;
        Value get_final_value()=0;
        void show()=0;
};

template <class Value,class Data>
States<Value,Data>::~States(){}

#endif // __STATES__
