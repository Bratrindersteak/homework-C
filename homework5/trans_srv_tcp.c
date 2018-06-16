//
// gcc db_cli_tcp.c -o db_cli_tcp
// db_cli_tcp 192.168.56.1
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
#include <sys/shm.h>

#define MYPORT  10987
#define QUEUE   20
//#define DB_SIZE 1024*1024*1024
#define DB_SIZE 1024*1024

int main(int argc, char **argv)
{
  // 定义sockfd
  int server_sockfd;
  int sock_opt;
  // 定义sockaddr_in
  struct sockaddr_in server_sockaddr;
  struct sockaddr_in client_addr;
  // 客户端套接字
  socklen_t length = sizeof(client_addr);
  int conn;
  // vars for the source data block
  int ii;
  FILE *fpSrc;
  long long dqFileSize;
  long long dqTotalSend;
  int dwTimes;
  int dwValidSize;
  char *pBuf;
  unsigned char *pT;

  if (argc == 2)
  {
    fpSrc = fopen(argv[1], "rb");
    if (fpSrc != NULL)
    {
      fseek(fpSrc, 0, SEEK_END);
      dqFileSize = ftell(fpSrc);
      fseek(fpSrc, 0, SEEK_SET);
      pBuf = (char *)malloc(DB_SIZE);
      dwTimes = (int)(dqFileSize / (DB_SIZE));
    }
    else
    {
      printf("ERROR : can not find the source data file !\n");
      return -1;
    }
  }
  else
  {
    printf("usage : %s <sourceFileName>\n", argv[0]);
    return 0;
  }

  server_sockaddr.sin_family = AF_INET;
  server_sockaddr.sin_port = htons(MYPORT);
  server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  pT = (unsigned char *)&server_sockaddr;
  for (ii=0; ii<sizeof(struct sockaddr); ii++)
  {
    printf("%02X ", pT[ii]);
  }
  printf("\n");

  server_sockfd = socket(AF_INET,SOCK_STREAM, 0);

  sock_opt = 1;
  setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(sock_opt));

  // bind，成功返回0，出错返回-1
  if (bind(server_sockfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr))==-1)
  {
    perror("bind");
    exit(1);
  }

  // listen，成功返回0，出错返回-1
  printf("INFO : waiting for the client connection ...\n");
  if (listen(server_sockfd,QUEUE) == -1)
  {
    perror("listen");
    exit(1);
  }

  // 成功返回非负描述字，出错返回-1
  conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
  if(conn<0)
  {
    perror("connect");
    exit(1);
  }
  pT = (unsigned char *)&client_addr;
  for (ii=0; ii<sizeof(struct sockaddr); ii++)
  {
    printf("%02X ", pT[ii]);
  }
  printf("\n");
  printf("INFO : found the connection from the client\n");

  printf("INFO : it's need %d times to send the data to the client ...\n", dwTimes);
  dqTotalSend = 0;
  /*
  for (ii=0; ii<dwTimes; ii++)
  {
    memset(pBuf, 0, DB_SIZE);
    dwValidSize = fread(pBuf, 1, DB_SIZE, fpSrc);
    send(conn, pBuf, dwValidSize, 0);
    dqTotalSend = dqTotalSend + (long long)dwValidSize;
  }
  if (!feof(fpSrc))
  {
    // do the last time
    memset(pBuf, 0, DB_SIZE);
    dwValidSize = fread(pBuf, 1, DB_SIZE, fpSrc);
    send(conn, pBuf, dwValidSize, 0);
    dqTotalSend = dqTotalSend + (long long)dwValidSize;
  }
  */
  memset(pBuf, 0, DB_SIZE);
  while ((dwValidSize = fread(pBuf, 1, DB_SIZE, fpSrc)) > 0)
  {
    printf("  send %d bytes to client ...\n", dwValidSize);
    send(conn, pBuf, dwValidSize, 0);
    dqTotalSend = dqTotalSend + (long long)dwValidSize;
  }

  //shutdown(conn, SHUT_WR);
  //recv(conn, pBuf, DB_SIZE, 0);

  free(pBuf);

  close(conn);
  close(server_sockfd);

  fclose(fpSrc);

  printf("INFO : %lld bytes were sent to the client !\n", dqTotalSend);

  return 0;
}
