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
  unsigned int dwGroupID;
  struct LLC_PROTO *llcInfo;
  struct DATA_PROTO *contentInfo;
  struct ifreq buffer;
  unsigned char broadcastAddr[6];
  unsigned char pLocalMAC[6];
  struct timeval tvNetTimeout={3, 0};
  struct sockaddr_ll devSend;
  char *strInterfaceName = "wlp3s0";
  char sendBuf[128];
  
  bzero(&sendBuf, sizeof(sendBuf));
  sscanf(argv[1], "%x", &dwGroupID);
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
  strcpy(buffer.ifr_name, strInterfaceName);
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
  strcpy(buffer.ifr_name, strInterfaceName);
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

  llcInfo = (struct LLC_PROTO *)(&sendBuf[0]);
  broadcastAddr[0] = 0xff;
  broadcastAddr[1] = 0xff;
  broadcastAddr[2] = 0xff;
  broadcastAddr[3] = 0xff;
  broadcastAddr[4] = 0xff;
  broadcastAddr[5] = 0xff;
  memcpy((void *)llcInfo->destMacAddr, (void *)broadcastAddr, 6);
  memcpy((void *)llcInfo->srcMacAddr, (void *)pLocalMAC, 6);
  llcInfo->protocolNo = 0x9876;
  // printf("llcInfo->destMacAddr: %012x\n", (int *)(&llcInfo->destMacAddr));
  // printf("llcInfo->srcMacAddr: %012x\n", (int *)(&llcInfo->srcMacAddr));
  printf("llcInfo->protocolNo: %04x\n", llcInfo->protocolNo);

  contentInfo = (struct DATA_PROTO *)(&sendBuf[14]);
  contentInfo->dwGroupID = dwGroupID;
  // contentInfo->dwRequestTimes = 123456789;
  contentInfo->dwRequestTimes = time(&t);
  contentInfo->wGroupCmd = 0x0FF0;
  printf("contentInfo->dwGroupID: %04x\n", contentInfo->dwGroupID);
  printf("contentInfo->dwRequestTimes: %08x\n", contentInfo->dwRequestTimes);
  printf("contentInfo->wGroupCmd: %04x\n", contentInfo->wGroupCmd);

  bzero(&devSend, sizeof(devSend));
  devSend.sll_family = AF_PACKET;
  memcpy(devSend.sll_addr, pLocalMAC, 6);
  devSend.sll_halen = htons(6);
  devSend.sll_ifindex = dwLocalIfIndex; // This need to be CAUSED !!!

  int sendMsg = sendto(sockfd, (char *)&sendBuf, sizeof(struct LLC_PROTO) + sizeof(struct DATA_PROTO), 0, (struct sockaddr *)(&devSend), sizeof(devSend));
  printf("sendMsg: %d\n", sendMsg);

  close(sockfd);

	return 0;
}
