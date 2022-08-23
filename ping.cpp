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
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <chrono>
#define ss 51200
#define PORT 9007
using namespace std;
using namespace std::chrono;
bool timeout(int sock)
{
    struct pollfd x;
    x.fd=sock;
    x.events=POLLIN;
    int ret=poll(&x,1,10000);
    if(ret==-1)
    {
        return true;
    }
    if(ret==0)
    {
        return true;
    }
    return false;
}
unsigned short cal_chksum(unsigned short *addr, int len)
{
    unsigned short *w=addr;
    unsigned short answer=0;
    int nleft=len;
    int sum=0;
    while (nleft>1)
    {
        nleft=nleft-2;
        sum+=*w++;
    }
    if (nleft==1)
    {
        *(unsigned char*)(&answer)=*(unsigned char*)w;
        sum+=answer;
    }

    sum=(sum >> 16)+(sum &0xffff);
    sum=sum+(sum >> 16);
    return ~sum;
}

int main(int argc,char* argv[])
{
    if(argc>2 || argc<2)
    {
        cout<<"Input improperly formatted.Make sure that you are entering IP address also\n";
        exit(0);
    }
    int sockfd=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
    if(sockfd<0)
    {
        cout<<"Socket could not be created\n";
        exit(0);
    }
    int siz=ss;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&siz,sizeof(siz));
    struct sockaddr_in serv_addr;
    struct hostent *hoste;
    hoste=gethostbyname(argv[1]);
    if(hoste==NULL)
    {
        cout<<"Bad hostname\n";
        exit(0);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr=*((struct in_addr *)(hoste)->h_addr); 
	serv_addr.sin_family=AF_INET;
    char pck[4096];
    struct icmp* icm=(struct icmp*)(pck);
    int psize=64;
    icm->icmp_id=getpid();
    icm->icmp_code=0;
    icm->icmp_seq=0; 
    icm->icmp_type=ICMP_ECHO;                        
    icm->icmp_cksum=cal_chksum((unsigned short*)icm,psize);
    auto stime=high_resolution_clock::now();
    int a=sendto(sockfd,pck,psize,0,(struct sockaddr*)&serv_addr,sizeof(serv_addr));
    if(!(a>=0))
    {
        cout<<"Request timed out or host unreacheable\n";
    }
    bool d=timeout(sockfd);
    if(d)
    {
        cout<<"Request timed out or host unreacheable\n";
        exit(0);
    }
    char to_fill[4096];
    struct sockaddr_in f;
    unsigned int leng=sizeof(f);
    int b=recvfrom(sockfd,to_fill,sizeof(to_fill),0,(struct sockaddr*) &f,&leng);
    auto etime=high_resolution_clock::now();
    if(!(b>=0))
    {
        cout<<"Request timed out or host unreacheable\n";
    }
    auto duration=duration_cast<milliseconds>(etime-stime);
    cout<<"RTT is: "<<duration.count()<<"ms\n";
}