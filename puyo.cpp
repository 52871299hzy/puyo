#include <bits/stdc++.h>
#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
inline void Delay(long long t)
{
	Sleep(t);
}
inline void Cls()
{
	system("cls");
}
inline void initcon()
{
	Cls();
}
inline void resetcon()
{
	system("pause");
}
#define getchar getch
#else
#include <unistd.h>
inline void Delay(long long t)
{
	usleep(t*1000);
}
inline void Cls()
{
	system("clear");
}
inline void initcon()
{
	system("stty -icanon");	
	Cls();
}
inline void resetcon()
{
	system("stty icanon");
}
#endif
//TODO:cross-platform support
using namespace std;
const int atkpow[]={0,0,8,16,32,64,96,128,160,192,224,256,288,320,352,384,416,448,480,512,544,576,608,640,672};
const int grpbns[]={0,0,0,0,0,2,3,4,5,6,7,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10};
const int colbns[]={0,0,3,6,12,24};
const int colors=4;
const int dx[]={1,0,-1,0},dy[]={0,1,0,-1};
struct player
{
	bool alive;
	int board[20][20];//number from bottom to top, left to right;color:0(empty),1-5,-1(nuisance)
	double TP;//target point
	int vis[20][20],grp[80];//use in DFS
	double NL;//leftover nuisance points
	int gartotal,gardrop;//total garbage & dropping garbage
	int lstchn;//last chain length made
	int score;//score
	int seed;
	int nowpiece,nxtpiece,nnxtpiece;//e.g. 11,12,34 (first is center)
	int curpos,curst;//same as in drop(),urdl
	player()
	{
		memset(board,0,sizeof(board));
		TP=0;
		NL=0;
		gartotal=gardrop=lstchn=0;
		score=0;
		alive=0;
	}
	void print()
	{
		Cls();
		char buf[30][40]={};
		for(int i=1;i<=20;i++)
		{
			for(int j=1;j<38;j++)
			{
				buf[i][j]=' ';
			}
		}
		int a=nowpiece/10,b=nowpiece%10;
		if(nowpiece)
		{
			buf[2][curpos+1]=a+'0';
			buf[2-dx[curst]][curpos+dy[curst]+1]=b+'0';	
		}
		a=nxtpiece/10,b=nxtpiece%10;
		buf[3][10]=a+'0';
		buf[2][10]=b+'0';
		a=nnxtpiece/10,b=nnxtpiece%10;
		buf[3][12]=a+'0';
		buf[2][12]=b+'0';
		for(int i=5;i<=16;i++)
		{
			buf[i][1]=buf[i][8]='|';
		}
		for(int i=2;i<=7;i++)
		{
			buf[17][i]='-';
		}
		buf[4][1]=buf[4][8]=buf[17][1]=buf[17][8]='*';
		sprintf(buf[18]+1,"Score:%d",score);
		sprintf(buf[19]+1,"last:%dchain--%d",lstchn,gardrop);
		sprintf(buf[20]+1,"total garbage:%d",gartotal);
		sprintf(buf[21]+1,"seed:%d",seed);
		for(int i=1;i<=13;i++)
		{
			for(int j=1;j<=6;j++)
			{
				if(board[i][j])
				buf[17-i][j+1]=board[i][j]+'0';
			}
		}
		if(!alive)
		{
			sprintf(buf[22]+1,"%s","ばたんきゅー");
		}
		for(int i=1;i<=23;i++)
		{
			printf("%s\n",buf[i]+1);
		}
	}
	void gamestart(int sd)
	{
		alive=1;
		TP=70;
		if(sd==0)
		{
			srand(time(0));
			seed=rand()%1000000;
		}
		else seed=sd;
		srand(seed);
		int a,b,c,d;
		a=rand()%4+1;
		b=rand()%4+1;
		c=rand()%4+1;
		d=rand()%4+1;
		while(a+b+c+d==10&&a*b*c*d==24)
		{
			c=rand()%4+1;
			d=rand()%4+1;
		}
		nowpiece=a*10+b;
		nxtpiece=c*10+d;
		a=rand()%4+1;
		b=rand()%4+1;
		nnxtpiece=a*10+b;
		curpos=3;
		curst=0;
		print();
		char ch;
		while(alive&&(ch=getchar()))
		{
			switch(ch)
			{
				case 'w'://hard-drop
				drop(curpos,curst);
				curpos=3;
				curst=0;
				break;
				case 'a'://move left
				if(curpos-(curst==3)>=2)curpos--;
				break;
				case 'd'://move right
				if(curpos+(curst==1)<=5)curpos++;
				break;
				case 'j'://rotate clockwise
				if(curpos==6&&curst==0)
				{
					curpos--;
				}
				if(curpos==1&&curst==2)
				{
					curpos++;
				}
				curst=(curst+1)%4;
				break;
				case 'k'://rotate anti-clockwise
				if(curpos==1&&curst==0)
				{
					curpos++;
				}
				if(curpos==6&&curst==2)
				{
					curpos--;
				}
				curst=(curst+3)%4;
				break;
			}
			print();
		}
	}
	void gameover()
	{
		alive=0;
	}
	bool DFS(int x,int y,int no)//vis[][] index of connected components, returns 1 if can pop
	{
		vis[x][y]=no;
		grp[no]++;
		for(int i=0;i<4;i++)
		{
			int xx=x+dx[i],yy=y+dy[i];
			if(xx<=12&&xx>=1&&yy<=6&&yy>=1&&board[xx][yy]==board[x][y]&&!vis[xx][yy])
			{
				DFS(xx,yy,no);
			}
		}
		if(grp[no]>=4)return 1;
		return 0;
	}
	int chain(int cn)//given the number of chain, returns damage for this chain
	{
		double ans;
		int PC=0,CP=atkpow[cn],CB=0,GB=0,col=0;
		bool colused[10]={0,0,0,0,0,0,0,0,0,0};
		int factor=0;
		for(int i=1;i<=12;i++)
		{
			for(int j=1;j<=6;j++)
			{
				if(board[i][j]>0&&grp[vis[i][j]]>=4)
				{
					colused[board[i][j]]=1;
					GB+=grpbns[grp[vis[i][j]]];
					PC+=grp[vis[i][j]];
					grp[vis[i][j]]=-1;
					board[i][j]=0;
				}
				if(board[i][j]>0&&grp[vis[i][j]]==-1)
				{
					board[i][j]=0;
				}
			}
		}
		for(int i=1;i<=4;i++)
		{
			col+=colused[i]?1:0;
		}
		CB=colbns[col];
		factor=min(999,max(1,(CP+GB+CB)));
		int ascore=(10*PC)*factor;
		score+=ascore;
		ans=(double)ascore/TP+NL;
		NL=ans-(int)ans;
		return (int)ans;
	}
	bool search()
	{
		memset(vis,0,sizeof(vis));
		memset(grp,0,sizeof(grp));
		int tot=0;
		bool flag=0;
		for(int i=1;i<=12;i++)
		{
			for(int j=1;j<=6;j++)
			{
				if(board[i][j]>0&&!vis[i][j])
				{
					flag|=DFS(i,j,++tot);
				}
			}
		}
		return flag;
	}
	void gravity()
	{
		for(int i=1;i<=12;i++)
		{
			for(int j=1;j<=6;j++)
			{
				if(!board[i][j])
				{
					for(int k=i+1;k<=14;k++)
					{
						if(board[k][j])
						{
							board[i][j]=board[k][j];
							board[k][j]=0;
							break;
						}
					}
				}
			}
		}
	}
	void drop(int x,int st)//drop, st=0-3(u,r,d,l)
	{
		int a=nowpiece/10,b=nowpiece%10;
		switch(st)
		{
			case 0:
				board[12][x]=a;
				board[13][x]=b;
				break;
			case 1:
				board[13][x]=a;
				board[13][x+1]=b;
				break;
			case 2:
				board[13][x]=a;
				board[12][x]=b;
				break;
			case 3:
				board[13][x]=a;
				board[13][x-1]=b;
				break;
		}
		gravity();
		print();
		Delay(10);
		int chn=0;
		int atk=0;
		bool flag=0;
		nowpiece=0;
		while(search())
		{
			if(!flag)gardrop=0;
			flag=1;
			atk=chain(++chn);
			gravity();
			gardrop+=atk;
			gartotal+=atk;
			lstchn=chn;
			print();
			Delay(500);
		}
		if(board[12][3])
		{
			gameover();
			return;
		}
		nowpiece=nxtpiece;
		nxtpiece=nnxtpiece;
		a=rand()%4+1;
		b=rand()%4+1;
		nnxtpiece=10*a+b;
	}
}p1;
int main()
{
	int sed=0;
	initcon();
	cout<<"==========Puyo Puyo!==========\nMade by Bob\nPress any key...\nPS:You can press s to set seed.";
	if(getchar()=='s')
	{
		cout<<"\nInput seed:";
		cin>>sed;
	}
	p1.gamestart(sed);
	resetcon();
}