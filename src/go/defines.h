#ifndef __DEFINES__
#define __DEFINES__

#include <stdint.h>

#define VERSION          "1.0"
#define GTP_VERSION      2
#define MAX_BOARD        19      
#define UNUSED(x)        (void)x
#define PASSI            -1
#define RESIGNI          -2
#define PASS(player)     DataGo(PASSI,PASSI,player)
#define IS_PASS(d)       (d.i==PASSI)
#define RESIGN(player)   DataGo(RESIGNI,RESIGNI,player)
#define IS_RESIGN(d)     d.i==RESIGNI
#define MOVE(player,i,j) DataGo(i,j,player)
#define INIT_DATA(p)     PASS(p)
#define CHANGE_PLAYER(p) (p==White ? Black : White)
#define MIN(a,b)         ((a)<(b)? a : b)
#define MAX(a,b)         ((a)>(b)? a : b)

typedef enum : char{
    Black = -1,
    Empty = 0,
    White = 1,
} Player;

typedef char INDEX;

union POS {
    struct{
      INDEX i;
      INDEX j;
    };
    uint16_t hash;
    POS(INDEX _i,INDEX _j) : i(_i),j(_j) {}
    POS() {}
    bool operator==(POS a) {
       return a.hash==hash;
    }
    bool operator>(POS a) {
       return a.hash<hash;
    }
    bool operator<(POS a) {
       return a.hash>hash;
    }
};

union DataGo
{
    struct{
        Player player;
        INDEX i;
        INDEX j;
        INDEX flag;
    };
    uint32_t hash;
    bool operator==(DataGo a) {
       return a.hash==hash;
    }
    DataGo(INDEX _i,INDEX _j,Player _p) : i(_i),j(_j),player(_p),flag(0) {}
    DataGo(POS _pos,Player _p) : i(_pos.i),j(_pos.j),player(_p),flag(0) {}
    DataGo() : flag(0) {}
};

#endif// __DEFINES__
