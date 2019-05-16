#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <utility>
#include <algorithm>
#include <unordered_map>
#define INFINITE 1000000000 
#define MAXDEPTH 3
#define GRID 15
#define ANTI(x) (3-x) 

using namespace std;
typedef pair<int,int> location;
struct pattern{
    int n;
    int a[10];
    int val;
};
struct score{
    int horizon[GRID];//水平
    int vertical[GRID];//垂直
    int leftDiagonal[2*GRID-1];//左对角线
    int rightDiagonal[2*GRID-1];//右对角线
};
int map[GRID][GRID];//棋盘，0代表没有棋子，1代表黑子，2代表白子
vector<location> chessList;//双方所有落子位置的记录
pattern p[30];//所有的模式定义
int patterNum;//所有模式的数目
//函数声明：
void definePattern();
int maxValue(int depth,int alpha,int beta,int &x,int &y,score s);
int minValue(int depth,int alpha,int beta,int &x,int &y,score s);
bool insideBoard(int x,int y);
bool terminal(int player);
bool prun(int x,int y);
int getState(int x,int y,int dir,int dist,int player);
void search(int &x,int &y);
bool decideByRule(int &x,int &y);
void logInFile();
int evaluateInc(int x,int y,score &s);
int evaluateLine(int x,int y,int k,int player);
void evaluateGlobal(score &s);
int sum(score &s);

//函数实现：
//模式定义
void definePattern(){
    //注意：对称的模式的权重减半，因为它可以从两个方向计算
    patterNum=17;
    p[8]={5,{1,1,1,1,1},4000000};//连五

    p[1]={6,{0,1,1,1,1,0},500000};//活四

    p[2]={6,{0,1,1,1,1,2},100000};//死四
    p[3]={5,{1,0,1,1,1},100000};//死四
    p[4]={5,{1,1,0,1,1},50000};//死四

    p[5]={5,{0,1,1,1,0},40000};//活三
    p[6]={6,{0,1,0,1,1,0},80000};//活三
    p[7]={6,{0,1,1,0,1,0},80000};//活三    

    p[9]={5,{0,1,1,1,2},10000};//死三
    p[10]={6,{0,1,1,0,1,2},10000};//死三
    p[11]={5,{1,0,1,0,1},5000};//死三
    p[12]={6,{0,1,0,1,1,2},10000};//死三
    p[13]={5,{1,0,0,1,1},10000};//死三
    p[14]={7,{2,0,1,1,1,0,2},5000};//死三

    p[15]={6,{0,0,1,1,0,0},3000};//活二
    p[16]={5,{0,1,0,1,0},3000};//活二
    p[0]={6,{0,1,0,0,1,0},3000};//活二
}

//判断该点是否在棋盘内
bool insideBoard(int x,int y){
    return (x>=0) && (x<GRID) && (y>=0) && (y<GRID);
}

//判断棋局是否终止，即判断是否有连续的五个棋子，参数player为1表示判断连续的黑子，为2判断白子
bool terminal(int player){
    int dx[]={1,0,1,1};
    int dy[]={0,1,1,-1};
    //棋子合法的延伸方向，右、下、右下、右上（剩下的4个方向是对称的）
    for (int i=0;i<GRID;i++){
        for (int j=0;j<GRID;j++){
            if (map[i][j]==player){
                for (int d=0;d<4;d++){
                    if (insideBoard(i-dx[d],j-dy[d]) && map[i-dx[d]][j-dy[d]]==player) continue;
                    //如果前一个棋子已经是本方的，那么不需要从该棋子开始进行检查，因为检查一定不通过
                    bool temp=insideBoard(i+4*dx[d],j+4*dy[d]) && (map[i+dx[d]][j+dy[d]]==player) 
                                && (map[i+2*dx[d]][j+2*dy[d]]==player) && (map[i+3*dx[d]][j+3*dy[d]]==player) 
                                && (map[i+4*dx[d]][j+4*dy[d]]==player);
                    //检查该方向后续的4枚棋子是否都是本方的
                    if (temp) return true;
                }    
            }
        }
    }
    return false;
}

