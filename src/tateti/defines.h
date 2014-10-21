#ifndef __DEFINES__
#define __DEFINES__

#include "node.hpp"

#define MOVE(i,j,player) ((i)*6+(j)*2+(player))
#define I(m) (m)/6
#define J(m) ((m)%6)/2
#define Player(m) (m)%2
#define PlayerToCell(p) (p==Circle ? CIRCLE : CROSS)
#define CellToPlayer(p) (p==CIRCLE ? Circle : (p==CROSS ? Cross : assert(0)))
#define ChangePlayer(p) (p==Circle ? Cross  : Circle)
#define NUM_THREADS 5
#define NUM_CYCLES 100000

typedef long DataTateti;
typedef double ValTateti;
typedef NodeUCT<ValTateti,DataTateti> Nod;

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
