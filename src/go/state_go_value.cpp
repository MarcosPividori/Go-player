
#define FLAG_B    0x80000000
#define FLAG_W    0x40000000
#define FLAG_BOTH (FLAG_B | FLAG_W)
#define NO_FLAG   0x3fffffff

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

unsigned int StateGo::count_area(bool **visited,INDEX i,INDEX j)
{
    unsigned int res=1,v;
    visited[i][j]=true;
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

