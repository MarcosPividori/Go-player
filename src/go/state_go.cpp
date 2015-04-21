
#include "state_go.hpp"
#include <iomanip>
#include <cassert>
#include <queue>

#define FOR_EACH_ADJ(i,j,k,l,Code)  ( \
            { \
              INDEX k,l;\
              if(i>0)      { k=i-1;l=j;Code;} \
              if(i<_size-1){ k=i+1;l=j;Code;} \
              if(j>0)      { k=i;l=j-1;Code;} \
              if(j<_size-1){ k=i;l=j+1;Code;} \
            })

#include "state_go_atari.cpp"
#include "state_go_show.cpp"
#include "state_go_utils.cpp"
#include "state_go_value.cpp"
#include "state_go_blocks.cpp"

bool StateGo::japanese_rules;
double StateGo::pattern_coeff;
double StateGo::capture_coeff;
double StateGo::atari_delete_coeff;
double StateGo::atari_escape_coeff;

StateGo::StateGo(int size,float komi,PatternList *p) : 
         _size(size)
        ,_komi(komi)
        ,patterns(p)
        ,num_movs(0)
        ,captured_b(0)
        ,captured_w(0)
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
            b_mov.insert(POS(i,j));
            w_mov.insert(POS(i,j));
        }
    pass=0;
    turn=Black;
    last_mov=PASS(CHANGE_PLAYER(turn));
}

StateGo::StateGo(StateGo *src) :
         _size(src->_size)
        ,_komi(src->_komi)
        ,patterns(src->patterns)
        ,num_movs(src->num_movs)
        ,ko(src->ko)
        ,pass(src->pass)
        ,turn(src->turn)
        ,last_mov(src->last_mov)
        ,w_mov(&(src->w_mov))
        ,b_mov(&(src->b_mov))
        ,captured_b(src->captured_b)
        ,captured_w(src->captured_w)
{
    Stones = new Player*[_size];
    for(int i=0;i<_size;i++)
        Stones[i] = new Player[_size];

    Blocks = new Block**[_size];
    for(int i=0;i<_size;i++)
        Blocks[i] = new Block*[_size];

    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++){
            Stones[i][j]=src->Stones[i][j];
            Blocks[i][j]=src->Blocks[i][j];
        }

    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++)
            if(Stones[i][j]!=Empty && Blocks[i][j]==src->Blocks[i][j]){
                Blocks[i][j]=new Block;
                *(Blocks[i][j])=*(src->Blocks[i][j]);
                update_block(src->Blocks[i][j],Blocks[i][j],i,j);
            }

    w_atari.reserve(src->w_atari.capacity());
    for(int l=0;l<src->w_atari.size();l++)
        w_atari.push_back(src->w_atari[l]);
    b_atari.reserve(src->b_atari.capacity());
    for(int l=0;l<src->b_atari.size();l++)
        b_atari.push_back(src->b_atari[l]);
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

void StateGo::get_possible_moves(vector<DataGo>& v)
{
    if(pass==2)
        return;
    if(turn==White)
      for(int i=0;i< w_mov.size();i++){
        DataGo mov= DataGo(w_mov[i],turn);
        if(is_useful_move(mov))
          v.push_back(mov);
      }  
    else
      for(int i=0;i< b_mov.size();i++){
        DataGo mov= DataGo(b_mov[i],turn);
        if(is_useful_move(mov))
          v.push_back(mov);
      }
}

int StateGo::possible_moves_size()
{
    if(pass==2)
      return 0;
    if(turn==White)
      return w_mov.size();
    else
      return b_mov.size();
}

DataGo StateGo::get_possible_moves_by_index(int i)
{
    if(turn==White)
      return DataGo(w_mov[i],turn);
    else
      return DataGo(b_mov[i],turn);
}

void StateGo::get_atari_escape_moves(vector<DataGo>& v)
{
    if(IS_PASS(last_mov))
      return;
    INDEX k,l;
    int sum=0,c=0,count,size;
    vector<POS> *atari_blocks;
    if(turn==White)
        atari_blocks=&w_atari;
    else
        atari_blocks=&b_atari;
    for(int i=0;i<atari_blocks->size();i++){
        k=(*atari_blocks)[i].i;
        l=(*atari_blocks)[i].j;
        DataGo e=get_delete_atari(k,l,size);
        if(!IS_PASS(e))
            for(int count=(Blocks[k][l]->size+size)*atari_delete_coeff;count>0;count--)
                v.push_back(DataGo(e.i,e.j,turn));
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
              for(int count=(Blocks[k][l]->size * atari_escape_coeff);count>0;count--)
                  v.push_back(DataGo(d.i,d.j,turn));
        }
    }
}

void StateGo::get_pattern_moves(vector<DataGo>& v)
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
               || remove_opponent_block_and_no_ko(k,l)){
                for(int i=0;i<pattern_coeff;i++)
                  v.push_back(DataGo(k,l,turn));
            }
          Stones[k][l]=Empty;
        }
}

