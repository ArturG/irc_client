
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

sem_t mutex;

void send_message1(void* arg)
{
  while (1)
  {
    // seems that semaphore doesn't work
    sem_wait(&mutex);
    printf("oneoneoneoneoneoneone");
    printf("\n");
    fflush(stdout);
    sem_post(&mutex);
  }
}

void send_message2(void* arg)
{
  while (1)
  {
    sem_wait(&mutex);
    printf("twotwotwotwotwotwotwotwo");
    printf("\n");
    fflush(stdout);
    sem_post(&mutex);
  }
}

void send_message3(void* arg)
{
  while (1)
  {
    sem_wait(&mutex);
    printf("threethreethreethreethree");
    printf("\n");
    fflush(stdout);
    sem_post(&mutex);
  }
}

void send_message4(void* arg)
{
  while (1)
  {
    sem_wait(&mutex);
    printf("fourfourfourfourfourfour");
    printf("\n");
    sem_post(&mutex);
  }
}

int main(int argc, char *argv[])
{
  sem_init (&mutex, 0, 1);
  pthread_t receiver;
  pthread_t sender;
  pthread_t timer;
  pthread_t timer2;
  
  if (pthread_create (&receiver, NULL, (void *) &send_message1, NULL) != 0) 
  {
    fprintf(stderr, ".:. Couldn't create thread .:.\n"); 
  }

  if (pthread_create (&sender, NULL, (void *) &send_message2, NULL) != 0)
  {
    fprintf(stderr, ".:. Couldn't create thread .:.\n"); 
  }

  if (pthread_create (&timer, NULL, (void *) &send_message3, NULL) != 0)
  {
    fprintf(stderr, ".:. Couldn't create thread .:.\n"); 
  }
        
  if (pthread_create (&timer2, NULL, (void *) &send_message4, NULL) != 0)
  {
    fprintf(stderr, ".:. Couldn't create thread .:.\n"); 
  }

  sleep(10);

  pthread_join(receiver, NULL);
  pthread_join(sender, NULL);
  pthread_join(timer, NULL);
  pthread_join(timer2, NULL);

  return 0;
}
