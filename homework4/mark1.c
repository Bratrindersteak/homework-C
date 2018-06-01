#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

sem_t morra_sem; // 徒弟出拳信号.

//int master_hand;
char *master_hand_name; // 师父出拳名称.
int apprentice_hand;  // 徒弟出拳ID.
char *apprentice_hand_name;  // 徒弟出拳名称.

void *master_func(void *arg)  // 师父线程方法.
{
  sem_wait(&morra_sem);  // 等待徒弟出拳.
  
  switch(apprentice_hand)
  {
    case 0:
      master_hand_name = "锤";
      break;
    case 1:
      master_hand_name = "剪";
      break;
    case 2:
      master_hand_name = "包";
      break;
    default:
      master_hand_name = "...徒弟不按套路出牌，不玩了！";
  }

  printf("师父出了：%s\n\n", master_hand_name);
  printf("师父胜!\n");
}

void *apprentice_func(void *arg)  // 徒弟线程方法.
{
  printf("徒弟先出：");
  scanf("%d", &apprentice_hand);  // 徒弟出拳.

  while(!(apprentice_hand >= 0 && apprentice_hand <= 2 || apprentice_hand == 99))  // 判断徒弟出拳合法性.
  {
    printf("不存在这种操作，请重新输入：");
    scanf("%d", &apprentice_hand);
  }

  switch(apprentice_hand)
  {
    case 0:
    case 1:
    case 2:
      sem_post(&morra_sem);  // 正确出拳.

      if (apprentice_hand == 0)
      {
        apprentice_hand_name = "剪";
      }
      else if (apprentice_hand == 1)
      {
        apprentice_hand_name = "包";
      }
      else
      {
        apprentice_hand_name = "锤";
      }

      printf("\n徒弟出了: %s\n", apprentice_hand_name);
      break;

    case 99:
      printf("徒弟不玩了！～\n");
      exit(0);
      break;

    default:
      printf("意外崩溃了...\n");
      exit(1);
    }
}


int main () {
  sem_init(&morra_sem, 0, 0);

  pthread_t master_pthread;
  pthread_t apprentice_pthread;

  pthread_create(&master_pthread, NULL, master_func, NULL);
  pthread_create(&apprentice_pthread, NULL, apprentice_func, NULL);
  
  pthread_join(master_pthread, NULL);
  pthread_join(apprentice_pthread, NULL);

  sem_destroy(&morra_sem);

  return 0;
}