//对当前局面进行全局评估
void evaluateGlobal(score &s){
    int k1=2,k2=-3;
    int x=0,y=0;
    for (y=0;y<GRID;y++){
        s.horizon[y]=k1*(evaluateLine(x,y,1,1)+evaluateLine(x,y,6,1))+k2*(evaluateLine(x,y,1,2)+evaluateLine(x,y,6,2));
        s.leftDiagonal[x-y+GRID-1]=k1*(evaluateLine(x,y,2,1)+evaluateLine(x,y,7,1))+k2*(evaluateLine(x,y,2,2)+evaluateLine(x,y,7,2));
        s.rightDiagonal[x+y]=k1*(evaluateLine(x,y,0,1)+evaluateLine(x,y,5,1))+k2*(evaluateLine(x,y,0,2)+evaluateLine(x,y,5,2));
    }
    y=0;
    for (x=0;x<GRID;x++){
        s.vertical[x]=k1*(evaluateLine(x,y,3,1)+evaluateLine(x,y,4,1))+k2*(evaluateLine(x,y,3,2)+evaluateLine(x,y,4,2));
    }
    y=0;
    for (x=1;x<GRID;x++){
        s.leftDiagonal[x-y+GRID-1]=k1*(evaluateLine(x,y,2,1)+evaluateLine(x,y,7,1))+k2*(evaluateLine(x,y,2,2)+evaluateLine(x,y,7,2));
    }
    y=GRID-1;
    for (x=1;x<GRID;x++){
        s.rightDiagonal[x+y]=k1*(evaluateLine(x,y,0,1)+evaluateLine(x,y,5,1))+k2*(evaluateLine(x,y,0,2)+evaluateLine(x,y,5,2));
    }
}

//对结构体score求和
int sum(score &s){
    int val=0;
    for (int i=0;i<GRID;i++){
        val+=s.horizon[i];
        val+=s.vertical[i];
    }
    for (int i=0;i<2*GRID-1;i++){
        val+=s.leftDiagonal[i];
        val+=s.rightDiagonal[i];
    }
    return val;
}

//增量评估，修改(x,y)的四个方向上的评分
int evaluateInc(int x,int y,score &s){
    int k1=2,k2=-3;
    s.horizon[y]=k1*(evaluateLine(x,y,1,1)+evaluateLine(x,y,6,1))+k2*(evaluateLine(x,y,1,2)+evaluateLine(x,y,6,2));
    s.vertical[x]=k1*(evaluateLine(x,y,3,1)+evaluateLine(x,y,4,1))+k2*(evaluateLine(x,y,3,2)+evaluateLine(x,y,4,2));
    s.leftDiagonal[x-y+GRID-1]=k1*(evaluateLine(x,y,2,1)+evaluateLine(x,y,7,1))+k2*(evaluateLine(x,y,2,2)+evaluateLine(x,y,7,2));
    s.rightDiagonal[x+y]=k1*(evaluateLine(x,y,0,1)+evaluateLine(x,y,5,1))+k2*(evaluateLine(x,y,0,2)+evaluateLine(x,y,5,2));
    return sum(s)+rand()%1000;//随机数可以增加落子的随即性
}

//站在某一方的角度上，评估一条线上的评分
int evaluateLine(int x,int y,int k,int player){
    int dx[]={1,1,1,0,0,-1,-1,-1};//右下，右，右上，上，下，左上，左，左下
    int dy[]={-1,0,1,1,-1,1,0,-1};//右下，右，右上，上，下，左上，左，左下
    int value=0;
    while (insideBoard(x,y)){
        x=x-dx[k];
        y=y-dy[k];
    }
    x=x+dx[k];
    y=y+dy[k];
    //回退到这条线的起点
    while (insideBoard(x,y)){
        for (int i=0;i<patterNum;i++){
            int n=p[i].n;
            if (!insideBoard(x+dx[k]*(n-1),y+dy[k]*(n-1))) continue;
            bool flag=true;
            for (int j=0;j<n;j++){
                if (getState(x,y,k,j,player)!=p[i].a[j]){
                    flag=false;
                    break;
                }
            }//短路运算
            if (flag) value+=p[i].val;
        }//搜索所有的模式
        x=x+dx[k];
        y=y+dy[k];
    }
    return value;
}


