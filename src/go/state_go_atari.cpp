
#define INIT_LIB  0
#define FOUND_LIB 1
#define FAIL_LIB  2

unsigned int StateGo::get_liberty_block(Block *block,Block *flag,INDEX i,INDEX j,INDEX &lib_i,INDEX &lib_j)
{
    Blocks[i][j]= flag;
    INDEX t_i,t_j;
    unsigned int res=INIT_LIB,v;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Blocks[k][l]==block){//If same block, propagate.
        v=get_liberty_block(block,flag,k,l,t_i,t_j);
        if(v == FAIL_LIB)
          return FAIL_LIB;
        if(v == FOUND_LIB)
          if(res == FOUND_LIB){
            if(lib_i!=t_i || lib_j!=t_j)
              return FAIL_LIB;
          }
          else{
            lib_i=t_i;lib_j=t_j;
            res=FOUND_LIB;
          }
      }
      else
        if(Stones[k][l]==Empty)
          if(res == FOUND_LIB){
            if(lib_i!=k || lib_j!=l)
              return FAIL_LIB;
          }
          else{
            lib_i=k;lib_j=l;
            res=FOUND_LIB;
          }
    }
    );
    return res;
}

inline bool StateGo::is_block_in_atari(INDEX i,INDEX j,INDEX &i_atari,INDEX &j_atari)
{
    if(Blocks[i][j]->adj >4)
      return false;
    Block *block=Blocks[i][j];
    bool res=false;
    Block flag;
    if(get_liberty_block(block,&flag,i,j,i_atari,j_atari)==FOUND_LIB)
        res=true;
    update_block(&flag,block,i,j);
    return res;
}

DataGo StateGo::look_for_delete_atari(Block *block,Block *flag,INDEX i,INDEX j,int &max_size)
{
    Blocks[i][j]= flag;
    Player opp=CHANGE_PLAYER(Stones[i][j]);
    DataGo res=PASS(Empty);
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Blocks[k][l]==block){//If same block, propagate.
        DataGo v=look_for_delete_atari(block,flag,k,l,max_size);
        if(!IS_PASS(v))
          res=v;
      }
      else
        if(Stones[k][l]==opp)
          if(Blocks[k][l]->is_atari() && Blocks[k][l]->size>max_size){
            DataGo d=Blocks[k][l]->atari;
            if(!(ko.flag && ko.i==d.i && ko.j==d.j)){
              max_size=Blocks[k][l]->size;
              res=d;
            }
          }
    }
    );
    return res;
}

inline DataGo StateGo::get_delete_atari(INDEX i,INDEX j,int &b_size)
{
    b_size=0;
    Block *block=Blocks[i][j];
    Block flag;
    DataGo res=look_for_delete_atari(block,&flag,i,j,b_size);
    update_block(&flag,block,i,j);
    return res;
}
