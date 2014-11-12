
#include "avl_tree.hpp"
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <cmath>

int main()
{
    AVLTree<int> *tree= new AVLTree<int>();

    std::srand(time(NULL));

    for(long int i=0;i<1000000;i++){
        if(std::rand()%2)
          tree->insert(std::rand()%10000);
        else
          tree->remove(std::rand()%10000);
        assert(tree->height()-1 <= 2*(log(float(tree->size())+1)/log(2)));
    }

    std::cout<<"ORDER:"<<std::endl;
    for(int i=0;i<tree->size();i++)
        std::cout<<(*tree)[i]<<std::endl;

    std::cout<<"FINAL HEIGHT: "<<tree->height()<<std::endl;
    delete tree;
}
