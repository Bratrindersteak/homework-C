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

int main (int argc, char **argv)
{
	int sockfd;
  int dwLocalIfIndex;
  int recvInfo;
  unsigned int dwGroupID;
  struct DATA_PROTO sendInfo;
  struct ifreq buffer;
  unsigned char broadcastAddr[6];
  unsigned char pLocalMAC[6];
  struct timeval tvNetTimeout={3, 0};
  struct sockaddr_ll devSend;
  char *interfaceName = "wlp112s0";
  // char *interfaceName = "wlp3s0";

  struct LLC_PROTO *sendLLC;
  struct DATA_PROTO *sendContent;
  struct LLC_PROTO *recvLLC;
  struct DATA_PROTO *recvContent;

  char pBuf[sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO)];
  // char *pBuf;
  // pBuf = (char *)malloc(sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO));
  printf("struct LLC_PROTO: %ld\n", sizeof(struct LLC_PROTO));
  printf("struct DATA_PROTO: %ld\n", sizeof(struct DATA_PROTO));

  recvLLC = (struct LLC_PROTO *)(&pBuf[0]);
  recvContent = (struct DATA_PROTO *)(&pBuf[14]);

  printf("pBuf: %s\n", pBuf);

  sscanf(argv[1], "%x", &dwGroupID);
  sendInfo.dwGroupID = dwGroupID;
  printf("sendInfo.dwGroupID: %x\n", sendInfo.dwGroupID);

  time_t t;
  sendInfo.dwRequestTimes = time(&t);
  printf("sendInfo.dwRequestTimes: %d\n", sendInfo.dwRequestTimes);


	printf("grp_ID: %s\n", argv[1]);
	if (argv[1] == NULL)
	{
    printf("grp_ID is %s\n", argv[1]);
    return -1;
	}
	printf("(int *s)argv[1]: %ls\n", (int *)argv[1]);

  sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  printf("sockfd: %d\n", sockfd);
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

  memset(&buffer, 0x00, sizeof(buffer));
  strcpy(buffer.ifr_name, interfaceName);
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

  // send broadcast package.
  sendLLC = (struct LLC_PROTO *)(&pBuf[0]);
  broadcastAddr[0] = 0xff;
  broadcastAddr[1] = 0xff;
  broadcastAddr[2] = 0xff;
  broadcastAddr[3] = 0xff;
  broadcastAddr[4] = 0xff;
  broadcastAddr[5] = 0xff;
  memcpy((void *)sendLLC->destMacAddr, (void *)broadcastAddr, 6);
  memcpy((void *)sendLLC->srcMacAddr, (void *)pLocalMAC, 6);
  sendLLC->protocolNo = 0x9876;
  printf("sendLLC->destMacAddr: %012x\n", (int *)(sendLLC->destMacAddr));
  printf("sendLLC->srcMacAddr: %012x\n", (int *)(sendLLC->srcMacAddr));
  printf("sendLLC->protocolNo: %04x\n", sendLLC->protocolNo);

  sendContent = (struct DATA_PROTO *)(&pBuf[14]);
  sendContent->dwGroupID = dwGroupID;
  // sendContent->dwRequestTimes = 123456789;
  sendContent->dwRequestTimes = time(&t);
  sendContent->wGroupCmd = 0x0FF0;
  sendContent->wNodeID = -1;
  printf("sendContent->dwGroupID: %04x\n", sendContent->dwGroupID);
  printf("sendContent->dwRequestTimes: %08x\n", sendContent->dwRequestTimes);
  printf("sendContent->wGroupCmd: %04x\n", sendContent->wGroupCmd);
  printf("sendContent->wNodeID: %04x\n", sendContent->wNodeID);

  bzero(&devSend, sizeof(devSend));
  devSend.sll_family = AF_PACKET;
  memcpy(devSend.sll_addr, pLocalMAC, 6);
  devSend.sll_halen = htons(6);
  devSend.sll_ifindex = dwLocalIfIndex; // This need to be CAUSED !!!

  int sendMsg = sendto(sockfd, (char *)&pBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0, (struct sockaddr *)(&devSend), sizeof(devSend));
  printf("sendMsg: %d\n", sendMsg);

  // recv package.
  while (recvLLC->protocolNo != 0x9876) {
    recvInfo = recv(sockfd, pBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0);
    if (recvInfo == -1)
    {
      printf("ERROR : Can NOT receive package !!!\n");
      return -1;
    }
    printf("------\n");
    printf("recvInfo: %02X\n", recvLLC->destMacAddr);
    printf("recvInfo: %02X\n", recvLLC->srcMacAddr);
    printf("recvInfo: %02X\n", recvLLC->protocolNo);
    printf("recvInfo: %02X\n", recvContent->dwGroupID);
    printf("recvInfo: %02X\n", recvContent->dwRequestTimes);
    printf("recvInfo: %02X\n", recvContent->wGroupCmd);
  }

  switch(recvContent->wGroupCmd)
  {
    case 0x00F0:
      printf("This cmd is 0x00F0(服务器响应成员的数据包，告知自己为group中的服务器\n");
      break;
    case 0x0001:
      printf("This cmd is 0x0001(服务器发放给成员的NodeID响应包)\n");
      break;
    default:
      printf("ERROR: no such recv cmd !!!\n");
  }

  close(sockfd);

	return 0;
}
