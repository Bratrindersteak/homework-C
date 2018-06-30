//
// gcc server_mark1.c -o server_mark1
// ./server_mark1
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
  
#define PORT 1500 //端口号
#define BACKLOG 5 //最大监听数
#define BUF_SIZE 4096

struct FILE_DESC
{
  unsigned long long fileSize;
  char fileName[64];
};

int main(int argc, char **argv)
{
  FILE *fp;
  unsigned long long fileSize;
  unsigned long long reader;
  int ii;
  char *buf;
  int sockfd,new_fd; //socket句柄和建立连接后的句柄
  struct sockaddr_in my_addr; //本方地址信息结构体，下面有具体的属性赋值
  struct sockaddr_in their_addr; //对方地址信息
  int sin_size;
  int feedback;
  char feedback_info[16];
  struct FILE_DESC fileInfo;

  //fileInfo.fileName = argv[1];
  strcpy(fileInfo.fileName, argv[1]);
  fp = fopen(argv[1], "r");
  fseek(fp, 0, SEEK_END);
  fileSize = ftell(fp);
  // printf("fileSizefileSizefileSize: %lld\n", fileSize);
  fileInfo.fileSize = fileSize;
  fseek(fp, 0, SEEK_SET);
  buf = (char *)malloc(fileSize);
  reader = fread(buf, 1, fileSize, fp);

  //printf("fileSize: %lld", fileSize);
  sockfd=socket(AF_INET,SOCK_STREAM,0); //建立socket

  if(sockfd==-1)
  {
      printf("socket failed:%d",errno);
      return -1;
  }

  my_addr.sin_family=AF_INET; //该属性表示接收本机或其他机器传输
  my_addr.sin_port=htons(PORT); //端口号
  my_addr.sin_addr.s_addr=htonl(INADDR_ANY); //IP，括号内容表示本机IP
  bzero(&(my_addr.sin_zero),8); //将其他属性置0

  if (bind(sockfd,(struct sockaddr*)&my_addr, sizeof(struct sockaddr))<0)
  { //绑定地址结构体和socket
      printf("bind error");
      return -1;
  }

  listen(sockfd, BACKLOG); //开启监听 ，第二个参数是最大监听数
  
  sin_size = sizeof(struct sockaddr_in);
  new_fd = accept(sockfd, (struct sockaddr*)&their_addr,&sin_size); //在这里阻塞知道接收到消息，参数分别是socket句柄，接收到的地址信息以及大小
  
  if (new_fd==-1)
  {
    printf("receive failed");
  }
  else
  {
    printf("receive success %d\n", new_fd);
    send(new_fd, (char *)&fileInfo, sizeof(fileInfo), 0);
    feedback = recv(new_fd, feedback_info, 3, 0);
    if (feedback == -1)
    {
      printf("feedback error");
    }
    printf("feedback: %d\n", feedback);
    printf("feedback_info: %s\n", feedback_info);

    send(new_fd, buf, fileSize, 0);
    // if (feedback_info == "ok")
    // {
    //   printf("fileSize: %lld", fileSize);
    // }
    //send(new_fd, buf, fileSize, 0);
    // send(new_fd, (char *)&fileInfo, sizeof(fileInfo), 0); //发送内容，参数分别是连接句柄，内容，大小，其他信息（设为0即可）
    close(new_fd);
  }

  close(sockfd);
  fclose(fp);

  return 1;
}
