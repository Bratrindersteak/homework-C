#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

sem_t sem_send;
sem_t sem_recv;

int dwExit = 1;

void *send_func(void *arg)
{
	while (dwExit != 0)
	{
		sem_wait(&sem_send);
		printf("sending...\n");
	}
}

void *recv_func(void *arg)
{
	sem_wait(&sem_recv);
	printf("recving...\n");
	sem_post(&sem_send);
}

int main (int argc, char **argv)
{
  sem_init(&sem_send, 0, 0);
  sem_init(&sem_recv, 0, 0);

  pthread_t send_pthread;
  pthread_t recv_pthread;

  pthread_create(&send_pthread, NULL, send_func, NULL);
  pthread_create(&recv_pthread, NULL, recv_func, NULL);

  sem_post(&sem_send);

  // while (dwExit != 0)
  // {
    sem_post(&sem_recv);
    printf("in main\n");
  // }

  
  pthread_join(send_pthread, NULL);
  pthread_join(recv_pthread, NULL);

  sem_destroy(&sem_send);
  sem_destroy(&sem_recv);

  return 0;
}
