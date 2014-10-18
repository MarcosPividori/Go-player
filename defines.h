#ifndef __DEFINES__
#define __DEFINES__

#define VERSION          "1.0"
#define GTP_VERSION      2
#define MAX_BOARD        19      

#define UNUSED(x)        (void)x

#define PASSI            -1
#define RESIGNI          -2
#define PASS(player)     {player,PASSI,PASSI}
#define IS_PASS(d)       (d.i==PASSI)
#define RESIGN(player)   {player,RESIGNI,RESIGNI}
#define IS_RESIGN(d)     d.i==RESIGNI
#define MOVE(player,i,j) {player,i,j}
#define CHANGE_PLAYER(p) (p==White ? Black : White)

#define MIN(a,b)  ((a)<(b)? a : b)
#define MAX(a,b)  ((a)>(b)? a : b)

typedef enum : char{
    Black = -1,
    Empty = 0,
    White = 1,
} Player;

typedef char INDEX;

struct DataGo
{
    Player player;
    INDEX i;
    INDEX j;
    bool operator==(DataGo a) {
       if (a.i==i && a.j==j && a.player==player)
          return true;
       else
          return false;
    }
};

#endif// __DEFINES__
