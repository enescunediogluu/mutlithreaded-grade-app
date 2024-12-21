#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>
#include<semaphore.h>

#define THREAD_NUM 2

int buffer[10];
int count = 0;

int main() {
    printf("hello world\n");
}