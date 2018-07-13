//
// gcc grp_pthread.c -lpthread -o grp_pthread
// ./grp_pthread <grp_ID> <status>
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

unsigned char masterStatus[2];
unsigned char workerStatus[2];
unsigned int myGroupID;
unsigned int status;
int isServerPackage (int protocolNo, int dwGroupID, int wGroupCmd);
int isClientPackage (int protocolNo, int dwGroupID, int wGroupCmd);
int dwExit = 1;
int node_ID = 1;

void *send_func(&sendLLC, &sendContent)
{
  while (dwExit != 0)
  {
    sem_wait(&sem_send);
    sendContent->dwRequestTimes = time(&t);
    sendto(sockfd, (char *)&pSendBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0, (struct sockaddr *)(&devSend), sizeof(devSend));
  }
}

void *recv_func(sockfd, &recvLLC, &recvContent)
{
  while (dwExit != 0)
  {
    recv(sockfd, pRecvBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0);

    if (recvLLC->protocolNo == PRIVATE_PROTOCOL && recvContent->dwGroupID = myGroupID)
    {
      sem_post(&sem_recv);
    }
  }
}

int main (int argc, char **argv)
{
  int sockfd;
  int dwLocalIfIndex;
  int recvPackage;
  int isMyPackage = 0;
  int wNodeID = 1;
  struct ifreq buffer;
  unsigned char broadcastAddr[6];
  unsigned char pLocalMAC[6];
  struct timeval tvNetTimeout={3, 0};
  struct sockaddr_ll devSend;
  // char *interfaceName = "wlp112s0";
  char *interfaceName = "wlp3s0";
  struct LLC_PROTO *sendLLC;
  struct DATA_PROTO *sendContent;
  struct LLC_PROTO *recvLLC;
  struct DATA_PROTO *recvContent;
  char pSendBuf[MAX_SIZE];
  char pRecvBuf[MAX_SIZE];

  sendLLC = (struct LLC_PROTO *)(&pSendBuf[0]);
  sendContent = (struct DATA_PROTO *)(&pSendBuf[14]);
  recvLLC = (struct LLC_PROTO *)(&pRecvBuf[0]);
  recvContent = (struct DATA_PROTO *)(&pRecvBuf[14]);
  sendContent->wNodeID = -1;
  sscanf(argv[1], "%x", &myGroupID);
  sscanf(argv[2], "%d", &status);
  time_t t;

  sem_init(&sem_send, 0, 0);
  sem_init(&sem_recv, 0, 0);

  pthread_t send_pthread;
  pthread_t recv_pthread;

  pthread_create(&send_pthread, NULL, send_func, &sendLLC, &sendContent);
  pthread_create(&recv_pthread, NULL, recv_func, sockfd, &recvLLC, &recvContent);

  if (argv[1] == NULL)
  {
    printf("grp_ID is %s\n", argv[1]);
    return -1;
  }

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

  broadcastAddr[0] = 0xff;
  broadcastAddr[1] = 0xff;
  broadcastAddr[2] = 0xff;
  broadcastAddr[3] = 0xff;
  broadcastAddr[4] = 0xff;
  broadcastAddr[5] = 0xff;
  memcpy((void *)sendLLC->destMacAddr, (void *)broadcastAddr, 6);
  memcpy((void *)sendLLC->srcMacAddr, (void *)pLocalMAC, 6);
  sendLLC->protocolNo = PRIVATE_PROTOCOL;
  sendContent->dwGroupID = myGroupID;
  sendContent->dwRequestTimes = time(&t);
  sendContent->wGroupCmd = 0x0FF0;
  printf("sendContent->dwGroupID: %04x\n", sendContent->dwGroupID);
  printf("sendContent->dwRequestTimes: %08x\n", sendContent->dwRequestTimes);
  printf("sendContent->wGroupCmd: %04x\n", sendContent->wGroupCmd);

  bzero(&devSend, sizeof(devSend));
  devSend.sll_family = AF_PACKET;
  memcpy(devSend.sll_addr, pLocalMAC, 6);
  devSend.sll_halen = htons(6);
  devSend.sll_ifindex = dwLocalIfIndex; // This need to be CAUSED !!!

  sem_post(&sem_send);
  while (dwExit != 0)
  {
    sem_wait(&sem_recv);

    switch (recvContent->wGroupCmd)
    {
      case 0x0FF0:
        if (status)
        {
          sendLLC->destMacAddr = recvLLC->srcMacAddr;
          sendContent->wGroupCmd = 0x0F01;
          sem_post(&&sem_send);
        }
        break;
      case 0x0F01:
        if (status)
        {
          dwExit = 0;
        }
        else
        {
          sendLLC->destMacAddr = recvLLC->srcMacAddr;
          sendContent->wGroupCmd = 0x00F0;
          sem_post(&&sem_send);
        }
        break;
      case 0x00F0:
        if (status)
        {
          sendLLC->destMacAddr = recvLLC->srcMacAddr;
          sendContent->wGroupCmd = 0x0001;
          sendContent->wNodeID = node_ID;
          sem_post(&sem_send);
          node_ID++;
        }
        break;
      case 0x0001:
        if (!status)
        {
          dwExit = 0;
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

int isServerPackage (int protocolNo, int dwGroupID, int wGroupCmd)
{
  if (protocolNo != PRIVATE_PROTOCOL)
  {
    return 0;
  }

  if (dwGroupID != myGroupID)
  {
    return 0;
  }

  if (wGroupCmd != 0x0FF0 && wGroupCmd != 0x0F01)
  {
    return 0;
  }

  return 1;
}

int isClientPackage (int protocolNo, int dwGroupID, int wGroupCmd)
{
  if (protocolNo != PRIVATE_PROTOCOL)
  {
    return 0;
  }

  if (dwGroupID != myGroupID)
  {
    return 0;
  }

  if (wGroupCmd != 0x00F0 && wGroupCmd != 0x0001)
  {
    return 0;
  }

  return 1;
}
