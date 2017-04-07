#include "defines.h"

class MoveRecorderGo : public MoveRecorder<DataGo> {
 private:
  Player Stones[MAX_BOARD][MAX_BOARD];
 public:
  void clear();
  void postMove(DataGo move);
  void prevMove(DataGo move);
  bool isMove(DataGo move);
};

inline void MoveRecorderGo::clear() {
  for (int i = 0; i < MAX_BOARD; i++)
    for (int j = 0; j < MAX_BOARD; j++)
      Stones[i][j] = Empty;
}

inline void MoveRecorderGo::postMove(DataGo move) {
  if (!IS_PASS(move))
    if (Stones[move.i][move.j] == Empty)
      Stones[move.i][move.j] = move.player;
}

inline void MoveRecorderGo::prevMove(DataGo move) {
  if (!IS_PASS(move))
    Stones[move.i][move.j] = move.player;
}

inline bool MoveRecorderGo::isMove(DataGo move) {
  if (IS_PASS(move))
    return false;
  if (Stones[move.i][move.j] == move.player)
    return true;
  return false;
}
