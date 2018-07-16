//
// gcc grp.c -lpthread -o grp
// ./grp <grp_ID> <status>
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/shm.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>
#include <linux/sockios.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_SIZE 1514
#define PRIVATE_PROTOCOL 0x7698

sem_t sem_send;
sem_t sem_recv;

int dwExit = 1;
int sockfd;
int dwLocalIfIndex;
int recvPackage;
int isMyPackage = 0;
int wNodeID = 1;
unsigned int myGroupID;
unsigned int status;
struct ifreq buffer;
struct timeval tvNetTimeout={3, 0};
struct sockaddr_ll devSend;
// char *interfaceName = "wlp112s0";
// char *interfaceName = "wlp3s0";
char *interfaceName = "enp0s3";
char pSendBuf[MAX_SIZE];
char pRecvBuf[MAX_SIZE];
time_t t;

struct LLC_PROTO
{
  unsigned char destMacAddr[6];
  unsigned char srcMacAddr[6];
  unsigned short protocolNo;
};

struct DATA_PROTO
{
	unsigned int dwGroupID;
	unsigned int dwRequestTimes;
	unsigned short wGroupCmd;
	unsigned short wNodeID;
};

struct LLC_PROTO *sendLLC;
struct DATA_PROTO *sendContent;
struct LLC_PROTO *recvLLC;
struct DATA_PROTO *recvContent;

void *send_func(void *arg)
{
  while (dwExit != 0)
  {
    sem_wait(&sem_send);

    if (dwExit == 0) {
    	break;
    }

    sendContent->dwRequestTimes = time(&t);
    sendto(sockfd, (char *)&pSendBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0, (struct sockaddr *)(&devSend), sizeof(devSend));
    printf("sendContent->wGroupCmd: %04x\nsendContent->wNodeID: %d\n", sendContent->wGroupCmd, sendContent->wNodeID);
  }
}

void *recv_func(void *arg)
{
  while (dwExit != 0)
  {
    recv(sockfd, pRecvBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0);
    if (recvLLC->protocolNo == PRIVATE_PROTOCOL && recvContent->dwGroupID == myGroupID)
    {
      sem_post(&sem_recv);
      printf("recvContent->wGroupCmd: %04x\nrecvContent->wNodeID: %d\n", recvContent->wGroupCmd, recvContent->wNodeID);
    }
  }
}

int main (int argc, char **argv)
{
	if (argv[1] == NULL)
	{
	  printf("ERROR: grp_ID is needed !!!\n");
	  return -1;
	}

	if (argv[2] == NULL)
	{
	  printf("ERROR: status is needed !!!\n");
	  return -1;
	}
	sscanf(argv[1], "%x", &myGroupID);
  sscanf(argv[2], "%d", &status);

  sendLLC = (struct LLC_PROTO *)(&pSendBuf[0]);
  sendContent = (struct DATA_PROTO *)(&pSendBuf[14]);
  recvLLC = (struct LLC_PROTO *)(&pRecvBuf[0]);
  recvContent = (struct DATA_PROTO *)(&pRecvBuf[14]);
  unsigned char broadcastAddr[6];
  unsigned char pLocalMAC[6];

  sem_init(&sem_send, 0, 0);
  sem_init(&sem_recv, 0, 0);

  pthread_t send_pthread;
  pthread_t recv_pthread;

  pthread_create(&send_pthread, NULL, send_func, NULL);
  pthread_create(&recv_pthread, NULL, recv_func, NULL);

  sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sockfd == -1)
  {
    printf("create socket error\n");
    return -1;
  }

  memset(&buffer, 0x00, sizeof(buffer));
  strcpy(buffer.ifr_name, interfaceName);
  if (ioctl(sockfd, SIOCGIFHWADDR, &buffer) == -1)
  {
    printf("ERROR : Can NOT get the local mac address !!!\n");
    return -2;
  }
  else
  {
    memcpy((void *)pLocalMAC, (void *)buffer.ifr_hwaddr.sa_data, 6);
  }

  if (ioctl(sockfd, SIOCGIFINDEX, &buffer) == -1)
  {
    printf("ERROR : Can NOT get the local interface index !!!\n");
    return -2;
  }
  else
  {
    dwLocalIfIndex = buffer.ifr_ifindex;
    printf("INFO : IfIndex = %d\n", dwLocalIfIndex);
  }

  broadcastAddr[0] = 0xFF;
  broadcastAddr[1] = 0xFF;
  broadcastAddr[2] = 0xFF;
  broadcastAddr[3] = 0xFF;
  broadcastAddr[4] = 0xFF;
  broadcastAddr[5] = 0xFF;
  memcpy((void *)sendLLC->destMacAddr, (void *)broadcastAddr, 6);
  memcpy((void *)sendLLC->srcMacAddr, (void *)pLocalMAC, 6);
  sendLLC->protocolNo = PRIVATE_PROTOCOL;
  sendContent->dwGroupID = myGroupID;
  sendContent->dwRequestTimes = time(&t);
  sendContent->wGroupCmd = 0x0FF0;
  sendContent->wNodeID = -1;

  bzero(&devSend, sizeof(devSend));
  devSend.sll_family = AF_PACKET;
  memcpy(devSend.sll_addr, pLocalMAC, 6);
  devSend.sll_halen = htons(6);
  devSend.sll_ifindex = dwLocalIfIndex;

  sem_post(&sem_send);
  while (dwExit != 0)
  {
    sem_wait(&sem_recv);
    switch (recvContent->wGroupCmd)
    {
      case 0x0FF0:
        if (status)
        {
        	memcpy((void *)sendLLC->destMacAddr, (void *)recvLLC->srcMacAddr, 6);
          sendContent->wGroupCmd = 0x0F01;
          sendContent->wNodeID = -1;
          sem_post(&sem_send);
        }
        break;
      case 0x0F01:
        if (status == 1)
        {
          dwExit = 0;
          sem_post(&sem_send);
        }
        else
        {
          memcpy((void *)sendLLC->destMacAddr, (void *)recvLLC->srcMacAddr, 6);
          sendContent->wGroupCmd = 0x00F0;
          sendContent->wNodeID = -1;
          sem_post(&sem_send);
        }
        break;
      case 0x00F0:
        if (status)
        {
          memcpy((void *)sendLLC->destMacAddr, (void *)recvLLC->srcMacAddr, 6);
          sendContent->wGroupCmd = 0x0001;
          sendContent->wNodeID = wNodeID;
          sem_post(&sem_send);
          wNodeID++;
        }
        break;
      case 0x0001:
        if (status == 0)
        {
          dwExit = 0;
          sem_post(&sem_send);
        }
        break;
    }
  }

  pthread_join(send_pthread, NULL);
  pthread_join(recv_pthread, NULL);

  sem_destroy(&sem_send);
  sem_destroy(&sem_recv);

  close(sockfd);
  
  return 0;
}
