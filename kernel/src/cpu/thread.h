#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>

typedef enum {
    THREAD_RUNNING,
    THREAD_READY,
    THREAD_BLOCKED,
    THREAD_TERMINATED
} thread_state_t;

typedef struct thread {
    uint64_t *stack;       // base of stack allocation
    uint64_t *rsp;         // saved stack pointer
    thread_state_t state;
    uint64_t id;
    void (*entry_point)(void);
} thread_t;

#define MAX_THREADS 16
extern thread_t* thread_list[MAX_THREADS];
extern int current_thread_id;
extern int next_thread_id;

void thread_init(void);
void thread_create(void (*entry_point)(void));
void thread_yield(void);
void scheduler(void);

// implemented in assembly
void switch_context(uint64_t **old_rsp, uint64_t *new_rsp);

#endif
