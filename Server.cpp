#include<bits/stdc++.h>
#include <iostream>
#include <algorithm>
#include <list>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <bits/local_lim.h>
#include <pthread.h>
#include <poll.h>
#define PORT 9003

using namespace std;

void clr_jus(int);
int cc=0;
map<int,int> conn_sock;
map<int,int> sock_name;
map<int,int> play_again;
vector<struct TicTacToe> games;
pthread_mutex_t games_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;
map<int,int> game_no;

void print_(string s)
{
    pthread_mutex_lock(&print_lock);
    cout<<s<<endl;
    pthread_mutex_unlock(&print_lock);   
}

void upd_log_1(int me,string s)
{
    if(me==1)
    {
        print_(s);
    }
}
bool is_alive(int sock)
{
    struct pollfd x;
    x.fd=sock;
    x.events=POLLIN;
    int ret=poll(&x,1,1000);
    if(ret==-1)
        {
            return false;
        }
    return true;
}

bool timeout(int sock)
{
    struct pollfd x;
    x.fd=sock;
    x.events=POLLIN;
    int ret=poll(&x,1,15000);
    if(ret==-1)
    {
        //cout<<"FFk\n";
        return true;
    }
    if(ret==0)
    {
        //cout<<"FFk\n";
        return true;
    }
    return false;
}

bool send_msg(int sock,string s)
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
    int done=write(sock,f,sizeof(f));
    if(done<=0)
    {
        return false;
    }
    return true;
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

void disconn_me(int other_player)
{
    send_msg(other_player,"Sorry, your partner disconnected\n");
    send_msg(other_player,"DISCONN");
}
struct TicTacToe
{
    char moves[3][3];
    int game_done=0;
    int whose_move=0;
    int sock_p1=-1;
    int sock_p2=-1;
    int time_out=0;//1 if timeout occured
    int disconn=0;

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
        time_out=0;
        disconn=0;
    }

    bool make_move(int r,int c,char x,int sock)
    {
        if(r>=3 || c>=3)
            {
                send_msg(sock,"Invalid Move\n");
                return false;
            }
        if(moves[r][c]!=' ')
            {
                send_msg(sock,"Invalid Move\n");
                return false;
            }
        if(game_done)
           {
               send_msg(sock,"Invalid Move\n");
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
        cout<<moves[0][0]<<"|"<<moves[0][1]<<"|"<<moves[0][2]<<endl;
        for(int i=0;i<5;i++){cout<<'-';}
        cout<<endl;

        cout<<moves[1][0]<<"|"<<moves[1][1]<<"|"<<moves[1][2]<<endl;
        for(int i=0;i<5;i++){cout<<'-';}
        cout<<endl;

        cout<<moves[2][0]<<"|"<<moves[2][1]<<"|"<<moves[2][2]<<endl;
        
    }
};

