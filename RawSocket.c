#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/if.h>
#include <linux/in.h>
#include <linux/sockios.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> 
#include <pthread.h>

//unsigned char mac[6]="\x08\x00\x27\x03\x75\x3E";
unsigned char mac[6]="\x08\x00\x27\x53\x10\xFA";
unsigned char src_mac[6]="\x74\xe6\xe2\x00\x39\x0c";
int sock_r;

void *runner(void *vargp)//function for sending packet to destination mac
{
  int sock_raw=socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
   if(sock_raw == -1) 
     printf("error in socket");
   
  struct ifreq ifreq_i;
  memset(&ifreq_i,0,sizeof(ifreq_i));
//strncpy(ifreq_i.ifr_name,"wlp6s0",IFNAMSIZ-1); //giving name of Interface
  strncpy(ifreq_i.ifr_name,"enp7s0",IFNAMSIZ-1); //giving name of Interface

  if((ioctl(sock_raw,SIOCGIFINDEX,&ifreq_i))<0)
    printf("error in index ioctl reading");//getting Index Name

  struct ifreq ifreq_c;
  memset(&ifreq_c,0,sizeof(ifreq_c));
  strncpy(ifreq_c.ifr_name,"enp7s0",IFNAMSIZ-1);//giving name of Interface
 
  if((ioctl(sock_raw,SIOCGIFHWADDR,&ifreq_c))<0) //getting MAC Address
    printf("error in SIOCGIFHWADDR ioctl reading");

  struct ifreq ifreq_ip;
  memset(&ifreq_ip,0,sizeof(ifreq_ip));
  strncpy(ifreq_ip.ifr_name,"enp7s0",IFNAMSIZ-1);//giving name of Interface
  if(ioctl(sock_raw,SIOCGIFADDR,&ifreq_ip)<0) //getting IP Address
    printf("error in SIOCGIFADDR \n");

  while(1)
  {
  
  unsigned char* sendbuff=malloc(64*sizeof(unsigned char));
  memset(sendbuff,'\0',64);

  struct ethhdr *eth1 = (struct ethhdr *)(sendbuff);
  
  for(int i=0;i<6;i++)
  {
   eth1->h_source[i]=src_mac[i];
   eth1->h_dest[i]=mac[i];
  }

  eth1->h_proto = htons(ETH_P_IP);
  char ch[46];
 
  scanf(" %s",&ch);
  if(!strcmp(ch,"exit"))
  {
      close(sock_raw);
      close(sock_r);
      exit(0);
  }  

  strcpy(sendbuff+14,ch);
  
  for(int i=0;i<=60;i++)
   fprintf(stderr,"%.02x|",sendbuff[i]);
  printf("\n");
  struct sockaddr_ll sadr_ll;
  sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex; // index of interface
  sadr_ll.sll_halen = ETH_ALEN; // length of destination mac address

  for(int i=0;i<6;i++)
    sadr_ll.sll_addr[i] = mac[i];

  int send_len = sendto(sock_raw,sendbuff,64,0,(const struct sockaddr*)&sadr_ll,sizeof(struct sockaddr_ll));

  if(send_len<0)
  {
   perror("sendto");
   return NULL;
  }

  memset(sendbuff,'\0',64); 
 }

 close(sock_raw);

 pthread_exit(0);

}


int main()
{
  pthread_t t1;
  int x=0;
  sock_r=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));//socket for receiving all ethernet packets
  if(sock_r<0)
  {
   perror("error in socket\n");
   return -1;
  }

  unsigned char *buffer = (unsigned char *) malloc(65536); //to receive data
  memset(buffer,0,65536);
  struct sockaddr_in addr,source,dest;
  int saddr_len = sizeof (addr);
  pthread_create(&t1,NULL,runner,NULL);//calling the thread function
 
 //Receive a network packet and copy in to buffer
 while(1)   
 {
 	 int buflen=recvfrom(sock_r,buffer,65536,0,(struct sockaddr *)&addr,(socklen_t *)&saddr_len);
   if(buflen<0)
   {
      perror("error in reading recvfrom function\n");
      return -1;
   }
   struct ethhdr *eth = (struct ethhdr *)(buffer);
   int flag=1;
   for(int i=0;i<6;i++)
   {
	  if(eth->h_source[i]!=mac[i])
	  {
		 flag=0;
		 break;
	  }

   }

   if(flag)
   {
     printf("|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
     buffer=buffer+14;
     printf("DATA=%s\n",buffer );
     printf("--------------------------------------------------------\n");
   }
 }

}  
