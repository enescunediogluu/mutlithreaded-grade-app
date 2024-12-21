#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#define THREAD_NUM 2

int buffer[10];
int count = 0;

void *producer(void *args)
{
    //produce
    int x = rand() % 100;

    //add to the buffer
    buffer[count] = x;
    count++;
}

void *consumer(void *args)
{
    //remove from buffer
    int y = buffer[count - 1];
    count--;

    //consume
    printf("Got %d\n", y);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    pthread_t th[THREAD_NUM];
    int i;
    for (i = 0; i < THREAD_NUM; i++){
        if (i % 2 == 0){
            if (pthread_create(&th[i], NULL, &producer, NULL) != 0){
                perror("Failed to create thread!\n");
            }
        else{
            if (pthread_create(&th[i], NULL, &consumer, NULL) != 0){
                perror("Failed to create thread!\n");
            }
        }
        }
    }

    for(int i = 0; i<THREAD_NUM; i++){
        if(pthread_join(th[i],NULL) != 0){
        //
        }
    }
    return 0;
}
