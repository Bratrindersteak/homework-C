//
// gcc db_srv_tcp.c -o db_srv_tcp
// db_srv_tcp
//
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/shm.h>

#define MYPORT  10987
#define DB_SIZE 1024*1024

int main(int argc, char **argv)
{
  // 定义sockfd
  int sock_cli;
  // 定义sockaddr_in
  struct sockaddr_in servaddr;
  char *pBuf;
  //
  int ii;
  int dwValidSize;
  long long dqTotalRecv;
  unsigned char *pT;

  if (argc != 2)
  {
    printf("usage : %s <Srv_IP>\n", argv[0]);
    return 0;
  }

  sock_cli = socket(AF_INET,SOCK_STREAM, 0);

  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(MYPORT);  ///服务器端口
  //servaddr.sin_addr.s_addr = inet_addr(argv[1]);  ///服务器ip
  servaddr.sin_addr.s_addr = inet_addr("192.168.56.1");  ///服务器ip
  pT = (unsigned char *)&servaddr;
  for (ii=0; ii<sizeof(struct sockaddr); ii++)
  {
    printf("%02X ", pT[ii]);
  }
  printf("\n");

  ///连接服务器，成功返回0，错误返回-1
  if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
  {
    perror("connect");
    exit(1);
  }

  pBuf = (char *)malloc(DB_SIZE);
  if (pBuf==NULL) printf("error : no recv buffer !\n");
  dqTotalRecv = 0;
  ii = 0;
  /*
  while (1)
  {
    dwValidSize = recv(sock_cli, pBuf, DB_SIZE, 0); ///接收
    if ((dwValidSize==-1) || (dwValidSize==0))
      break;
    dqTotalRecv = dqTotalRecv + (long long)dwValidSize;
    printf("  %d : %d %lld\n", ii, dwValidSize, dqTotalRecv);
  }
  */
  //dwValidSize = recv(sock_cli, pBuf, DB_SIZE, 0);
  //printf("dwValidSize = %d\n", dwValidSize);
  //if (dwValidSize == -1)
  //  printf("errno = %d\n", errno);
  
  while ((dwValidSize = recv(sock_cli, pBuf, DB_SIZE, 0)) > 0)
  {
    dqTotalRecv = dqTotalRecv + (long long)dwValidSize;
    printf("  %d : %d %lld\n", ii, dwValidSize, dqTotalRecv);
  }
  

  free(pBuf);

  close(sock_cli);
  printf("INFO : %lld bytes recved !\n", dqTotalRecv);

  return 0;
}
