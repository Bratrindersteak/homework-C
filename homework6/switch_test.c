//
// gcc switch_test.c -o switch_test
// ./switch_test <cmd> <status>
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (int argc, char **argv)
{
  int cmd, status;
  sscanf(argv[1], "%x", &cmd);
  sscanf(argv[2], "%d", &status);
  printf("cmd: %04x\n", cmd);
  printf("status: %d\n", status);

  switch (cmd)
  {
    case 0x0FF0:
      if (status)
      {
        printf("INFO: Request looking for Master !!!\n");
      }
      break;
    case 0x0F01:
      if (status)
      {
        printf("INFO: Request to apply NodeID !!!\n");
      }
      break;
    case 0x00F0:
      if (!status)
      {
        printf("INFO: Reply from Master !!!\n");
      }
      break;
    case 0x0001:
      if (!status)
      {
        printf("INFO: NodeID from Master !!!\n");
      }
      break;
  }

  return 0;
}