#ifndef __DEFINES__
#define __DEFINES__

#define MOVE(i,j,player) ((i)*22+(j)*2+(player))
#define I(m)             (m)/22
#define J(m)             ((m)%22)/2
#define Player(m)        (m)%2
#define PlayerToCell(p)  (p==Circle ? CIRCLE : CROSS)
#define CellToPlayer(p)  (p==CIRCLE ? Circle : (p==CROSS ? Cross : assert(0)))
#define ChangePlayer(p)  (p==Circle ? Cross  : Circle)

typedef long DataHexa;
typedef double ValHexa;

#ifdef RAVE
#include "node_rave.hpp"
typedef NodeUCTRave<ValHexa,DataHexa> Nod;
#else
#include "node.hpp"
typedef NodeUCT<ValHexa,DataHexa> Nod;
#endif

typedef enum {
    Circle = 0,
    Cross  = 1,
} Player;

typedef enum {
    EMPTY  =  0,
    CIRCLE = -1,
    CROSS  =  1,
} CELL;

#endif// __DEFINES__
