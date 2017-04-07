#ifndef __DEFINES__
#define __DEFINES__

#include "node.hpp"
#include <stdint.h>

#define PlayerToCell(p)  (p==CIRCLE_P ? CIRCLE : CROSS)
#define CellToPlayer(p)  (p==CIRCLE ? CIRCLE_P : (p==CROSS ? CROSS_P : assert(0)))
#define ChangePlayer(p)  (p==CIRCLE_P ? CROSS_P  : CIRCLE_P)
#define NUM_THREADS      5
#define NUM_CYCLES       100000
#define MAX_SECONDS      1

typedef enum:uint8_t {
  CIRCLE_P = 0,
  CROSS_P = 1,
} Player;

typedef enum:int8_t {
  EMPTY = 0,
  CIRCLE = -1,
  CROSS = 1,
} Cell;

struct DataTateti {
  uint8_t i;
  uint8_t j;
  Player player;
  DataTateti(uint8_t a = 0, uint8_t b = 0, Player p = CIRCLE_P) :
    i(a), j(b), player(p) {};
  inline bool operator ==(const DataTateti & dat) const {
    return i == dat.i && j == dat.j && player == dat.player;
  };
};

typedef double ValTateti;
typedef NodeUCT< ValTateti, DataTateti> Nod;

#endif