void StateGo::get_capture_moves(vector<DataGo>& v)
{
    if(pass==2)
        return;
    int c=0;
    Block *block;
    if(turn==White)
      for(int i=0;i<b_atari.size();i++){
        block = Blocks[b_atari[i].i][b_atari[i].j];
        if(ko.flag && ko.i==block->atari.i && ko.j==block->atari.j)
            continue;
        for(c=0;c<(block->size * capture_coeff);c++)
          v.push_back(block->atari);
      }
    else
      for(int i=0;i<w_atari.size();i++){
        block = Blocks[w_atari[i].i][w_atari[i].j];
        if(ko.flag && ko.i==block->atari.i && ko.j==block->atari.j)
            continue;
        for(c=0;c<(block->size * capture_coeff);c++)
          v.push_back(block->atari);
      }
}

void StateGo::apply(DataGo d)
{
    assert(d.player == turn);
    num_movs++;
    if(num_movs == int(_size*_size * 0.4)){
        w_mov.insert(POS(PASSI,PASSI));
        b_mov.insert(POS(PASSI,PASSI));
    }
    if(ko.flag)
      if(turn==White){
        if(no_suicide(ko.i,ko.j,White))
          w_mov.insert(POS(ko.i,ko.j));
      }else{
        if(no_suicide(ko.i,ko.j,Black))
          b_mov.insert(POS(ko.i,ko.j));
      }
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
        INDEX i=d.i,j=d.j,k,l;
        bool ko_flag=true;
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
                if(Blocks[k][l]->size==1 && ko_flag){
                    ko.i=k;ko.j=l;ko.player=CHANGE_PLAYER(turn);ko.flag=1;
                }
                else
                    ko.flag=0;
                ko_flag=false;
                bool flagi= (Blocks[k][l]->size==1);
                remove_atari_block(k,l);
                delete Blocks[k][l];
                eliminate_block(Blocks[k][l],k,l);
                if(flagi){
                  if(!no_suicide(k,l,White))
                    w_mov.remove(POS(k,l));
                  if(!no_suicide(k,l,Black))
                    b_mov.remove(POS(k,l));
                }
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
              remove_atari_block(i,j);
              first=false;
            }
            else{
              remove_atari_block(k,l);
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
               add_atari_block(k,l);
            }
        }
        );
        if(is_block_in_atari(i,j,t_i,t_j)){
            Blocks[i][j]->atari=DataGo(t_i,t_j,CHANGE_PLAYER(turn));
            add_atari_block(i,j);
            update_mov(t_i,t_j);
        }
        else
            if(Blocks[i][j]->is_atari()){
              INDEX a=Blocks[i][j]->atari.i,b=Blocks[i][j]->atari.j;
              Blocks[i][j]->no_atari();
              update_mov(a,b);
              remove_atari_block(i,j);
            }
        //Refresh possible moves.
        FOR_EACH_ADJ(i,j,k,l,
        {
            if(Stones[k][l]==Empty){
              if(!no_suicide(k,l,White))
                w_mov.remove(POS(k,l));
              if(!no_suicide(k,l,Black))
                b_mov.remove(POS(k,l));
            }
        }
        );
        for(int x=0;x<c;x++){
          k=b[x]->atari.i;
          l=b[x]->atari.j;
          update_mov(k,l);
        }
        //Finish checking for ko position.
        ko_flag=true;
        if(ko.flag){
            FOR_EACH_ADJ(ko.i,ko.j,k,l,
            {
              if(Blocks[k][l]->is_atari()){
                if(!ko_flag || Blocks[k][l]->size>1)
                    ko.flag=0;
                ko_flag=false;
              }
            }
            );
        }
        if(ko.flag)
          if(turn==White)
            b_mov.remove(POS(ko.i,ko.j));
          else
            w_mov.remove(POS(ko.i,ko.j));
        b_mov.remove(POS(i,j));
        w_mov.remove(POS(i,j));
    }
    turn=CHANGE_PLAYER(turn);
    last_mov=d;
}

inline void StateGo::update_mov(INDEX i,INDEX j)
{
    if(no_suicide(i,j,White))
      w_mov.insert(POS(i,j));
    else
      w_mov.remove(POS(i,j));
    if(no_suicide(i,j,Black))
      b_mov.insert(POS(i,j));
    else
      b_mov.remove(POS(i,j));
}

inline void StateGo::add_atari_block(INDEX i,INDEX j){
    if(Stones[i][j]==White)
        w_atari.push_back(POS(i,j));
    else
        b_atari.push_back(POS(i,j));
}

inline void StateGo::remove_atari_block(INDEX i,INDEX j){
    if(Stones[i][j]==White){
      for(int l=0;l<w_atari.size();l++)
        if(Blocks[w_atari[l].i][w_atari[l].j] == Blocks[i][j]){
          w_atari.erase(w_atari.begin()+l);
          l--;break;
        }
    }
    else
      for(int l=0;l<b_atari.size();l++){
        if(Blocks[b_atari[l].i][b_atari[l].j] == Blocks[i][j]){
          b_atari.erase(b_atari.begin()+l);
          l--;break;
        }
      }
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

