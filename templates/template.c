$warning

/*
*   template.c: template file
*
*   Copyright 2016 Mattia Maldini 
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License along
*   with this program; if not, write to the Free Software Foundation, Inc.,
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#define _GNU_SOURCE
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <sys/syscall.h>

#include "wrapper.h"

$headers


int add_${libname}_handle();
int current_${libname}_handle();
int switch_${libname}_handle(int ld);
static struct lib_handle *get_${libname}_handle(int ld);

/*list common to all library instances. No need for access control, since
 it is only written at the beginning by the constructor.*/
struct thread_manager_list *thread_manager_head;


static void __destructor(void* arg);

static int ${libname}_last;
static const char *${libname}_libname = "$lib";
static pthread_key_t ${libname}_handle_key;
static struct lib_handle *${libname}_head;
static struct lib_handle *default_${libname}_handle;

static int lock;
static struct timespec waiting = {
    0,
    1000
};


static int futex_wait(void *addr, int val1) {
    return syscall(__NR_futex, addr, val1, NULL, NULL, 0);
}

static int futex_wake(void *addr, int n) {
    return syscall(__NR_futex, addr, n, NULL, NULL, 0);
}

__attribute__ ((constructor((102)))) static void __wrapper_init(){
    int i;
    struct thread_manager_list *new, *tmp;
    //printf("initializing ${libname}\n");

    new = (struct thread_manager_list*) malloc(sizeof(struct thread_manager_list));
    new->current = current_${libname}_handle;
    new->switcher = switch_${libname}_handle;
    new->next = NULL;
    lock = 0;

    if (thread_manager_head == NULL)
        thread_manager_head = new; 
    else{
        new->next = thread_manager_head;
        thread_manager_head = new;
    }
    ${libname}_head = NULL;
    ${libname}_last = 0;
    pthread_key_create(&${libname}_handle_key, __destructor);
    if (i = add_${libname}_handle() == -1)
        exit(-1);
    default_${libname}_handle = get_${libname}_handle(i);
}


/****************************************
*  functions to manage loaded library   *
*  instances                            *
*****************************************/

static int get_${libname}_ld(){
    return ${libname}_last++;
}


static struct lib_handle *get_${libname}_handle(int ld){
    struct lib_handle* aux;

    while (__sync_lock_test_and_set(&lock, 1)) futex_wait(&lock, 1);

    aux = ${libname}_head;
    while (aux && (aux->ld != ld))
        aux = aux->next;

    lock = 0;
    futex_wake(&lock, INT_MAX);
    return aux;
}


static void __delete(int ld){
    struct lib_handle *aux, *tmp;
    if (!${libname}_head)
        return;
    aux = ${libname}_head;
    if (${libname}_head->ld == ld){
        ${libname}_head = ${libname}_head->next;
        dlclose(aux->handle);
        free(aux);
    }
    else{
        while (aux->next && aux->next->ld != ld)
            aux = aux->next;
        if (!aux->next)
            return;
        tmp = aux->next;
        aux->next = tmp->next;
        dlclose(tmp->handle);
        free(tmp);
    }   
}


static void __destructor(void* handle){

    while (__sync_lock_test_and_set(&lock, 1)) futex_wait(&lock, 1);
    ((struct lib_handle*)handle)->thread_count--;
    if (((struct lib_handle*)handle)->thread_count < 1)
        __delete(((struct lib_handle*)handle)->ld);

    lock = 0;
    futex_wake(&lock, INT_MAX);
}

/*add a new library instance*/
int add_${libname}_handle(){
    char *error;
    struct lib_handle *new, *aux;

    new = (struct lib_handle*) malloc(sizeof(struct lib_handle*));
    if ((new->handle = dlmopen(LM_ID_NEWLM, ${libname}_libname, RTLD_LAZY)) == NULL){
        error = dlerror();
        if (error)
            printf("%s\n", error);
        return -1;
    }
    new->ld = get_${libname}_ld();
    new->thread_count = 1;
    
    while (__sync_lock_test_and_set(&lock, 1)) futex_wait(&lock, 1);

    if (${libname}_head){
        aux = ${libname}_head->next;
        ${libname}_head->next = new;
        new->next = aux;
    }
    else
        ${libname}_head = new;

    if (pthread_setspecific(${libname}_handle_key, new) != 0)
        return -1;

    futex_wake(&lock, INT_MAX);
    lock = 0;
    return new->ld;
}


/*get current library instance (as a "library descriptor" ld)*/
int current_${libname}_handle(){
    struct lib_handle* cur;
    if ((cur = (struct lib_handle*) pthread_getspecific(${libname}_handle_key)) == NULL)
        cur = default_${libname}_handle;
    return cur->ld;
}


/*change currently used library instance*/
int switch_${libname}_handle(int ld){
    struct lib_handle *new, *cur;
    int old;
    if ((cur = (struct lib_handle*) pthread_getspecific(${libname}_handle_key)) == NULL)
        cur = default_${libname}_handle;
    if ((new = get_${libname}_handle(ld)) == NULL)
        return -1;
    if (cur){
        old = cur->ld;
        cur->thread_count--;
    }
    else
        old = -1;

    pthread_setspecific(${libname}_handle_key, new);
    new->thread_count++;
    return old;
}


/*change default library instance for thread that are still missing one*/
int switch_${libname}_default_handle(int ld){
    struct lib_handle *new, *cur;
    int old;
    cur = default_${libname}_handle;
    if ((new = get_${libname}_handle(ld)) == NULL)
        return -1;
    old = cur->ld;

    default_${libname}_handle = new;

    return old;
}
 


/********************************
*   automatically generated     *
*   functions                   *
*********************************/
    
$functions
