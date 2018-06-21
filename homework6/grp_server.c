//
// gcc grp_server.c -o grp_server
// ./grp_server <grp_ID>
//
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
  struct DATA_PROTO sendInfo;

  // sendInfo.dwGroupID = (int *)&argv[1];
  sendInfo.dwGroupID = 0x1234;
  // strcpy(sendInfo.dwGroupID, argv[1]);
  printf("sendInfo.dwGroupID: %d\n", sendInfo.dwGroupID);

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
  // sockfd = socket(PF_PACKET, SOCK_RAW, 0);
  printf("sockfd: %d\n", sockfd);
  if (sockfd == -1)
  {
    printf("create socket error\n");
    return -1;
  }
  
  printf("so...,the next~\n");

	return 0;
}