
#include "state_go.hpp"
#include <cassert>
#include <queue>

#define FLAG_B    0x80000000
#define FLAG_W    0x40000000
#define FLAG_BOTH (FLAG_B | FLAG_W)
#define NO_FLAG   0x3fffffff

#define INIT_LIB  0
#define FOUND_LIB 1
#define FAIL_LIB  2

#define FOR_EACH_ADJ(i,j,k,l,Code)  ( \
             { if(i>0){ k=i-1;l=j;Code;} \
              if(i<_size-1){ k=i+1;l=j;Code;} \
              if(j>0){ k=i;l=j-1;Code;} \
              if(j<_size-1){ k=i;l=j+1;Code;} })

StateGo::StateGo(int size,float komi,PatternList *p) : _size(size),_komi(komi),patterns(p),num_movs(0)
#ifdef JAPANESE
        ,captured_b(0),captured_w(0)
#endif
{
    Stones = new Player*[_size];
    for(int i=0;i<_size;i++)
        Stones[i] = new Player[_size];

    Blocks = new Block**[_size];
    for(int i=0;i<_size;i++)
        Blocks[i] = new Block*[_size];
    
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++){
            Stones[i][j]=Empty;
            Blocks[i][j]=NULL;
        }
    pass=0;
    turn=Black;
    last_mov=PASS(CHANGE_PLAYER(turn));
}

StateGo *StateGo::copy()
{
    StateGo *p= new StateGo(_size,_komi,patterns);
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++){
            p->Stones[i][j]=Stones[i][j];
            p->Blocks[i][j]=Blocks[i][j];
        }
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++)
            if(p->Stones[i][j]!=Empty && p->Blocks[i][j]==Blocks[i][j]){
                p->Blocks[i][j]=new Block;
                *(p->Blocks[i][j])=*Blocks[i][j];
                p->update_block(Blocks[i][j],p->Blocks[i][j],i,j);
            }
    p->ko=ko;
    p->pass=pass;
    p->turn=turn;
    p->patterns=patterns;
    p->last_mov=last_mov;
#ifdef JAPANESE
    p->captured_b=captured_b;
    p->captured_w=captured_w;
#endif 
    p->num_movs=num_movs;
    return p;
}

StateGo::~StateGo()
{
    for(int i=0;i<_size;i++){
      for(int j=0;j<_size;j++)
        if(Blocks[i][j]!=NULL){
          delete Blocks[i][j];
          update_block(Blocks[i][j],NULL,i,j);
        }
      delete[] Blocks[i];
      delete[] Stones[i];
    }
    delete[] Blocks;
    delete[] Stones;    
}

// RECURSIVE ELIMINATING (DFS)
void StateGo::eliminate_block(Block *block,INDEX i,INDEX j)
{
    Blocks[i][j]=NULL;
#ifdef JAPANESE
    if(Stones[i][j]==Black)
        captured_b++;
    else
        captured_w++;
#endif
    Stones[i][j]=Empty;
    INDEX k,l;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Blocks[k][l]==block)//If same block, propagate.
        eliminate_block(block,k,l);
      else
        if(Stones[k][l]!=Empty){//Add a free adjacency.
          Blocks[k][l]->adj+=1;
          Blocks[k][l]->no_atari();
        }
    }
    );
}

// RECURSIVE UPDATING (DFS)
void StateGo::update_block(Block *block,Block *new_block,INDEX i,INDEX j)
{
    Blocks[i][j]=new_block;
    INDEX k,l;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Blocks[k][l]==block)//If same block, propagate.
        update_block(block,new_block,k,l);
    }
    );
}

unsigned int StateGo::get_liberty_block(Block *block,Block *flag,INDEX i,INDEX j,INDEX &lib_i,INDEX &lib_j)
{
    Blocks[i][j]= flag;
    INDEX t_i,t_j;
    unsigned int res=INIT_LIB,v;
    INDEX k,l;
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

DataGo StateGo::look_for_delete_atari(Block *block,Block *flag,INDEX i,INDEX j)
{
    Blocks[i][j]= flag;
    INDEX k,l;
    Player opp=CHANGE_PLAYER(Stones[i][j]);
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Blocks[k][l]==block){//If same block, propagate.
        DataGo v=look_for_delete_atari(block,flag,k,l);
        if(!IS_PASS(v))
          return v;
      }
      else
        if(Stones[k][l]==opp)
          if(Blocks[k][l]->is_atari()){
            DataGo d=Blocks[k][l]->atari;
            if(!(ko.flag && ko.i==d.i && ko.j==d.j))
              return d;
          }
    }
    );
    return PASS(Empty);
}

