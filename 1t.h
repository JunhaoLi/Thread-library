/*
 * 1t.h -- public interface to Project 1t thread library
 *
 * This file should be included in both the thread library and application
 * programs that use the thread library.
 */
#ifndef _1T_H
#define _1T_H

#define STACK_SIZE 262144	/* size of each thread's stack */

typedef void (*dthreads_func_t) (void *);

extern int dthreads_init(dthreads_func_t func, void *arg);
extern int dthreads_start(dthreads_func_t func, void *arg);
extern int dthreads_yield(void);

/* dthreads_seminit: takes an initial value for a semaphore and returns
 * a unique semaphore identifier or -1 for an error */
extern int dthreads_seminit(unsigned int sem, unsigned int value);
/* dthreads_semup: takes the identifier for an initialized semaphore and
 * increments its value */
extern int dthreads_semup(unsigned int sem);
/* dthreads_semdown: takes the identifier for an initialized semaphore
 * and returns after the semaphore's value has been decremented by
 * the calling thread; a semaphore's value can only be decremented if
 * its value if it is greater than 0; returns 0 on success or -1 for
 * an error */ 
extern int dthreads_semdown(unsigned int sem);

/*
 * begin_preemptions() can be used in testing to configure the generation
 * of interrupts (which in turn lead to preemptions).
 *
 * The sync and async parameters allow several styles of preemptions:
 *
 *     1. async = true: generate asynchronous preemptions every 10 ms using
 *        SIGALRM.  These are non-deterministic.
 *
 *     2. sync = true: generate synchronous, pseudo-random preemptions before
 *        interrupt_disable and after interrupt_enable.  You can generate
 *        different (but deterministic) preemption patterns by changing
 *        random_seed.
 *
 * begin_preemptions() should be called (at most once) in the application
 * function started by dthreads_libinit().  Make sure this is after the thread
 * system is done being initialized.
 *
 * If begin_preemptions() is not called, no interrupts will be generated.
 *
 * The code for begin_preemptions is in interrupt.cc, but the declaration
 * is in 1t.h because it's part of the public thread interface.
 */
extern void begin_preemptions(bool async, bool sync, int random_seed);

#endif /* _1T_H */

