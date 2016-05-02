#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "counter.h"
#include "libcounter-wrapper.h"

void* count_to(void *args){
    int x;
    x = *((int*)args);
    add_counter_handle();
    printf("thread %i: current handle: %i\n", (int)pthread_self(), current_counter_handle());
    while (x--){
        printf("thread %i counting: %i\n", (int)pthread_self(), count());
        sleep(1);
    }
    printf("thread %i done\n", (int)pthread_self());
    pthread_exit(0);
}

int main(){
    char *error;
    char buf[10];
    pthread_t thread;
    int *args = malloc(sizeof(int));

    while (read(STDIN_FILENO, buf, 10)){
        if (atoi(buf) > 0){
            *args = atoi(buf);
            pthread_create(&thread,NULL, count_to, args);
        }
        else{
            printf("main counter %i\n", count());
        }
        buf[0] = 0;
    }
    return 0;
}
