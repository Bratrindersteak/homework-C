#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>

int main()
{
  int fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0)
  {
    perror("cannot create socket");
    return 0;
  }

  printf("created socket, fd: %d\n", fd);
  exit(0);
}
