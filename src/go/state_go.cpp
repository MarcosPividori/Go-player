
#include "state_go.hpp"
#include <iomanip>
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
            { \
              if(i>0)      { k=i-1;l=j;Code;} \
              if(i<_size-1){ k=i+1;l=j;Code;} \
              if(j>0)      { k=i;l=j-1;Code;} \
              if(j<_size-1){ k=i;l=j+1;Code;} \
            })

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

// RECURSIVE ELIMINATING (DFS)
void StateGo::eliminate_block(Block *block,INDEX i,INDEX j)
{
    Blocks[i][j]=NULL;
    if(Stones[i][j]==Black)
        captured_b++;
    else
        captured_w++;
    Stones[i][j]=Empty;
    b_mov.insert(POS(i,j));
    w_mov.insert(POS(i,j));
    INDEX k,l;
    FOR_EACH_ADJ(i,j,k,l,
    {
      if(Blocks[k][l]==block)//If same block, propagate.
        eliminate_block(block,k,l);
      else
        if(Stones[k][l]!=Empty){//Add a free adjacency.
          Blocks[k][l]->adj+=1;
          if(Blocks[k][l]->is_atari()){
            INDEX a=Blocks[k][l]->atari.i;
            INDEX b=Blocks[k][l]->atari.j;
            Blocks[k][l]->no_atari();
            update_mov(a,b);
            remove_atari_block(k,l);
          }
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

DataGo StateGo::look_for_delete_atari(Block *block,Block *flag,INDEX i,INDEX j,int &max_size)
{
    Blocks[i][j]= flag;
    INDEX k,l;
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

void StateGo::get_possible_moves(std::vector<DataGo>& v)
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

void StateGo::get_atari_escape_moves(std::vector<DataGo>& v)
{
    if(IS_PASS(last_mov))
      return;
    INDEX k,l,m,n;
    int sum=0,c=0,count,size;
    std::vector<POS> *atari_blocks;
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
               || remove_opponent_block_and_no_ko(k,l)){
                for(int i=0;i<pattern_coeff;i++)
                  v.push_back(DataGo(k,l,turn));
            }
          Stones[k][l]=Empty;
        }
}

void StateGo::get_capture_moves(std::vector<DataGo>& v)
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

bool StateGo::is_completely_empty(INDEX i,INDEX j)
{
    for(int k=MAX(i-1,0);k<= MIN(_size-1,i+1);k++)
      for(int l=MAX(j-1,0);l<= MIN(_size-1,j+1);l++)
        if(Stones[k][l]!=Empty)
            return false;
    return true;
}

bool StateGo::is_useful_move(DataGo mov)
{
    if(IS_PASS(mov))
        return true;
    return (no_self_atari_nor_suicide(mov.i,mov.j,turn)
         || remove_opponent_block_and_no_ko(mov.i,mov.j));
}

ValGo StateGo::get_final_value()
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
            else 
                if(!japanese_rules)
                  if(Stones[i][j]==White)
                    countw++;
                  else 
                    if(Stones[i][j]==Black)
                      countb++;
    for(int i=0;i<_size;i++)
        delete[] visited[i];
    delete[] visited;
    if(japanese_rules){
        countb=float(numb)-float(captured_b);
        countw=float(numw)+_komi-float(captured_w);
    }
    else{
        countb+=float(numb);
        countw+=float(numw)+_komi;
    }
    return countw-countb; 
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

inline void StateGo::add_atari_block(INDEX i,INDEX j){
    if(Stones[i][j]==White)
        w_atari.push_back(POS(i,j));
    else
        b_atari.push_back(POS(i,j));
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
#ifdef DEBUG
    std::cout<<std::endl<<std::endl;
    std::cout<<"WHITE MOVES:"<<std::endl;
    std::cout<<"   ";
    for(int i=0;i<_size;i++)
        printf(" %c",'A'+i+(i>7));
    for(int i=_size-1;i>=0;i--){
        printf("\n%2d ",i+1);
        for(int j=0;j<_size;j++){
            c='-';
            for(int k=0;k<w_mov.size();k++)
               if(w_mov[k].i == i && w_mov[k].j==j)
                 {c='O';break;}
            printf(" %c",c);
        }
    }
    std::cout<<std::endl<<std::endl;
    std::cout<<"BLACK MOVES:"<<std::endl;
    printf("   ");
    for(int i=0;i<_size;i++)
        printf(" %c",'A'+i+(i>7));
    for(int i=_size-1;i>=0;i--){
        printf("\n%2d ",i+1);
        for(int j=0;j<_size;j++){
            c='-';
            for(int k=0;k<b_mov.size();k++)
               if(b_mov[k].i == i && b_mov[k].j==j)
                 {c='X';break;}
            printf(" %c",c);
        }
    }
#endif
}

void StateGo::show(){
    show(stdout);
}

#ifdef DEBUG
void StateGo::debug(){
    std::cout<<"STATE VALUE: "<<get_final_value()<<std::endl<<std::endl;
    std::cout<<"BLOCKS:"<<std::endl;
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(Blocks[i][j]==NULL)
              std::cout<<std::setw(5)<<"----";
            else
              std::cout<<std::setw(5)<<(((long) Blocks[i][j])%10000);
        std::cout<<std::endl;
    }
    std::cout<<std::endl<<"BLOCKS'S VALUES:"<<std::endl;
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(Blocks[i][j] == NULL)
              std::cout<<std::setw(3)<<"--";
            else
              std::cout<<std::setw(3)<<(Blocks[i][j]->adj);
        std::cout<<std::endl;
    }
    std::cout<<std::endl;
}
#endif

inline bool StateGo::remove_opponent_block_and_no_ko(INDEX i,INDEX j)
{
    if(ko.flag && ko.i==i && ko.j==j)
      return false;
    Player opp=CHANGE_PLAYER(turn);
    INDEX k,l;
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

inline bool StateGo::no_suicide(INDEX i,INDEX j,Player p)
{
    INDEX k,l;
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
    INDEX k,l;
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

