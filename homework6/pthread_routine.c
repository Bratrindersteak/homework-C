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

	// printf("*((int *)arg)): \n", (int *)arg[0]);
}

int main (int argc, char **argv)
{
  struct TEST *test;

  test->one = 111;
  test->two = 222;

  pthread_t test_pthread;
  // pthread_create(&test_pthread, NULL, &test_func, argv[1]);
  pthread_create(&test_pthread, NULL, &test_func, (void *)test);
  pthread_join(test_pthread, NULL);

  return 0;
}

