#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

void *functionCount1();
void *functionCount2();
void *functionCount3();

int  count = 0;
#define COUNT_DONE  10
#define COUNT_HALT1  3
#define COUNT_HALT2  6

int main()
{
   pthread_t thread1, thread2, thread3;

   pthread_create( &thread1, NULL, &functionCount1, NULL);
   pthread_create( &thread2, NULL, &functionCount2, NULL);
   pthread_create( &thread3, NULL, &functionCount3, NULL);

   pthread_join( thread1, NULL);
   pthread_join( thread2, NULL);
   pthread_join( thread3, NULL);

   exit(0);
}

// Write numbers 1-3 and 8-10 as permitted by functionCount2()

void *functionCount1()
{
   for(;;)
   {
      // Lock mutex and then wait for signal to relase mutex
      pthread_mutex_lock( &count_mutex );

      // Wait while functionCount2() operates on count
      // mutex unlocked if condition varialbe in functionCount2() signaled.
      //pthread_cond_wait( &condition_var, &count_mutex );
      printf("one:%d\n", count);
      printf("\n");
      count++;
      if (count >= 10) {
        pthread_cond_signal( &condition_var );
        
      }
      pthread_mutex_unlock( &count_mutex );
    }
}

// Write numbers 4-7

void *functionCount2()
{
    for(;;)
    {
       pthread_mutex_lock( &count_mutex );
       //pthread_cond_signal( &condition_var );
       printf("two:%d\n", count);
       printf("\n");
       pthread_mutex_unlock( &count_mutex );

    }
}

void *functionCount3()
{
    for(;;)
    {
       pthread_mutex_lock( &count_mutex );
       pthread_cond_wait( &condition_var, &count_mutex );
       
       printf("beda! \n");

       pthread_mutex_unlock( &count_mutex );

    }
}