//剪枝函数，如果某个点与之前下的10个点的距离都超过2，那么剪去下在该点的分支
bool prun(int x,int y){
    bool ans=true;
    int n=chessList.size();
    for (int i=max(n-10,0);i<n;i++){
        int t=max(abs(chessList[i].first-x),abs(chessList[i].second-y));
        if (t<=2) return false;
    }
    return true;
}

//求取所有后继节点评价的最大值
int maxValue(int depth,int alpha,int beta,int &x,int &y,score s){
    int bestx,besty;
    if (terminal(2)) return -100000000/depth;
    int n=chessList.size();
    int value=evaluateInc(chessList[n-1].first,chessList[n-1].second,s);
    if (depth>MAXDEPTH) return value;
    int v=-INFINITE;
    int m[]={7,8,6,9,5,10,4,11,3,12,2,13,1,14,0};
    for (int h=0;h<GRID;h++){
        int i=m[h];
        for (int j=0;j<GRID;j++){
            if (map[i][j]==0 && !prun(i,j)){//可行的后继
                map[i][j]=1;
                chessList.push_back(location(i,j));
                int tx,ty,t=minValue(depth+1,alpha,beta,tx,ty,s);
                chessList.pop_back();
                map[i][j]=0;
                //递归回溯 
                if ( (v<t) || (v==t && (rand()%4==0))){
                    v=t;
                    bestx=i;
                    besty=j;
                }              
                if (v>=beta) return v;
                alpha=max(alpha,v);                
            }
        }
    }
    x=bestx;
    y=besty;
    return v;
}

//求取所有后继节点评价的最小值
int minValue(int depth,int alpha,int beta,int &x,int &y,score s){
    int bestx,besty;
    if (terminal(1)) return 100000000/depth;
    int n=chessList.size();
    int value=evaluateInc(chessList[n-1].first,chessList[n-1].second,s);
    if (depth>MAXDEPTH) return value;
    int v=INFINITE;
    int m[]={7,8,6,9,5,10,4,11,3,12,2,13,1,14,0};
    for (int h=0;h<GRID;h++){
        int i=m[h];
        for (int j=0;j<GRID;j++){
            if (map[i][j]==0 && !prun(i,j)){//可行的后继
                map[i][j]=2;
                chessList.push_back(location(i,j));
                int tx,ty,t=maxValue(depth+1,alpha,beta,tx,ty,s);
                chessList.pop_back();              
                map[i][j]=0;
                //递归回溯
                if ( (v>t) || (v==t && (rand()%4==0))){
                    v=t;
                    bestx=i;
                    besty=j;
                }
                if (v<=alpha) return v;
                beta=min(beta,v);                
            }
        }
    }
    x=bestx;
    y=besty;
    return v;
}

//得到以（x，y）为中心，方向为dir，距离为dist的位置的状态。
//player参数：1是黑子，2是白子。
//0是空格，1是己方棋子，2是对方棋子或越界。
int getState(int x,int y,int dir,int dist,int player){
    int dx[]={1,1,1,0,0,-1,-1,-1};
    int dy[]={-1,0,1,1,-1,1,0,-1};
    int nx=x+dx[dir]*dist,ny=y+dy[dir]*dist;
    if (!insideBoard(nx,ny) || map[nx][ny]==ANTI(player)) return 2;
    if (map[nx][ny]==0) return 0;
    else return 1; 
}

