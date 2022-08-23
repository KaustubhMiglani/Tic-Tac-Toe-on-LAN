#include<bits/stdc++.h>
#include <iostream>
#include <algorithm>
#include <list>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
using namespace std;
#define PORT 9003



void send_msg(int sock,string s)
{ 
    int a=s.size();
    char f[75];
    for(int i=0;i<a;i++)
    {
        f[i]=s[i];
    }
    for(int i=a;i<75;i++)
    {
        f[a]='\0';
    }
    write(sock,f,sizeof(f));
}

bool send_int(int socket, int x)
{
    size_t left=sizeof(x);
    ssize_t rc;
    char * data=(char*)&x;
    while(left)
    {
        rc=write(socket, data+sizeof(x)-left,left);
        if(rc<=0) return false;
        left-=rc;
    }

    return true;
}


bool recv_int(int socket,int *x)
{
    size_t left=sizeof(*x);
    ssize_t rc;
    int ret;
    char *data=(char*)&ret;

    while(left){
        rc=read(socket,data+sizeof(ret)-left,left);
        if(rc <= 0) return false;
        left-=rc;
    }

    *x=ret;
    return true;
}


bool recieve(int sock,string &s)
{
    char ret[75];
    ssize_t rc;
    char *data=(char*)(ret);
    size_t left =sizeof(ret);
    while (left) {
        rc=read(sock,data + sizeof(ret) - left, left);
        if(rc<=0){return false;}
        left-=rc;
    }
    s="";
    for(int i=0;i<75;i++)
    {
        if(ret[i]=='\0')
        {
            break;
        }
        s+=ret[i];
    }
    return true;
}

void clear_queue(int sock)
{
    while(1)
    {
        struct pollfd x;
        x.fd=sock;
        x.events=POLLIN;
        int ret=poll(&x,1,1000);
        if(ret==-1)
        {
            break;
        }
        if(ret==0)
        {
            break;
        }
        string ign;
        recieve(sock,ign);
    }
}

struct TicTacToe
{
    char moves[3][3];
    int game_done=0;
    int whose_move=0;
    int sock_p1=-1;
    int sock_p2=-1;

    void GameInit()
    {
        whose_move=sock_p1;
    }
    void set_first(int x)
    {
        sock_p1=x;
    }
    void set_second(int x)
    {
        sock_p2=x;
    }
    void empty_all()
    {
        for(int i=0;i<3;i++)
        {
            for(int j=0;j<3;j++)
            {
                moves[i][j]=' ';
            }
        }
        game_done=0;
        sock_p1=-1;
        sock_p2=-1;
    }

    bool make_move(int r,int c,char x)
    {
        if(r>=3 || c>=3)
            {
                cout<<"Invalid Move\n";
                return false;
            }
        if(moves[r][c]!=' ')
            {
                cout<<"Invalid Move\n";
                return false;
            }
        if(game_done)
           {
               cout<<"Invalid Move\n";
               return false;
           }
        moves[r][c]=x;
        if(whose_move==sock_p1)
        {
            whose_move=sock_p2;
        }
        else
        {
            whose_move=sock_p1;
        }
        return true;
    }

    int who_won()
    {
        //1 if X won(current player)
        //-1 if O won(other player)
        //0 if draw
        //2 --> Nothing can be concluded yet
        for(int i=0;i<3;i++)
        {
            int won_row=1;
            for(int j=0;j<3;j++)
            {
                if(moves[i][j]!='X')
                    {
                        won_row=0;
                        break;
                    }
            }
            if(won_row){game_done=1;return 1;}
        }

        for(int i=0;i<3;i++)
        {
            int won_col=1;
            for(int j=0;j<3;j++)
            {
                if(moves[j][i]!='X')
                    {
                        won_col=0;
                        break;
                    }
            }
            if(won_col){game_done=1;return 1;}
        }

        int won_dmain=1;
        for(int i=0;i<3;i++)
        {
            if(moves[i][i]!='X')
            {
                won_dmain=0;
                break;
            }
        }
        if(won_dmain){game_done=1;return 1;}

        int won_other=1;
        for(int i=0;i<3;i++)
        {
            if(moves[i][2-i]!='X')
            {
                won_other=0;
                break;
            }
        }

        if(won_other){game_done=1;return 1;}

        for(int i=0;i<3;i++)
        {
            int won_row=1;
            for(int j=0;j<3;j++)
            {
                if(moves[i][j]!='O')
                    {
                        won_row=0;
                        break;
                    }
            }
            if(won_row){game_done=1;return -1;}
        }

        for(int i=0;i<3;i++)
        {
            int won_col=1;
            for(int j=0;j<3;j++)
            {
                if(moves[j][i]!='O')
                    {
                        won_col=0;
                        break;
                    }
            }
            if(won_col){game_done=1;return -1;}
        }

        won_dmain=1;
        for(int i=0;i<3;i++)
        {
            if(moves[i][i]!='O')
            {
                won_dmain=0;
                break;
            }
        }
        if(won_dmain){game_done=1;return -1;}

        won_other=1;
        for(int i=0;i<3;i++)
        {
            if(moves[i][2-i]!='O')
            {
                won_other=0;
                break;
            }
        }

        if(won_other){game_done=1;return -1;}

        int is_draw=1;
        for(int i=0;i<3;i++)
        {
            for(int j=0;j<3;j++)
            {
                if(moves[i][j]==' ')
                    {
                        is_draw=0;
                        break;
                    }
            }
        }
        if(is_draw){game_done=1;return 0;}
        return 2;      
    }

