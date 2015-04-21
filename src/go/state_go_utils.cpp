
bool StateGo::is_useful_move(DataGo mov)
{
    if(IS_PASS(mov))
        return true;
    return (no_self_atari_nor_suicide(mov.i,mov.j,turn)
         || remove_opponent_block_and_no_ko(mov.i,mov.j));
}

bool StateGo::is_completely_empty(INDEX i,INDEX j)
{
    for(int k=MAX(i-1,0);k<= MIN(_size-1,i+1);k++)
      for(int l=MAX(j-1,0);l<= MIN(_size-1,j+1);l++)
        if(Stones[k][l]!=Empty)
            return false;
    return true;
}

inline bool StateGo::remove_opponent_block_and_no_ko(INDEX i,INDEX j)
{
    if(ko.flag && ko.i==i && ko.j==j)
      return false;
    Player opp=CHANGE_PLAYER(turn);
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Stones[k][l]==opp)
        if(Blocks[k][l]->is_atari())
          return true;
    }
    );
    return false;
}

inline bool StateGo::no_self_atari_nor_suicide(INDEX i,INDEX j,Player p)
{
    int c_empty=0;
    bool flag=false;
    INDEX l_i,l_j;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Stones[k][l]==Empty){
        flag=true;
        l_i=k;
        l_j=l;
        c_empty++;
      }
    }
    );

    if(c_empty>1)
      return true;

    //Check if next blocks of same player wont be atari.
    Stones[i][j]=p;
    INDEX t_i,t_j;
    FOR_EACH_ADJ(i,j,k,l,
    {
        if(Stones[k][l]==p)
          if(!(Blocks[k][l]->is_atari()))
            if(is_block_in_atari(k,l,t_i,t_j))
              if(flag){
                if(t_i!=l_i || t_j!=l_j){
                  Stones[i][j]=Empty;
                  return true;
                }
              }
              else{
                flag=true;
                l_i=t_i;
                l_j=t_j;
              }
            else{
              Stones[i][j]=Empty;
              return true;
            }
    }
    );
    Stones[i][j]=Empty;
    return false;
}

inline bool StateGo::no_suicide(INDEX i,INDEX j,Player p)
{
    if(Stones[i][j]!=Empty)
      return false;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Stones[k][l]==Empty)
        return true;
      if(Stones[k][l]==p){
        if(!(Blocks[k][l]->is_atari()))
          return true;
      }else
        if(Blocks[k][l]->is_atari())
          return true;
    }
    );
    return false;
}

inline bool StateGo::no_ko_nor_suicide(INDEX i,INDEX j,Player p)
{
    if(ko.flag && ko.i==i && ko.j==j)
      return false;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Stones[k][l]==Empty)
        return true;
      if(Stones[k][l]==p){
        if(!(Blocks[k][l]->is_atari()))
          return true;
      }else
        if(Blocks[k][l]->is_atari())
          return true;
    }
    );
    return false;
}
