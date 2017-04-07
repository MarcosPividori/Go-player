// RECURSIVE ELIMINATING (DFS)
void StateGo::eliminate_block(Block * block, INDEX i, INDEX j) {
  Blocks[i][j] = NULL;
  if (Stones[i][j] == Black)
    captured_b++;
  else
    captured_w++;
  Stones[i][j] = Empty;
  b_mov.insert(POS(i, j));
  w_mov.insert(POS(i, j));
  FOR_EACH_ADJ(i, j, k, l, {
    if (Blocks[k][l] == block)       //If same block, propagate.
      eliminate_block(block, k, l);
    else
      if (Stones[k][l] != Empty) {     //Add a free adjacency.
        Blocks[k][l]->adj += 1;
        if (Blocks[k][l]->is_atari()) {
          INDEX a = Blocks[k][l]->atari.i;
          INDEX b = Blocks[k][l]->atari.j;
          Blocks[k][l]->no_atari();
          update_mov(a, b); remove_atari_block(k, l);
        }
      }
  });
}

// RECURSIVE UPDATING (DFS)
void StateGo::update_block(Block * block, Block * new_block, INDEX i, INDEX j) {
  Blocks[i][j] = new_block;
  FOR_EACH_ADJ(i, j, k, l, {
    if (Blocks[k][l] == block)       //If same block, propagate.
      update_block(block, new_block, k, l);
  });
}