inline DataGo StateGo::get_delete_atari(INDEX i,INDEX j)
{
    Block *block=Blocks[i][j];
    Block flag;
    DataGo res=look_for_delete_atari(block,&flag,i,j);
    update_block(&flag,block,i,j);
    return res;
}

unsigned int StateGo::count_area(bool **visited,INDEX i,INDEX j)
{
    unsigned int res=1,v;
    visited[i][j]=true;
    INDEX k,l;
    FOR_EACH_ADJ(i,j,k,l,
    {
      switch(Stones[k][l]){
        case Black: res|=FLAG_B;break;
        case White: res|=FLAG_W;break;
        default: if(!visited[k][l]){
                    v=count_area(visited,k,l);
                    res = (v | (res & FLAG_BOTH)) + (res & NO_FLAG);
                 }
      }
    }
    );
    return res;
}

inline void StateGo::get_possible_moves(std::vector<DataGo>& v)
{
    if(pass==2)
        return;
    for(INDEX i=0;i<_size;i++)
        for(INDEX j=0;j<_size;j++)
            if(Stones[i][j]==Empty && no_ko_nor_suicide(i,j,turn))
                v.push_back(DataGo(i,j,turn));
    v.push_back(PASS(turn));
}

#ifdef KNOWLEDGE
void StateGo::get_simulation_possible_moves(std::vector<DataGo>& v)
{
    if(pass==2)
        return;
/*
    unsigned int res;
    bool **visited= new bool*[_size];
    bool **area_one_color= new bool*[_size];
    for(int i=0;i<_size;i++){
        visited[i]= new bool[_size];
        area_one_color[i]= new bool[_size];
        for(int j=0;j<_size;j++){
            visited[i][j]=false;
            area_one_color[i][j]=false;
        }
    }
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++)
            if(Stones[i][j]==Empty && !visited[i][j]){
                res=count_area(visited,i,j);
                if((res & FLAG_B) && (res & FLAG_W))
                    continue;
                if((res & FLAG_B) && turn == White)
                    continue;
                if((res & FLAG_W) && turn == Black)
                    continue;
                if((res & NO_FLAG)==3 ||(res & NO_FLAG) >(_size*_size/3))
                    continue;
                count_area(area_one_color,i,j);
            }

    for(INDEX i=0;i<_size;i++)
        for(INDEX j=0;j<_size;j++)
            if(Stones[i][j]==Empty
               && ((!area_one_color[i][j]
                    && no_self_atari_nor_suicide(i,j,turn))
                   || remove_opponent_block_and_no_ko(i,j,turn)))
                v.push_back(DataGo(i,j,turn));
*/
    for(INDEX i=0;i<_size;i++)
        for(INDEX j=0;j<_size;j++)
            if(Stones[i][j]==Empty
               && (no_self_atari_nor_suicide(i,j,turn)
                   || remove_opponent_block_and_no_ko(i,j,turn)))
                v.push_back(DataGo(i,j,turn));
    v.push_back(PASS(turn));
/*
    for(int i=0;i<_size;i++){
        delete[] visited[i];
        delete[] area_one_color[i];
    }
    delete[] visited;
    delete[] area_one_color;
*/
}

void StateGo::get_atari_escape_moves(std::vector<DataGo>& v)
{
    if(IS_PASS(last_mov))
      return;
    Block *b[4]={NULL,NULL,NULL,NULL};
    INDEX k,l,m,n,i=last_mov.i,j=last_mov.j;
    int sum=0,c=0,count;
    FOR_EACH_ADJ(i,j,k,l,
    {
        if(Stones[k][l]==turn
           && Blocks[k][l]!=b[0]
           && Blocks[k][l]!=b[1]
           && Blocks[k][l]!=b[2]
           && Blocks[k][l]->is_atari()){
           b[c]=Blocks[k][l];
           c++;
           DataGo e=get_delete_atari(k,l);
           if(!IS_PASS(e))
               for(int count=Blocks[k][l]->size;count>0;count--){
                 v.push_back(DataGo(e.i,e.j,turn));
                 v.push_back(DataGo(e.i,e.j,turn));
                 v.push_back(DataGo(e.i,e.j,turn));
                 v.push_back(DataGo(e.i,e.j,turn));
               }
           DataGo d=Blocks[k][l]->atari;
           if(no_self_atari_nor_suicide(d.i,d.j,turn)){
             sum=0;
             FOR_EACH_ADJ(d.i,d.j,m,n,
             {
               if(Stones[m][n]==Empty) sum++;
               if(Stones[m][n]==turn && Blocks[m][n]!=Blocks[k][l]) sum=4;
             }
             );
             if(sum>2)
               for(int count=Blocks[k][l]->size;count>0;count--){
                 v.push_back(DataGo(d.i,d.j,turn));
               }
           }
        }
    }
    );
}

