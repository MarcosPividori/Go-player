
void StateGo::show(FILE *output) const
{
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
    cerr<<endl<<endl;
    cerr<<"WHITE MOVES:"<<endl;
    cerr<<"   ";
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
    cerr<<endl<<endl;
    cerr<<"BLACK MOVES:"<<endl;
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

void StateGo::show() const
{
    show(stdout);
}

#ifdef DEBUG
void StateGo::debug() const
{
    cerr<<"STATE VALUE: "<<get_final_value()<<endl<<endl;
    cerr<<"BLOCKS:"<<endl;
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(Blocks[i][j]==NULL)
              cerr<<setw(5)<<"----";
            else
              cerr<<setw(5)<<(((long) Blocks[i][j])%10000);
        cerr<<endl;
    }
    cerr<<endl<<"BLOCKS'S VALUES:"<<endl;
    for(int i = _size-1;i>=0;i--){
        for(int j=0;j<_size;j++)
            if(Blocks[i][j] == NULL)
              cerr<<setw(3)<<"--";
            else
              cerr<<setw(3)<<(Blocks[i][j]->adj);
        cerr<<endl;
    }
    cerr<<endl;
}
#endif