    void PrintBoard()
    {
        cout<<moves[0][0]<<"|"<<moves[0][1]<<"|"<<moves[0][2]<<"\n";
        for(int i=0;i<5;i++){cout<<'-';}
        cout<<"\n";

        cout<<moves[1][0]<<"|"<<moves[1][1]<<"|"<<moves[1][2]<<"\n";
        for(int i=0;i<5;i++){cout<<'-';}
        cout<<"\n";

        cout<<moves[2][0]<<"|"<<moves[2][1]<<"|"<<moves[2][2]<<"\n";
        
    }
};

string recieve(int sock)
{
    char ret[75];
    ssize_t rc;
    char *data=(char*)(ret);
    size_t left =sizeof(ret);
    while (left) {
        rc=read(sock,data + sizeof(ret) - left, left);
        left-=rc;
    }
    string s="";
    for(int i=0;i<75;i++)
    {
        if(ret[i]=='\0')
        {
            break;
        }
        s+=ret[i];
    }
    return s;
}


void Playgame(int sock)
{
    //send_int(sock,1);
    //return;
    //cout<<recieve(sock)<<"\n";
    //int xx;
    //recv_int(sock,xx);
    //cout<<xx<<"\n";
    //return;
    struct TicTacToe x;
    x.GameInit();
    x.empty_all();
    int d=0;
    while(1)
    {
        string a=recieve(sock);
        if(a=="CONN?")
        {
            if(d==0){
            send_msg(sock,"Yes");
            }
            d=0;
            continue;
        }
        if(a=="RET")
        {
            return;
        }
        //cout<<a<<"P\n";
        if(a!="DISCONN" && a!="MOVE" && a!="ENTER" && a!="WAIT")
        {
            cout<<a;
            continue;
        }
        if(a=="DISCONN")
        {
            exit(0);
            return;
        }
        if(a=="MOVE")
        {
            int i,j;
            recv_int(sock,&i);
            recv_int(sock,&j);
            string f=recieve(sock);
            assert(f.size()==1);
            char op=f[0];
            x.make_move(i,j,op);
            continue;
        }
        if(a=="ENTER")
        {
            send_msg(sock,"Yes");
            d=1;
            x.PrintBoard();
            cout<<"Enter your row i and column j(1 indexed)"<<"\n";
            int i,j;
            cin>>i>>j;
            i--;
            j--;
            send_msg(sock,"MOVE");
            send_int(sock,i);
            send_int(sock,j);
            continue;
        }
        if(a=="WAIT")
        {
            x.PrintBoard();
            cout<<"Waiting for opponent to make move..\n";
            continue;
        }
        cout<<a;
    }
}
int main(int argc,char *argv[])
{
    if(argc!=2)
    {
        cout<<"Arguments not properly formatted\n";
        cout<<"Make sure you are entering the IP address as well\n";
        exit(0);
    }
    struct hostent *hoste;
    hoste=gethostbyname(argv[1]);
    if(hoste==NULL)
    {
        cout<<"Host not found\n";
        exit(0);
    }

	struct sockaddr_in serv_addr;
    int sock;
    memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr=*((struct in_addr *)(hoste)->h_addr); 
	serv_addr.sin_family=AF_INET;
	sock=socket(AF_INET, SOCK_STREAM, 0);
    if(sock<0)
    {
        cout<<"Socket could not be created\n";
        exit(0);
    }
	int connection=connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
	if (connection<0)
	{	
        cout<<"Could not connect to server\n";
        exit(0);
	}
    cout<<"Connected to server\n";
    /*cout<<"Please enter your name\n";
    char names[1024];
    scanf("%s",names);
    char to_send[1024];
    snprintf(to_send,1024,"%s\n",names);
    write(sock, to_send, strlen(to_send));*/
    Playgame(sock);
    clear_queue(sock);
    while(1){
    cout<<"Do you want to play again?\n";
    string ans;
    cin>>ans;
    send_msg(sock,ans);
    string x;
    recieve(sock,x);
    //cout<<x<<"Ko\n";
    if(x=="PRO")
    {
        Playgame(sock);  
        clear_queue(sock);
        continue;     
    }
    else
    {
        break;
    }
  }
    return 0;
}