void StateGo::get_pattern_moves(std::vector<DataGo>& v)
{
    if(patterns==NULL)
        return;
    if(IS_PASS(last_mov))
        return;
    for(INDEX k=MAX(last_mov.i-1,0);k<= MIN(_size-1,last_mov.i+1);k++)
      for(INDEX l=MAX(last_mov.j-1,0);l<= MIN(_size-1,last_mov.j+1);l++)
        if(Stones[k][l]==Empty){ 
          Stones[k][l]=turn;
          if(patterns->match(this,k,l))
            if(no_self_atari_nor_suicide(k,l,turn)
               || remove_opponent_block_and_no_ko(k,l,turn))
                v.push_back(DataGo(k,l,turn));
          Stones[k][l]=Empty;
        }
}

void StateGo::get_capture_moves(std::vector<DataGo>& v)
{
    if(pass==2)
        return;
    int c=0;
    for(INDEX i=0;i<_size;i++)
      for(INDEX j=0;j<_size;j++)
        if(Stones[i][j]==Empty && (c=remove_opponent_block_and_no_ko(i,j,turn)))
          for(;c>0;c--){
            v.push_back(DataGo(i,j,turn));
            v.push_back(DataGo(i,j,turn));
            v.push_back(DataGo(i,j,turn));
            v.push_back(DataGo(i,j,turn));
          }
}

bool StateGo::is_completely_empty(INDEX i,INDEX j)
{
    for(int k=MAX(i-1,0);k<= MIN(_size-1,i+1);k++)
      for(int l=MAX(j-1,0);l<= MIN(_size-1,j+1);l++)
        if(Stones[k][l]!=Empty)
            return false;
    return true;
}
#endif

inline ValGo StateGo::get_final_value()
{
    float res = final_value();
    if(res<0)
        return Black;
    else if(res>0)
        return White;
    return Empty;
}

float StateGo::get_final_score()
{
    return final_value();
}

inline float StateGo::final_value()
{
    if(pass<2)
        return Empty;
    bool **visited= new bool*[_size];
    for(int i=0;i<_size;i++){
        visited[i]= new bool[_size];
        for(int j=0;j<_size;j++)
            visited[i][j]=false;
    }
    unsigned int res,numb=0,numw=0;
    float countb=0,countw=0;
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++)
            if(Stones[i][j]==Empty && !visited[i][j]){
                res=count_area(visited,i,j);
                if((res & FLAG_B) && (res & FLAG_W))
                    continue;
                if(res & FLAG_B)
                    numb+=(res & NO_FLAG);
                if(res & FLAG_W)
                    numw+=(res & NO_FLAG);
            }
#ifndef JAPANESE
             else if(Stones[i][j]==White)
                countw++;
             else if(Stones[i][j]==Black)
                countb++;
#endif
    for(int i=0;i<_size;i++){
        delete[] visited[i];
    }
    delete[] visited;
#ifdef JAPANESE
    countb=float(numb)-float(captured_b);
    countw=float(numw)+_komi-float(captured_w);
#else
    countb+=float(numb);
    countw+=float(numw)+_komi;
#endif
    return countw-countb; 
}

inline void StateGo::apply(DataGo d)
{
    assert(d.player == turn);
    num_movs++;
    if(ko.player==turn)
        ko.flag=0;
    if(IS_PASS(d)){
        assert(pass<2);
        pass++;
    }
    else
    {
        assert(d.i>=0 && d.i<_size);
        assert(d.j>=0 && d.j<_size);
        assert(Stones[d.i][d.j] == Empty);
        pass=0;
        Block *loc_block = new Block;
        Blocks[d.i][d.j] = loc_block;
        Stones[d.i][d.j] = turn;
        int i=d.i,j=d.j,k,l;
        //Reduce adj of adjacent blocks.
        //Count actual adj.
        //Delete blocks of opponent with adjacency equal to zero.
        FOR_EACH_ADJ(i,j,k,l,
        {
          if(Stones[k][l]==Empty)
            loc_block->adj++;
          else
            if(! --(Blocks[k][l]->adj))//if no free adjacency.
              if(Stones[k][l]!=d.player){
                if(Blocks[k][l]->size==1){
                    ko.i=k;ko.j=l;ko.player=CHANGE_PLAYER(turn);ko.flag=1;
                }
                delete Blocks[k][l];
                eliminate_block(Blocks[k][l],k,l);
              }
        }
        );
        //Try to join blocks of same color.
        bool first=true;
        FOR_EACH_ADJ(i,j,k,l,
        {
          if(Stones[k][l]==d.player && Blocks[k][l]!=Blocks[i][j])
            if(first){
              Blocks[k][l]->join(loc_block);
              delete loc_block;
              Blocks[i][j]=Blocks[k][l];
              first=false;
            }
            else{
              Blocks[i][j]->join(Blocks[k][l]);
              delete Blocks[k][l];
              update_block(Blocks[k][l],Blocks[i][j],k,l);
            }
        }
        );

        //update atari state of adjacent blocks.
        Block *b[4]={NULL,NULL,NULL,NULL};
        INDEX t_i,t_j;
        int sum=0,c=0,count;
        Player opp=CHANGE_PLAYER(turn);
        FOR_EACH_ADJ(i,j,k,l,
        {
            if(Stones[k][l]==opp
               && Blocks[k][l]!=b[0]
               && Blocks[k][l]!=b[1]
               && Blocks[k][l]!=b[2]
               && is_block_in_atari(k,l,t_i,t_j)){
               Blocks[k][l]->atari=DataGo(t_i,t_j,turn);
               b[c]=Blocks[k][l];
               c++;
            }
        }
        );
        if(is_block_in_atari(i,j,t_i,t_j))
           Blocks[i][j]->atari=DataGo(t_i,t_j,CHANGE_PLAYER(turn));
        else
           Blocks[i][j]->atari=PASS(Empty);

    }
    turn=CHANGE_PLAYER(turn);
    last_mov=d;
}

