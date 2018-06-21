//
// gcc grp_server.c -o grp_server
// ./grp_server <grp_ID>
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <netinet/if_ether.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

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
	int tempSocket;
  struct DATA_PROTO sendInfo;
  struct ifreq buffer;
  unsigned char pLocalMAC[6];
  char *strInterfaceName = "wlp3s0";

  // sendInfo.dwGroupID = (int *)&argv[1];
  // strcpy(sendInfo.dwGroupID, argv[1]);

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

  // tempSocket = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  tempSocket = socket(PF_PACKET, SOCK_RAW, 0);
  printf("tempSocket: %d\n", tempSocket);
  if (tempSocket == -1)
  {
    printf("create socket error\n");
    return -1;
  }

  memset(&buffer, 0x00, sizeof(buffer));
  strcpy(buffer.ifr_name, strInterfaceName);
  if (ioctl(tempSocket, SIOCGIFHWADDR, &buffer) == -1)
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

  printf("so...,the next~\n");

	return 0;
}