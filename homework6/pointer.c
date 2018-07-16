//
// gcc pointer.c -o pointer
// ./pointer
//
#include <stdio.h>
// #include <stdlib.h>

int main ()
{
  int num;
  int arr[2] = {666, 777};
  int arr2[2];

  arr2[0] = 888;
  arr2[1] = 999;

  printf("arr[0]: %d\n", arr[0]);
  printf("arr[1]: %d\n", arr[1]);

  printf("arr2[0]: %d\n", arr2[0]);
  printf("arr2[1]: %d\n", arr2[1]);
}
