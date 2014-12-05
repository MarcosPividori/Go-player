
#include "pattern_list.hpp"
#include <cassert>

PatternList::PatternList(){
    for(int i=0;i<27000;i++)
      list[i]=false;
}

void PatternList::add(Pattern pattern){
    int sum=0;
    for(int i=0;i<3;i++)
      for(int j=0;j<3;j++){
        switch(pattern.x[i][j]){
          case p_empty: sum*=3;break;
          case p_white: sum*=3;sum+=1;break;
          case p_black: sum*=3;sum+=2;break;
          case p_everything:
                pattern.x[i][j]=p_empty;
                add(pattern);
                pattern.x[i][j]=p_white;
                add(pattern);
                pattern.x[i][j]=p_black;
                add(pattern);
                return;
          case p_no_white:
                pattern.x[i][j]=p_empty;
                add(pattern);
                pattern.x[i][j]=p_black;
                add(pattern);
                return;
          case p_no_black:
                pattern.x[i][j]=p_empty;
                add(pattern);
                pattern.x[i][j]=p_white;
                add(pattern);
                return;
          case p_border: break;
          default: assert(0);
        }
     }
    int border_sum=0;
    if(pattern.x[0][1]==p_border) border_sum+=1;
    if(pattern.x[1][0]==p_border) border_sum+=3;
    if(pattern.x[2][1]==p_border) border_sum+=2;
    if(pattern.x[1][2]==p_border) border_sum+=6;
    if(border_sum)
        sum+= 19683+border_sum*729;
    list[sum]=true;
}

void PatternList::mirror_pattern(Pattern &pattern){
    Pattern pat;
    for(int i=0;i<3;i++)
      for(int j=0,l=2;j<3;j++,l--)
        pat.x[i][l]=pattern.x[i][j];
    for(int i=0;i<3;i++)
      for(int j=0;j<3;j++)
        pattern.x[i][j]=pat.x[i][j];
}

void PatternList::rotate_pattern(Pattern &pattern){
    Pattern pat;
    for(int i=0,l=0;i<3;i++,l++)
      for(int j=0,k=2;j<3;j++,k--)
        pat.x[k][l]=pattern.x[i][j];
    for(int i=0;i<3;i++)
      for(int j=0;j<3;j++)
        pattern.x[i][j]=pat.x[i][j];
}

void PatternList::invert_pattern(Pattern &pattern){
    for(int i=0;i<3;i++)
      for(int j=0;j<3;j++)
        switch(pattern.x[i][j]){
          case p_empty: break;
          case p_white: pattern.x[i][j]=p_black;break;
          case p_black: pattern.x[i][j]=p_white;break;
          case p_everything: break;
          case p_no_white: pattern.x[i][j]=p_no_black;break;
          case p_no_black: pattern.x[i][j]=p_no_white;break;
          case p_border: break;
          default: assert(0);
        }
}

void PatternList::add_pattern(Pattern pattern){
    add(pattern);
    invert_pattern(pattern);
    add(pattern);
    rotate_pattern(pattern);
    add(pattern);
    invert_pattern(pattern);
    add(pattern);
    rotate_pattern(pattern);
    add(pattern);
    invert_pattern(pattern);
    add(pattern);
    rotate_pattern(pattern);
    add(pattern);
    invert_pattern(pattern);
    add(pattern);
}

bool PatternList::match(StateGo *state,INDEX i,INDEX j){
    int sum=0;
    for(int k=MAX(i-1,0);k<=MIN(state->_size-1,i+1);k++)
      for(int l=MAX(j-1,0);l<=MIN(state->_size-1,j+1);l++){
        sum*=3;
        switch(state->Stones[k][l]){
          case Empty: break;
          case White: sum+=1;break;
          case Black: sum+=2;break;
        }
      }
    int border_sum=0;
    if(i==0) border_sum+=1;
    else if(i==state->_size-1) border_sum+=2;
    if(j==0) border_sum+=3;
    else if(j==state->_size-1) border_sum+=6;
    if(border_sum)
        sum+= 19683+border_sum*729;
    return list[sum];
}

void PatternList::read_file(const char *file_name){
    char c;
    int i=0,j=0,k=0;
    Pattern pat;
    std::ifstream myfile;
    myfile.open(file_name);
    assert(myfile.is_open());
    while(!myfile.eof()){
        i=k/3;
        j=k%3;
        myfile>>c;
        switch(c){
          case '-': pat.x[i][j]=p_empty;k++;break;
          case 'w': pat.x[i][j]=p_white;k++;break;
          case 'b': pat.x[i][j]=p_black;k++;break;
          case 'x': pat.x[i][j]=p_everything;k++;break;
          case 'W': pat.x[i][j]=p_no_white;k++;break;
          case 'B': pat.x[i][j]=p_no_black;k++;break;
          case '*': pat.x[i][j]=p_border;k++;break;
          default:break;
        }
        if(k==9){
            add_pattern(pat);
            mirror_pattern(pat);
            add_pattern(pat);
            k=0;
        }
    }
    myfile.close();
}
