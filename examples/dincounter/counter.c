#define _GNU_SOURCE
#include <stdio.h>
#include "counter.h"
#include <unistd.h>
#include <stdlib.h>

int count(){
    return x++;
}

char *hello(int x){
    char *s = NULL;
    asprintf(&s, "hello %i", x);
    return s;
}

struct mystruct *function(){
    struct mystruct *n;
    n = (struct mystruct*) malloc(sizeof(struct mystruct));
    n->string = hello(5);
    n->number = 5;
    return n;
}
