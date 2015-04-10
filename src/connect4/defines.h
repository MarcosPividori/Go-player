#ifndef __DEFINES__
#define __DEFINES__

#include "node.hpp"

#define MOVE(pos,player) (pos*2+player)
#define Pos(m)           m/2
#define Player(m)        m%2
#define PlayerToCell(p)  (p==Circle ? CIRCLE : CROSS)
#define CellToPlayer(p)  (p==CIRCLE ? Circle : (p==CROSS ? Cross : assert(0)))
#define ChangePlayer(p)  (p==Circle ? Cross  : Circle)
#define NUM_THREADS      5
#define NUM_CYCLES       50000
#define MAX_SECONDS      1.5

typedef long DataConnect4;
typedef double ValConnect4;
typedef NodeUCT<ValConnect4,DataConnect4> Nod;

typedef enum {
    Circle = 0,
    Cross  = 1,
} Player;

typedef enum {
    EMPTY  =  0,
    CIRCLE = -1,
    CROSS  =  1,
} CELL;

#endif
