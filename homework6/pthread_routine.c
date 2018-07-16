//
// gcc pthread_routine.c -lpthread -o pthread_routine
// ./pthread_routine <arg1>
//
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

struct TEST
{
  unsigned int one;
  unsigned int two;
};

void *test_func(void *arg)
{
  struct TEST *test;
  test = (struct TEST *)arg;      
  printf("test->one:%d\n",test->one);
  printf("test->two:%d\n",test->two);
}

void *test_func2(void *arg)
{
  int test2 = (int *)arg;
  printf("test2: %x\n", test2);
}

void *test_func3(void *arg)
{
  int test31 = (int *)arg[0];
  int test32 = (int *)arg[1];
  printf("test31: %d\n", test31);
  printf("test32: %d\n", test32);
}

int main (int argc, char **argv)
{
  struct TEST test;

  test.one = 111;
  test.two = 222;

  int test2;
  sscanf(argv[1], "%x", &test2);

  int test3[2] = {666, 777};

  pthread_t test_pthread;
  pthread_t test_pthread2;
  pthread_t test_pthread3;

  pthread_create(&test_pthread, NULL, &test_func, (void *)&test);
  pthread_create(&test_pthread2, NULL, &test_func2, (void *)test2);
  pthread_create(&test_pthread3, NULL, &test_func3, (void *)&test3);

  pthread_join(test_pthread, NULL);
  pthread_join(test_pthread2, NULL);
  pthread_join(test_pthread3, NULL);

  return 0;
}

