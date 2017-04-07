#ifndef __AVL_TREE_
#define __AVL_TREE_

#include <cstddef>
#include <cassert>

#ifndef MAX
#define MAX(a,b)         ((a)>(b)? a : b)
#endif

template<class T>
class AVLTree {
 private:
  struct Node {
    Node *left;
    Node *right;
    int left_size;
    int h_right;
    int h_left;
    T value;
    Node *insert(T val, bool & flag);
     Node(T val);
    Node *update();
    Node *find_max(T & val);
    Node *remove(T val, bool & flag);
    T get_index(int index) const;
    void delete_tree();
    Node *copy_tree() const;
  };
  Node *_root;
  int _size;
 public:
  void insert(T value);
  bool remove(T value);
  T operator[] (int index) const;
  int size() const {
    return _size;
  } int height() const;
  AVLTree(AVLTree * src);
  AVLTree();
  ~AVLTree();
};

template<class T>
AVLTree<T>::Node::Node(T val) :
  left(NULL), right(NULL), value(val), left_size(0), h_right(0), h_left(0)
{}

template<class T>
inline typename AVLTree<T>::Node * AVLTree<T>::Node::update() {
  switch (h_left - h_right) {
  case -2:
    {
      Node *n;
      if ((right->h_left - right->h_right) == 1) {
        n = right->left;
        right->left = n->right;
        right->h_left = n->h_right;
        right->left_size -= n->left_size + 1;
        n->right = right;
        n->h_right = right->h_right + 1;
        right = n;
      }
      n = right;
      right = n->left;
      h_right = n->h_left;
      n->left = this;
      n->left_size += left_size + 1;
      n->h_left = MAX(h_right, h_left) + 1;
      return n;
    }
  case 2:
    {
      Node *n;
      if ((left->h_left - left->h_right) == -1) {
        n = left->right;
        left->right = n->left;
        left->h_right = n->h_left;
        n->left = left;
        n->h_left = left->h_left + 1;
        n->left_size += left->left_size + 1;
        left = n;
      }
      n = left;
      left = n->right;
      h_left = n->h_right;
      left_size -= n->left_size + 1;
      n->right = this;
      n->h_right = MAX(n->h_right, h_right) + 1;
      return n;
    }
  default:
    return this;
  }
}

template<class T>
typename AVLTree<T>::Node * AVLTree<T>::Node::find_max(T & val) {
  if (right) {
    right = right->find_max(val);
    if (right)
      h_right = MAX(right->h_left, right->h_right) + 1;
    else
      h_right = 0;
    return update();
  } else {
    val = value;
    Node *n = left;
    delete this;
    return n;
  }
};

template<class T>
typename AVLTree<T>::Node * AVLTree<T>::Node::insert(T val, bool & flag) {
  bool f = false;
  flag = false;
  if (val < value)
    if (left) {
      left = left->insert(val, f);
      if (f) {
        flag = true;
        left_size++;
        h_left = MAX(left->h_left, left->h_right) + 1;
        return update();
      } else
        return this;
    } else {
      left = new Node(val);
      left_size++;
      h_left += 1;
      flag = true;
      return this;
    } else
      if (val > value)
        if (right) {
          right = right->insert(val, f);
          if (f) {
            flag = true;
            h_right = MAX(right->h_left, right->h_right) + 1;
            return update();
          } else
            return this;
        } else {
          right = new Node(val);
          h_right += 1;
          flag = true;
          return this;
        }
      else
        return this;
}

template<class T>
typename AVLTree<T>::Node * AVLTree<T>::Node::remove(T val, bool & flag) {
  bool f = false;
  flag = false;
  if (val < value)
    if (left) {
      left = left->remove(val, f);
      if (f) {
        flag = true;
        left_size--;
        if (left)
          h_left = MAX(left->h_left, left->h_right) + 1;
        else
          h_left = 0;
        return update();
      }
      return this;
    }
    else
      return this;
  else if (val > value)
    if (right) {
      right = right->remove(val, f);
      if (f) {
        flag = true;
        if (right)
          h_right = MAX(right->h_left, right->h_right) + 1;
        else
          h_right = 0;
        return update();
      }
      return this;
    } else
      return this;
  else {
    flag = true;
    if (!right) {
      Node *n = left;
      delete this;
      return n;
    } else if (!left) {
      Node *n = right;
      delete this;
      return n;
    } else {
      T val;
      left = left->find_max(val);
      if (left)
        h_left = MAX(left->h_left, left->h_right) + 1;
      else
        h_left = 0;
      left_size--;
      value = val;
      return update();
    }
  }
};

template<class T>
T AVLTree<T>::Node::get_index(int index) const {
  if (index < left_size)
    return left->get_index(index);
  else if (index == left_size)
    return value;
  else
    return right->get_index(index - left_size - 1);
};

template<class T>
void AVLTree<T>::Node::delete_tree() {
  if (right)
    right->delete_tree();
  if (left)
    left->delete_tree();
  delete this;
};

template<class T>
typename AVLTree<T>::Node * AVLTree<T>::Node::copy_tree() const {
  Node *n = new Node(value);
  if (right)
    n->right = right->copy_tree();
  if (left)
    n->left = left->copy_tree();
  n->left_size = left_size;
  n->h_right = h_right;
  n->h_left = h_left;
  return n;
};


template<class T>
AVLTree<T>::AVLTree():_size(0), _root(NULL) {
}

template<class T>
void AVLTree<T>::insert(T value) {
  bool flag = false;
  if (!_root) {
    _root = new Node(value);
    _size++;
  } else {
    _root = _root->insert(value, flag);
    if (flag)
      _size++;
  }
}

template<class T>
bool AVLTree<T>::remove(T value) {
  bool flag = false;
  if (_root) {
    _root = _root->remove(value, flag);
    if (flag)
      _size--;
  }
  return flag;
}

template<class T>
T AVLTree<T>::operator[](int index) const {
  assert(index >= 0 && index < _size);
  return _root->get_index(index);
}

template<class T>
AVLTree<T>::~AVLTree() {
  if (_root)
    _root->delete_tree();
}

template<class T>
int AVLTree<T>::height() const {
  if (!_root)
    return 0;
  return (MAX(_root->h_left, _root->h_right) + 1);
}

template<class T>
AVLTree<T>::AVLTree(AVLTree<T> *src) {
  if (src->_root) {
    _root = src->_root->copy_tree();
    _size = src->_size;
  } else {
    _size = 0;
    _root = NULL;
  }
}

#endif                          // __AVL_TREE_
