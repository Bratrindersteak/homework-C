//
// gcc grp_server.c -o grp_server
// ./grp_server <grp_ID>
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

#define MAX_SIZE 1514
#define PRIVATE_PROTOCOL 0x7698

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
int isServerPackage (int protocolNo, int dwGroupID, int wGroupCmd);

int main (int argc, char **argv)
{
  int ii;
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

  strcpy(masterStatus, "1");
  strcpy(workerStatus, "0");

  if (strcmp(argv[2], masterStatus) == 0)
  {
    printf("INFO: This is Master !!!\n");
  }

  if (strcmp(argv[2], workerStatus) == 0)
  {
    printf("INFO: This is Worker !!!\n");
  }

  sendLLC = (struct LLC_PROTO *)(&pSendBuf[0]);
  sendContent = (struct DATA_PROTO *)(&pSendBuf[14]);
  recvLLC = (struct LLC_PROTO *)(&pRecvBuf[0]);
  recvContent = (struct DATA_PROTO *)(&pRecvBuf[14]);
  sendContent->wNodeID = -1;
  sscanf(argv[1], "%x", &myGroupID);
  time_t t;

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
  // sendContent->dwRequestTimes = 123456789;
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

  int sendMsg = sendto(sockfd, (char *)&pSendBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0, (struct sockaddr *)(&devSend), sizeof(devSend));
  printf("sendMsg: %d\n", sendMsg);

  while (1)
  {
    // recv package.
    while (!isMyPackage) {
      recvPackage = recv(sockfd, pRecvBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0);
      if (recvPackage == -1)
      {
        printf("ERROR : Can NOT receive package !!!\n");
        return -1;
      }
      isMyPackage = isServerPackage(recvLLC->protocolNo, recvContent->dwGroupID, recvContent->wGroupCmd);
    }

    if (strcmp(argv[2], masterStatus) == 0)
    {
      // Master is already exist.
      if (recvContent->wGroupCmd == 0x00F0)
      {
        printf("ERROR : Unfortunately, the Master is already exist, you are finished here !!!\n");
      }

      // Broadcast looking for Master.
      if (recvContent->wGroupCmd == 0x0FF0)
      {
        printf("INFO : This is a request looking for Master !!!\n");
        memcpy((void *)sendLLC->destMacAddr, (void *)recvLLC->srcMacAddr, 6);
        time_t t;
        sendContent->dwRequestTimes = time(&t);
        sendContent->wGroupCmd = 0x00F0;

        if (sendto(sockfd, (char *)&pSendBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0, (struct sockaddr *)(&devSend), sizeof(devSend)) == -1)
        {
          printf("ERROR : Can NOT send Master reply package !!!\n");
        }
      }

      // wNodeID request from client.
      if (recvContent->wGroupCmd == 0x0F01)
      {
        printf("INFO : This is a request for wNodeID !!!\n");
        memcpy((void *)sendLLC->destMacAddr, (void *)recvLLC->srcMacAddr, 6);
        time_t t;
        sendContent->dwRequestTimes = time(&t);
        sendContent->wGroupCmd = 0x0001;
        sendContent->wNodeID = wNodeID;

        if (sendto(sockfd, (char *)&pSendBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0, (struct sockaddr *)(&devSend), sizeof(devSend)) == -1)
        {
          printf("ERROR : Can NOT send Master reply package !!!\n");
        }
        wNodeID++;
      }
    }

    if (strcmp(argv[2], workerStatus) == 0)
    {
      // Found the Master.
      if (recvContent->wGroupCmd == 0x00F0)
      {
        printf("INFO : Response from The Master !!!\n");
        memcpy((void *)sendLLC->destMacAddr, (void *)recvLLC->srcMacAddr, 6);
        time_t t;
        sendContent->dwRequestTimes = time(&t);
        sendContent->wGroupCmd = 0x0F01;
        if (sendto(sockfd, (char *)&pSendBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0, (struct sockaddr *)(&devSend), sizeof(devSend)) == -1)
        {
          printf("ERROR : Can NOT send wNodeID apply package !!!\n");
        }
      }

      // Recv wNodeID.
      if (recvContent->wGroupCmd == 0x0001)
      {
        printf("INFO : Recv the wNodeID from Master !!!\n");
        printf("recvContent->wNodeID: %d\n", recvContent->wNodeID);
        break;
      }
    }
    isMyPackage = 0;
  }

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
