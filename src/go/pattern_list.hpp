#ifndef __PATTERN_LIST__
#define __PATTERN_LIST__

#include "defines.h"
#include "state_go.hpp"
#include <fstream>
#include <iostream>

class StateGo;

class PatternList {
private:
  typedef enum {
    p_empty,
    p_white,
    p_black,
    p_everything,
    p_no_white,
    p_no_black,
    p_border,
  } PattField;
  typedef struct {
    PattField x[3][3];
  } Pattern;
  bool list[27000];
  void add(Pattern pattern);
  void add_pattern(Pattern pattern);
  void rotate_pattern(Pattern & pattern) const;
  void invert_pattern(Pattern & pattern) const;
  void mirror_pattern(Pattern & pattern) const;
public:
  PatternList();
  bool match(StateGo * state, INDEX i, INDEX j) const;
  void read_file(const char *file_name);
};

#endif                          // __PATTERN_LIST__
