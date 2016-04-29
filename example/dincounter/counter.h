#ifndef __COUNTER_H__
#define __COUNTER_H__

struct mystruct {
    char *string;
    int number;
};

static int x = 0;

int count();

char *hello(int x);

struct mystruct *function();

#endif
