#include <stdint.h>
#include <stddef.h>
#include <drivers/framebuffer/kprint.h>
#include <mem/kmalloc.h>

#include <cpu/thread.h>

thread_t* thread_list[MAX_THREADS] = {0};
int current_thread_id = -1;
int next_thread_id = 0;

// Initialize threading system
void thread_init(void) {
    for (int i = 0; i < MAX_THREADS; i++)
        thread_list[i] = NULL;

    kprint_ok("Threading initialized.");
}

// Create new thread
void thread_create(void (*entry)(void)) {
    if (next_thread_id >= MAX_THREADS) {
        kprint_error("Max threads reached.");
        return;
    }

    thread_t *t = kmalloc(sizeof(thread_t));
    if (!t) return;

    t->id = next_thread_id++;
    t->state = THREAD_READY;
    t->entry_point = entry;

    // allocate 8 KB stack
    t->stack = kmalloc(8192);
    if (!t->stack) {
        kfree(t);
        return;
    }

    // stack grows downward
    uint64_t *sp = (uint64_t*)((uint8_t*)t->stack + 8192);

    // push entry address so 'ret' jumps to it
    *(--sp) = (uint64_t)entry;

    // save stack pointer
    t->rsp = sp;

    thread_list[t->id] = t;

    kprintf("Created thread %d\n", t->id);
}

// Simple round-robin scheduler
void scheduler(void) {
    int next = -1;

    for (int i = 0; i < MAX_THREADS; i++) {
        if (thread_list[i] && thread_list[i]->state == THREAD_READY) {
            next = i;
            break;
        }
    }

    if (next == -1)
        return;

    if (current_thread_id == next)
        return;

    thread_t *old = NULL;
    if (current_thread_id >= 0)
        old = thread_list[current_thread_id];

    thread_t *new = thread_list[next];
    new->state = THREAD_RUNNING;

    int old_id = current_thread_id;
    current_thread_id = next;

    if (old) {
        old->state = THREAD_READY;
        switch_context(&old->rsp, new->rsp);
    } else {
        // first ever thread switch (no previous thread)
        uint64_t dummy = 0;
        switch_context(&dummy, new->rsp);
    }
}

// yield CPU
void thread_yield(void) {
    scheduler();
}
