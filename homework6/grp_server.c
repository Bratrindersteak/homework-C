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

  printf("IPPROTO_ICMP: %d\n", IPPROTO_ICMP);
  sockfd = socket(AF_LOCAL, SOCK_RAW, IPPROTO_ICMP);
  // sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  // sockfd = socket(AF_INET, SOCK_STREAM, 0);
  printf("sockfd: %d\n", sockfd);
  if (sockfd == -1)
  {
    printf("sockfd error\n");
    return -1;
  }

  printf("so...,the next~\n");

	return 0;
}