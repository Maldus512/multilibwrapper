/****************************************                                   
*   wrapper for thread creation,        *
*   to grant the same library instance  *
*   to each new thread                  *
*****************************************/

#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

#include "wrapper.h"

extern struct thread_manager_list* thread_manager_head;


static void* thread_set(void *args){
    void *real_args;
    struct newargs *my_args;
    struct list *aux;

    my_args = (struct newargs*) args;
    real_args = my_args->args;
    aux = my_args->data;

    while (aux){
        aux->switcher(aux->ld);
        aux = aux->next;
    }

    return my_args->start_routine(real_args);
}


int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg){
    struct newargs *my_args;
    struct list *desc, *tmp;
    struct thread_manager_list *aux;
    void *handle;
    printf("hijacked pthread\n");
    int (*dpthread_create) (pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);

    desc = NULL;
    aux = thread_manager_head;
    dpthread_create = dlsym(RTLD_NEXT, "pthread_create");


    while (aux){
        tmp = (struct list*) malloc(sizeof(struct list));
        tmp->ld = aux->current();
        tmp->switcher = aux->switcher;
        tmp->next = NULL;
        if (desc == NULL)
            desc = tmp;
        else{
            tmp->next = desc;
            desc = tmp;
        }
        aux = aux->next;
    }

    my_args = (struct newargs*) malloc(sizeof(struct newargs));

    my_args->data = desc;
    my_args->start_routine = start_routine;
    my_args->args = arg;

    return dpthread_create(thread, attr, thread_set, my_args); 
}


/*
int clone(int (*fn)(void *), void *child_stack, int flags, void *arg, ... ){
    va_list ap;
    pid_t *ptid, *ctid;
    struct user_desc *tls;
    int (*dclone) ();

    va_start(ap, arg);

    ptid = va_arg(ap, pid_t*);
    tls = va_arg(ap, struct user_desc*);
    ctid = va_arg(ap, pid_t*);

    dclone = dlsym(RTLD_NEXT, "clone");
    printf("cloned\n");

    return dclone(fn, child_stack, flags, arg, ptid, tls, ctid);
}
*/
