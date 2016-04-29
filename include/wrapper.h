#ifndef __WRAPPER_H__
#define __WRAPPER_H__


struct list {
    int ld;
    int (*switcher) (int ld);
    struct list* next;
};

struct newargs{
    struct list* data;
    void *(*start_routine) (void*);
    void *args;
};

struct thread_manager_list {
    int (*current) ();
    int (*switcher) (int ld);
    struct thread_manager_list *next;
};

struct lib_handle {
    void *handle;
    int ld;
    int thread_count;
    struct lib_handle *next;
};


#endif
