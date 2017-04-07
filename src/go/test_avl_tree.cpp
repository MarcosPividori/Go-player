#include "avl_tree.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <time.h>

using namespace std;

int main() {
  AVLTree<int>* tree = new AVLTree < int >();

  srand(time(NULL));

  for (long int i = 0; i < 1000000; i++) {
    if (rand() % 2)
      tree->insert(rand() % 10000);
    else
      tree->remove(rand() % 10000);
    assert(tree->height() - 1 <= 2 * (log(float (tree->size()) + 1) / log(2)));
  }

  cout << "ORDER:" << endl;
  for (int i = 0; i < tree->size(); i++)
    cout << (*tree)[i] << endl;

  cout << "FINAL HEIGHT: " << tree->height() << endl;
  delete tree;
}
