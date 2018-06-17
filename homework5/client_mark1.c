//
// gcc client_mark1.c -o client_mark1
// ./client_mark1
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

#define DEST_PORT 1500 //目标地址端口号
#define DEST_IP "127.0.0.1" //目标地址IP，这里设为本机
#define MAX_DATA 4096 //接收到的数据最大程度

struct FILE_DESC
{
  unsigned long long fileSize;
  char fileName[64];
};

int main()
{
  FILE *fp;
  unsigned long long fileSize;
  char fileName[32];
  char recv_fileName[32];
  int sockfd,new_fd; //cocket句柄和接受到连接后的句柄
  int recv_info, recv_file;
  struct sockaddr_in dest_addr; //目标地址信息
  char *pBuf; //储存接收数据
  struct FILE_DESC fileInfo;

  sockfd=socket(AF_INET,SOCK_STREAM,0); //建立socket
  if(sockfd==-1)
  {
    printf("socket failed:%d",errno);
  }

  dest_addr.sin_family=AF_INET;
  dest_addr.sin_port=htons(DEST_PORT);
  dest_addr.sin_addr.s_addr=inet_addr(DEST_IP);
  bzero(&(dest_addr.sin_zero),8);
  
  if(connect(sockfd,(struct sockaddr*)&dest_addr,sizeof(struct sockaddr))==-1)
  {
    printf("connect failed:%d",errno);//失败时可以打印errno
  }
  else
  {
    printf("connect success\n");
    recv_info = recv(sockfd, (char *)&fileInfo, MAX_DATA, 0);
    if(recv_info == -1)
    {
      printf("recv info failed: %d",errno);
      return -1;
    }
    send(sockfd, "OK", 3, 0);
    fileSize = fileInfo.fileSize;
    strcpy(fileName, fileInfo.fileName);
    pBuf = (char *)malloc(fileSize);
    recv_file = recv(sockfd, pBuf, fileSize, 0);
    if (recv_file == -1)
    {
      printf("recv file failed: %d",errno);
      return -1;
    }

    strcpy(recv_fileName, "recv_");
    strcat(recv_fileName, fileName);

    fp = fopen(recv_fileName, "w");

    fwrite(pBuf, fileSize, 1, fp);

    printf("file_name: %s\n", fileName);
    printf("file_size: %lld\n", fileSize);
    printf("recv_info: %d\n", recv_info);
    printf("recv_file: %d\n", recv_file);
    printf("sizeof('OK'): %ld\n", sizeof("OK"));
    printf("(char *)&fileInfo.fileName: %s\n", (char *)&fileInfo.fileName);
    printf("(char *)&fileInfo.fileSize: %s\n", (char *)&fileInfo.fileSize);
  }

  close(sockfd);

  return 1;
}   
