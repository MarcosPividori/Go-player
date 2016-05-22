#ifndef __DEFINES__
#define __DEFINES__

#include <stdint.h>
#include "node.hpp"

#define PlayerToCell(p)  (p==CIRCLE_P ? CIRCLE : CROSS)
#define CellToPlayer(p)  (p==CIRCLE ? CIRCLE_P : (p==CROSS ? CROSS_P : assert(0)))
#define ChangePlayer(p)  (p==CIRCLE_P ? CROSS_P  : CIRCLE_P)
#define NUM_THREADS      5
#define NUM_CYCLES       50000
#define MAX_SECONDS      1.5

typedef enum : uint8_t {
    CIRCLE_P = 0,
    CROSS_P  = 1,
} Player;

typedef enum : int8_t {
    EMPTY  =  0,
    CIRCLE = -1,
    CROSS  =  1,
} Cell;

struct DataConnect4 {
    uint8_t pos;
    Player player;
    DataConnect4(uint8_t a=0,Player b=CIRCLE_P) : pos(a), player(b) {};
    inline bool operator ==(const DataConnect4& dat) const {
        return pos==dat.pos && player==dat.player;
    };
};

typedef double ValConnect4;
typedef NodeUCT<ValConnect4,DataConnect4> Nod;

#endif