//在某些情况下，可以使用简单的规则进行判别，得到应该落子的位置，减少运算量
bool decideByRule(int &x,int &y,score s){
    int dx[]={1,1,1,0,0,-1,-1,-1};
    int dy[]={-1,0,1,1,-1,1,0,-1};
    int priority=0;
    for (int i=0;i<GRID;i++){
        for (int j=0;j<GRID;j++){
            if (map[i][j]==0){
                for (int k=0;k<8;k++){
                    int t1=1,t2=1;
                    while (getState(i,j,k,t1,1)==1) t1++;
                    while (getState(i,j,k,-t2,1)==1) t2++;
                    if (t1+t2>=6 && priority<4){
                        priority=4;
                        x=i;
                        y=j;
                    }//我方连续四个子,优先级为4
                    int t3=1,t4=1;
                    while (getState(i,j,k,t3,2)==1) t3++;
                    while (getState(i,j,k,-t4,2)==1) t4++;
                    if (t3+t4>=6 && priority<3){
                        priority=3;
                        x=i;
                        y=j;
                    }//对方连续四个子,优先级为3
                    if (t1+t2>=5 && getState(i,j,k,t1,1)==0 && getState(i,j,k,-t2,1)==0 && priority<2){
                        priority=2;
                        x=i;
                        y=j;
                    }//我方连续三个子，优先级为2
                    if (t3+t4>=5 && getState(i,j,k,t3,2)==0 && getState(i,j,k,-t4,2)==0 && priority<1){
                        priority=1;
                        int val=-INFINITE;
                        map[i][j]=1;
                        score news=s;
                        int eva=evaluateInc(i,j,news);
                        if (val<eva){
                            val=eva;
                            x=i;
                            y=j;
                        }
                        map[i][j]=0;
                        map[i+t3*dx[k]][j+t3*dy[k]]=1;
                        news=s;
                        eva=evaluateInc(i+t3*dx[k],j+t3*dy[k],news);
                        if (val<eva){
                            val=eva;
                            x=i+t3*dx[k];
                            y=j+t3*dy[k];
                        }
                        map[i+t3*dx[k]][j+t3*dy[k]]=0;
                        map[i-t4*dx[k]][j-t4*dy[k]]=1;
                        news=s;
                        eva=evaluateInc(i-t4*dx[k],j-t4*dy[k],news);
                        if (val<eva){
                            val=eva;
                            x=i-t4*dx[k];
                            y=j-t4*dy[k];
                        }
                        map[i-t4*dx[k]][j-t4*dy[k]]=0;                       
                    }//对方连续三个子，优先级为1
                }
            }            
        }
    }
    return (priority>0);
}

//搜索合适的落子位置
void search(int &x,int &y){
    score s;
    evaluateGlobal(s);
    if (decideByRule(x,y,s))  return;   
    maxValue(1,-INFINITE,INFINITE,x,y,s);
}

//将对局及结果都记录在文件中
void logInFile(){
    ofstream fout("output.txt");
    if (!fout){
        cout<<"Cannot write into the log."<<endl;
        return;
    }
    fout<<"AI\tME\r\n";
    int n=chessList.size();
    for (int i=0;i<n;i=i+2){
        int x=chessList[i].first;
        int y=chessList[i].second;
        fout<<"["<<x<<","<<y<<"]\t";
        if (i+1<n){
            int x=chessList[i+1].first;
            int y=chessList[i+1].second;
            fout<<"["<<x<<","<<y<<"]\r\n";
        }
    }
    if (terminal(2)){
        fout<<"\r\nAI Lose!"<<endl;
    }
    else if (terminal(1)){
        fout<<"\r\nAI Win!"<<endl;
    }
    else{
        fout<<"\r\nDraw!"<<endl;
    }
    fout.close();
}

//主函数
int main(){
    definePattern();
    cout<<"7 7\r\n";
    fflush(stdout);
    map[7][7]=1;
    chessList.push_back(location(7,7));
    //第一步默认下在棋盘中央
    while (true){
        char s[10]; 
        fgets(s,10,stdin);
        if (strstr(s,"exit")!=NULL){
            break;
        }//用户输入exit主动结束程序
        int x,y;
        sscanf(s,"%d %d",&x,&y);
        map[x][y]=2;
        chessList.push_back(location(x,y));
        //读入用户（白子）落子的位置
        if (terminal(2)){
            cout<<"AI Lose!"<<endl;           
            break;
        }//白子落子后游戏结束，说明白子胜利       
        search(x,y);   
        map[x][y]=1;
        chessList.push_back(location(x,y));
        cout<<x<<" "<<y<<endl;
        fflush(stdout);      
        //使用alpha-beta搜索得到AI(黑子)落子的位置
        if (terminal(1)){
            cout<<"AI Win!"<<endl;            
            break;
        }//黑子落子后游戏结束，说明黑子胜利
    }
    logInFile();
    return 0;
}
