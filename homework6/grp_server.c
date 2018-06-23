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

#define DB_DEBUG 1

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
  struct DATA_PROTO sendInfo;
  struct ifreq buffer;
  unsigned char pLocalMAC[6];
  struct timeval tvNetTimeout={3, 0};
  struct sockaddr_ll devSend;
  char *strInterfaceName = "wlp3s0";
  char contentBuffer[22];

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
  printf("tempSocket: %d\n", sockfd);
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
    if (DB_DEBUG)
    {
      printf("INFO : LocalMAC = %02X:%02X:%02X:%02X:%02X:%02X\n", pLocalMAC[0], pLocalMAC[1], pLocalMAC[2], pLocalMAC[3], pLocalMAC[4], pLocalMAC[5]);
    }
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
    if (DB_DEBUG)
    {
      printf("INFO : IfIndex = %d\n", dwLocalIfIndex);
    }
  }

  memset(&contentBuffer, 0x00, sizeof(contentBuffer));
  contentBuffer[0] = 0xFF;
  contentBuffer[1] = 0xFF;
  contentBuffer[2] = 0xFF;
  contentBuffer[3] = 0xFF;
  contentBuffer[4] = 0xFF;
  contentBuffer[5] = 0xFF;
  contentBuffer[6] = pLocalMAC[0];
  contentBuffer[7] = pLocalMAC[1];
  contentBuffer[8] = pLocalMAC[2];
  contentBuffer[9] = pLocalMAC[3];
  contentBuffer[10] = pLocalMAC[4];
  contentBuffer[11] = pLocalMAC[5];
  contentBuffer[12] = 0x06;
  contentBuffer[13] = 0x06;
  contentBuffer[14] = 0x68;
  contentBuffer[15] = 0x65;
  contentBuffer[16] = 0x6C;
  contentBuffer[17] = 0x6C;
  contentBuffer[18] = 0x6F;
  contentBuffer[19] = 0x4D;
  contentBuffer[20] = 0x41;
  contentBuffer[21] = 0x58;

  bzero(&devSend, sizeof(devSend));
  devSend.sll_family = AF_PACKET;
  memcpy(devSend.sll_addr, pLocalMAC, 6);
  printf("devSend.sll_addr : %02X:%02X:%02X:%02X:%02X:%02X\n", devSend.sll_addr[0], devSend.sll_addr[1], devSend.sll_addr[2], devSend.sll_addr[3], devSend.sll_addr[4], devSend.sll_addr[5]);
  devSend.sll_halen = htons(6);
  devSend.sll_ifindex = dwLocalIfIndex; // This need to be CAUSED !!!
  sendto(sockfd, contentBuffer, sizeof(contentBuffer), 0, (struct sockaddr *)(&devSend), sizeof(devSend));

  close(sockfd);

	return 0;
}