void StateGo::show(FILE *output){
    char c;
    fprintf(output,"   ");
    for(int i=0;i<_size;i++)
        fprintf(output," %c",'A'+i+(i>7));
    for(int i=_size-1;i>=0;i--){
        fprintf(output,"\n%2d ",i+1);
        for(int j=0;j<_size;j++){
            switch(Stones[i][j]){
                case Empty: c='.';break;
                case White: c='O';break;
                case Black: c='X';break;
            }
            fprintf(output," %c",c);
        }
    }
}

void StateGo::show(){
    show(stdout);
}

inline int StateGo::remove_opponent_block_and_no_ko(INDEX i,INDEX j,Player p)
{
    int sum=0,c=0,remove_count=0;
    Block *b[4]={NULL,NULL,NULL,NULL};
    int sumb[4]={0,0,0,0},ib[4],jb[4];
    Player opp=CHANGE_PLAYER(turn);

    //Check if some opponent block will be removed.
    INDEX k,l;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Stones[k][l]==opp){
        if(Blocks[k][l]==b[0])
          sumb[0]-=1;
        else if(Blocks[k][l]==b[1])
          sumb[1]-=1;
        else if(Blocks[k][l]==b[2])
          sumb[2]-=1;
        else{
          sumb[c]+=(Blocks[k][l]->adj)-1;
          b[c]=Blocks[k][l];
          ib[c]=k;jb[c]=l;
          c++;
        }
      }
    }
    );

    int counter=0,i1,j1;
    for(int k=0;k<c;k++)
      if(sumb[k]==0){
        remove_count+=b[k]->size;
        counter++;
        i1=ib[k];
        j1=jb[k];
      }

    if(counter==0)
        return 0;

    //Check Ko situation.
    if(ko.flag && ko.i==i && ko.j==j && counter==1){
        //Check if block destroyed is size > 1
        if(Blocks[i1][j1]->size == 1)
            return 0;
    }
    return remove_count;
}

inline bool StateGo::no_self_atari_nor_suicide(INDEX i,INDEX j,Player p)
{
    int c_empty=0;
    bool flag=false;
    INDEX l_i,l_j;
    INDEX k,l;
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

inline bool StateGo::no_ko_nor_suicide(INDEX i,INDEX j,Player p)
{
    INDEX k,l;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Stones[k][l]==Empty)
        return true;
    }
    );

    //Check if free block near position.
    int sum=0,c=0;
    Block *b[4]={NULL,NULL,NULL,NULL};
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Stones[k][l]==p){
        if(Blocks[k][l]!=b[0] && Blocks[k][l]!=b[1] && Blocks[k][l]!=b[2]){
          sum+=Blocks[k][l]->adj;
          b[c]=Blocks[k][l];
          c++;
        }
        sum-=1;
      }
    }
    );

    if(sum!=0)
        return true;

    if(remove_opponent_block_and_no_ko(i,j,p))
        return true;
    return false;
}

bool StateGo::valid_move(DataGo d)
{
    if(d.player!=turn)
      return false;
    if(IS_PASS(d))
      return pass<2; 
    if(Stones[d.i][d.j]!=Empty)
      return false;
    if(no_ko_nor_suicide(d.i,d.j,d.player))
      return true;
    return false;
}