void StartGame(int cs)
{
    //int x;
    //send_msg(cs,"Hello??");
    //send_int(cs,3);
    //cout<<x<<"\n";
    //return;
    int game=game_no[cs];
    int to_ask=1;
    //0 --> recieve data from server
    while(games[game].sock_p1==-1 || games[game].sock_p2==-1)
    {

    }
    sleep(2);
    send_msg(cs,"Starting the game...\n");
    int my_player=1;
    int other_player=games[game].sock_p2;
    assert(other_player!=-1);
    if(games[game].sock_p2==cs)
    {
        my_player=2;
        other_player=games[game].sock_p1;
    }
    games[game].GameInit();
    int sent=0;
    string last_sent="";
    int ko=0;
    if(my_player==1)
    {
        print_("Game numbered "+to_string(game)+" started");
    }
    while(1)
    {
        if(games[game].time_out)
        {
            send_msg(cs,"Someone took too long to respond\n");
            send_msg(cs,"RET");
            upd_log_1(my_player,"Game "+to_string(game)+" closed because client took too long to respond");
            return;
        }
        if(games[game].disconn)
        {
            disconn_me(cs);
            upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
            pthread_exit(0);
        }  
        pthread_mutex_lock(&games_lock);
        if(games[game].disconn)
        {
            pthread_mutex_unlock(&games_lock);
            disconn_me(cs);
            upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
            pthread_exit(0);
        }
        if(games[game].game_done)
        {
            int x=games[game].who_won();
            if(x==-1)
            {
                if(my_player==1)
                {
                    send_msg(cs,"You lost\n");
                    send_msg(cs,"RET");
                    print_("Player "+to_string(sock_name[cs])+" lost! in game "+to_string(game));
                    //send_msg(other_player,"You won\n");
                    //send_msg(other_player,"DISCONN");
                    pthread_mutex_unlock(&games_lock);
                    return;
                }
                if(my_player==2)
                {
                    send_msg(cs,"You won\n");
                    send_msg(cs,"RET");
                    print_("Player "+to_string(sock_name[cs])+" won! in game "+to_string(game));
                    //send_msg(other_player,"You lost\n");
                    //send_msg(other_player,"DISCONN");
                    pthread_mutex_unlock(&games_lock); 
                    return;
                }

            }
            if(x==0)
            {
                send_msg(cs,"Its a draw!\n");
                print_("Player "+to_string(sock_name[cs])+" drawed! in game "+to_string(game));
                send_msg(cs,"RET");
                //send_msg(other_player,"Its a draw!\n");
                //send_msg(other_player,"DISCONN");
                pthread_mutex_unlock(&games_lock); 
                return;
            }
            if(x==1)
            {
                if(my_player==1)
                {
                    send_msg(cs,"You won\n");
                    send_msg(cs,"RET");
                    print_("Player "+to_string(sock_name[cs])+" won! in game "+to_string(game));
                    //send_msg(other_player,"You lost\n");
                    //send_msg(other_player,"DISCONN");
                    pthread_mutex_unlock(&games_lock);
                    return;
                }
                if(my_player==2)
                {
                    send_msg(cs,"You lost\n");
                    send_msg(cs,"RET");
                    print_("Player "+to_string(sock_name[cs])+" lost! in game "+to_string(game));
                    //send_msg(other_player,"You won\n");
                    //send_msg(other_player,"DISCONN");
                    pthread_mutex_unlock(&games_lock);
                    return;
                }
            }
        }
        pthread_mutex_unlock(&games_lock);
        if(games[game].disconn)
        {
            disconn_me(cs);
            upd_log_1(my_player,"Game "+to_string(game)+" closed becuase client disconnected");
            pthread_exit(0);
        }
        if(games[game].time_out){continue;}
        send_msg(cs,"CONN?");
        string xx;
        bool y=recieve(cs,xx);
        if(y==false)
        {
            //cout<<"FFG\n";
            //send_msg(other_player,"Sorry, your partner disconnected\n");
            //send_msg(other_player,"DISCONN");
            //cout<<cs<<"\n";
            games[game].disconn=1;
            upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
            pthread_exit(0);
        }
        if(games[game].game_done)
        {
            continue;
        }
        if(games[game].disconn)
        {
            disconn_me(cs);
            upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
            pthread_exit(0);
        }
        if(sent==0)
        {
            if(other_player!=games[game].whose_move)
            {
                int my_num=0;
                if(my_player==1)
                {
                    my_num=sock_name[games[game].sock_p1];
                }
                else
                {
                    my_num=sock_name[games[game].sock_p2];
                }
                cout<<"Player "+to_string(my_num)+" will make a move now"<<endl;
                cout<<"The current state of the game numbered "+to_string(game)+" is"<<endl;
                games[game].PrintBoard();
                if(last_sent!="ENTER")
                {
                    sleep(2);
                    games[game].who_won();
                    if(games[game].game_done){continue;}
                    if(games[game].time_out){continue;}
                    if(games[game].disconn){upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");disconn_me(cs);pthread_exit(0);}
                    bool x=send_msg(games[game].whose_move,"ENTER");
                    if(x==false)
                    {
                        /*cout<<"KKL\n";
                        send_msg(other_player,"Sorry, your partner disconnected\n");
                        send_msg(other_player,"DISCONN");*/
                        games[game].disconn=1;
                        upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
                        pthread_exit(0);
                    }
                    last_sent="ENTER";
                    sent=1;
                }
            }
            if(my_player==1 && games[game].sock_p2==games[game].whose_move)
            {
                if(last_sent!="WAIT")
                {
                    if(games[game].disconn){upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");disconn_me(cs);pthread_exit(0);}
                    if(games[game].time_out){continue;}
                    bool x=send_msg(games[game].sock_p1,"WAIT");
                    if(x==false)
                    {
                        /*cout<<"OPK";
                        send_msg(other_player,"Sorry, your partner disconnected\n");
                        send_msg(other_player,"DISCONN");*/
                        games[game].disconn=1;
                        upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
                        pthread_exit(0);
                    }
                    last_sent="WAIT";
                }
                sent=0;
            }
            else if(my_player==2 && games[game].sock_p1==games[game].whose_move)
            {
                if(last_sent!="WAIT")
                {
                    if(games[game].disconn){disconn_me(cs);upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");pthread_exit(0);}
                    if(games[game].time_out){continue;}
                    bool x=send_msg(games[game].sock_p2,"WAIT");
                    last_sent="WAIT";
                    if(x==false)
                    {
                       /* cout<<"MXY";
                        send_msg(other_player,"Sorry, your partner disconnected\n");
                        send_msg(other_player,"DISCONN");*/
                        games[game].disconn=1;
                        upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
                        pthread_exit(0);
                    }
                }
                sent=0;
            }
            continue;
        }
        if(games[game].time_out){continue;}
        int other_p=0;
        string val;
        bool is_p=timeout(cs);
        if(is_p)
            {
                games[game].time_out=1;
                send_msg(cs,"Someone took too long to respond\n");
                send_msg(cs,"RET");
                upd_log_1(my_player,"Game "+to_string(game)+" closed because client took too long to respond");
                clr_jus(cs);
                return;
            }
        bool rec=recieve(cs,val);
        if(rec==false)
        {
            /*cout<<"Pli";
            send_msg(other_player,"Sorry, your partner disconnected\n");
            send_msg(other_player,"DISCONN");*/
            games[game].disconn=1;
            upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
            pthread_exit(0);
        }
        if(val=="MOVE")
        {
            if(games[game].disconn)
            {
                disconn_me(cs);
                upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
                pthread_exit(0);
            }
            pthread_mutex_lock(&games_lock);
            int i,j;
            bool is_p=timeout(cs);
            if(is_p)
            {
                games[game].time_out=1;
                send_msg(cs,"Someone took too long to respond\n");
                send_msg(cs,"RET");
                upd_log_1(my_player,"Game "+to_string(game)+" closed because client took too long to respond");
                return;
            }
            bool rec=recv_int(cs,&i);
            if(rec==false)
            {
                /*cout<<"Ok";
                send_msg(other_player,"Sorry, your partner disconnected\n");
                send_msg(other_player,"DISCONN");*/
                games[game].disconn=1;
                upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
                pthread_mutex_unlock(&games_lock);
                pthread_exit(0);            
            }
            rec=recv_int(cs,&j);
            if(rec==false)
            {
                /*cout<<"GG";
                send_msg(other_player,"Sorry, your partner disconnected\n");
                send_msg(other_player,"DISCONN");*/
                games[game].disconn=1;
                upd_log_1(my_player,"Game "+to_string(game)+" closed because client disconnected");
                pthread_mutex_unlock(&games_lock);
                pthread_exit(0);
            }
           if(my_player==1)
           {
               bool done=games[game].make_move(i,j,'X',cs);
               if(done)
               {
                   send_msg(cs,"MOVE");
                   send_int(cs,i);
                   send_int(cs,j);
                   send_msg(cs,"X");
                   send_msg(other_player,"MOVE");
                   send_int(other_player,i);
                   send_int(other_player,j);
                   send_msg(other_player,"X");
                   games[game].who_won();
               }
               else
               {
                   last_sent="";
               }
           }
           else
           {
               bool done=games[game].make_move(i,j,'O',cs);
               if(done)
               {
                   send_msg(cs,"MOVE");
                   send_int(cs,i);
                   send_int(cs,j);
                   send_msg(cs,"O");
                   send_msg(other_player,"MOVE");
                   send_int(other_player,i);
                   send_int(other_player,j);
                   send_msg(other_player,"O");
                   games[game].who_won();
               }
               else
               {
                   last_sent="";
               }
           }
        pthread_mutex_unlock(&games_lock);
        sent=0;
        }

        if(games[game].game_done==0)
        {
            continue;
        }
        else
        {
            cout<<"The final state of game "+to_string(game)+" is"<<endl;
            games[game].PrintBoard();
        }
        pthread_mutex_lock(&games_lock);  
        if(games[game].game_done)
        {

            int x=games[game].who_won();
            if(x==-1)
            {
                if(my_player==1)
                {
                    send_msg(cs,"You lost\n");
                    send_msg(cs,"RET");
                    print_("Player "+to_string(sock_name[cs])+" lost! in game "+to_string(game));
                    //send_msg(other_player,"You won\n");
                    //send_msg(other_player,"DISCONN");
                    pthread_mutex_unlock(&games_lock);
                    return;
                }
                if(my_player==2)
                {
                    send_msg(cs,"You won\n");
                    send_msg(cs,"RET");
                    print_("Player "+to_string(sock_name[cs])+" won! in game "+to_string(game));
                    //send_msg(other_player,"You lost\n");
                    //send_msg(other_player,"DISCONN");
                    pthread_mutex_unlock(&games_lock); 
                    return;
                }

            }
            if(x==0)
            {
                send_msg(cs,"Its a draw!\n");
                send_msg(cs,"RET");
                print_("Player "+to_string(sock_name[cs])+" got a draw! in game "+to_string(game));
                //send_msg(other_player,"Its a draw!\n");
                //send_msg(other_player,"DISCONN");
                pthread_mutex_unlock(&games_lock); 
                return;
            }
            if(x==1)
            {
                if(my_player==1)
                {
                    send_msg(cs,"You won\n");
                    send_msg(cs,"RET");
                    print_("Player "+to_string(sock_name[cs])+" won! in game "+to_string(game));
                    //send_msg(other_player,"You lost\n");
                    //send_msg(other_player,"DISCONN");
                    pthread_mutex_unlock(&games_lock);
                    return;
                }
                if(my_player==2)
                {
                    send_msg(cs,"You lost\n");
                    send_msg(cs,"RET");
                    print_("Player "+to_string(sock_name[cs])+" lost! in game "+to_string(game));
                    //send_msg(other_player,"You won\n");
                    //send_msg(other_player,"DISCONN");
                    pthread_mutex_unlock(&games_lock);
                    return;
                }
            }
        }
        pthread_mutex_unlock(&games_lock);
    }
}
void CreateGame(int cs)
{
    pthread_mutex_lock(&games_lock);
    if(!games.empty())
        {
            int zz=games.size();
            if(games[zz-1].sock_p2==-1)
            {
                games[zz-1].set_second(cs);
                game_no[cs]=games.size();
                game_no[cs]--;
                cc++;
                sock_name[cs]=cc;
                send_msg(games[zz-1].sock_p2,"You are Player "+ to_string(cc)+".\nAnother person is already present\n");
                send_msg(games[zz-1].sock_p2,"Your symbol is O\n");           
                send_msg(games[zz-1].sock_p1,"Another player has joined\n");
                //StartGame();
                pthread_mutex_unlock(&games_lock);
                return;
            }
        }
    struct TicTacToe x;
    x.empty_all();
    cc++; 
    sock_name[cs]=cc;
    send_msg(cs,"You are Player "+to_string(cc)+"\n");
    send_msg(cs,"Your symbol will be X\n");
    send_msg(cs,"Waiting for Another Player\n");
    x.sock_p1=cs;
    games.push_back(x);
    game_no[cs]=games.size();
    game_no[cs]--;
    pthread_mutex_unlock(&games_lock);
}

void JusPlay(int cs)
{
    string s;
    play_again[cs]=0;
    sleep(3);
    recieve(cs,s);
    int other_player;
    int game=game_no[cs];
    if(games[game].sock_p1==cs)
    {
        other_player=games[game].sock_p2;
    }
    else
    {
        other_player=games[game].sock_p1;
    }
    //cout<<s<<"HH\n";
    if(s=="YES")
    {
        play_again[cs]=1;
    }
    else
    {
        play_again[cs]=-1;
    }
    while(play_again[other_player]==0)
    {

    }
    if(play_again[cs]==1 && play_again[other_player]==1)
    {
        send_msg(cs,"PRO");
        CreateGame(cs);
        StartGame(cs);
        JusPlay(cs);
    }
    else
    {
        send_msg(cs,"DISCONN");
    }
}
void clr_jus(int cs)
{
    string s;
    recieve(cs,s);
    int i,j;
    recv_int(cs,&i);
    recv_int(cs,&j);
    JusPlay(cs);
}
void * handle_one(void * arg)
{
    int client_sock = *(int *)arg;
    /*int con=1;
    char val[1024];
    while(con)
    {
        char to_read='a';
        for(int i=0;i<1024;i++)
        {
            int suc=read(client_sock,&to_read,1);
            if(suc)
            {
                val[i]=to_read;
            }
            else
            {
                con=0;
                val[i]='\0';
                break;
            }
            if(to_read=='\n')
            {
                val[i+1]='\0';
                con=0;
                break;
            }
        }
    }
    //cout<<val<<"\n";
    string name="";
    for(int i=0;i<1024;i++)
    {
        if(val[i]=='\0'){break;}
        name+=val[i];
    }
    sock_name[client_sock]=name;*/
    //cout<<name<<" "<<client_sock<<"\n";
    CreateGame(client_sock);
    StartGame(client_sock);
    int cs=client_sock;
    JusPlay(cs);
    return (void *)(0);
}

int sconnect()
{
    int sockserv=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_port=htons(PORT);
	serv_addr.sin_addr.s_addr=INADDR_ANY;
	serv_addr.sin_family=AF_INET;
    if(sockserv<0)
    {
        perror("Server socket could not be created\n");
        return -1;
    }

    if(bind(sockserv,(struct sockaddr *)(&serv_addr),sizeof(serv_addr))<0)
    {
        perror("Failed to bind\n");
        return -1;
    }

    if(listen(sockserv,27)<0)
    {
        perror("Failed to listen\n");
        return -1;
    }
    return sockserv;
}


int main()
{
    freopen("log.txt","w",stdout);
    int sockserv=sconnect();
    if(sockserv<0)
    {
        exit(0);
    }
    cout<<"Game server started"<<endl;
    int connections=0;
    while(1)
        {
            int cs=accept(sockserv,NULL,NULL);
            if(cs<0)
            {
                cout<<"Failed to accept\n";
                exit(0);
            }
            connections++;
            conn_sock[connections]=cs;
            cout<<"New client connected"<<endl;
            pthread_t new_thread;
            pthread_create(&new_thread, NULL, handle_one, (void *)&cs);
        }
    return 0;    
}