/* SPDX-License-Identifier: BSD-3-Clause */

#include "os_threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

/* === TASK === */

/* Creates a task that thread must execute */
os_task_t *task_create(void *arg, void (*f)(void *))
{
    os_task_t *task = (os_task_t *)malloc(sizeof(os_task_t));
    if (!task) exit(1);

    task->argument = arg;
    task->task = f;

    return task;
}

/* Add a new task to threadpool task queue */
void add_task_in_queue(os_threadpool_t *tp, os_task_t *t)
{
    pthread_mutex_lock(&tp->taskLock);

    // if task queue is empty
    if (!tp->tasks) {
        os_task_queue_t *new_rear = (os_task_queue_t *)malloc(sizeof(os_task_queue_t));
        if (!new_rear) exit(1);

        new_rear->task = t;
        new_rear->next = NULL;
        tp->tasks = new_rear;
    
        pthread_mutex_unlock(&tp->taskLock);

        return ;
    }
    
    os_task_queue_t *rear = tp->tasks;
    while (rear->next) { rear = rear->next; }

    // create the task to be be added
    os_task_queue_t *new_rear = (os_task_queue_t *)malloc(sizeof(os_task_queue_t));
    if (!new_rear) exit(1);

    new_rear->task = (os_task_t *)malloc(sizeof(os_task_t));
    if (!new_rear->task) exit(1);

    new_rear->task = t;
    new_rear->next = NULL;

    // Enqueue()
    // add the new task at the end of the queue
    rear->next = new_rear;

    pthread_mutex_unlock(&tp->taskLock);
}

/* Get the head of task queue from threadpool */
os_task_t *get_task(os_threadpool_t *tp)
{   
    pthread_mutex_lock(&tp->taskLock);

    if (!tp->tasks) {
        pthread_mutex_unlock(&tp->taskLock);
        return NULL;
    }

    os_task_queue_t *node = tp->tasks;
    os_task_t *front = tp->tasks->task;
    if (!front) {
        pthread_mutex_unlock(&tp->taskLock);
        return NULL;
    } 

    // Dequeue()
    // remove the first task from the queue, then return it
    tp->tasks = tp->tasks->next;
    
    pthread_mutex_unlock(&tp->taskLock);

    // free the removed node
    free(node);

    return front;
}

/* === THREAD POOL === */

/* Initialize the new threadpool */
os_threadpool_t *threadpool_create(unsigned int nTasks, unsigned int nThreads)
{
    os_threadpool_t *threadpool = (os_threadpool_t *)malloc(sizeof(os_threadpool_t));
    if (!threadpool) exit(1);

    threadpool->should_stop = 0;
    threadpool->num_threads = nThreads;
    
    if (pthread_mutex_init(&threadpool->taskLock, NULL)) exit(1);

    threadpool->threads = (pthread_t *)malloc(sizeof(pthread_t) * nThreads);
    if (!threadpool->threads) exit(1);

    int error;

    // a thread pool creates the threads when the thread pool is created
    for (int i = 0; i < nThreads; i++) {
        error = pthread_create(&threadpool->threads[i], NULL,
                                thread_loop_function, threadpool);

        if (error) exit(1);
    }

    return threadpool;
}

/* Loop function for threads */
void *thread_loop_function(void *args)
{
    os_threadpool_t *threadpool = (os_threadpool_t *)args;

    while (1) {
        if (threadpool->should_stop) break;

        os_task_t *task = get_task(threadpool);

        if (task) {
            task->task(task->argument);

            // free the argument of the processed task
            // allocated in traver_graph() and process_node()
            free(task->argument);

            // free the done task
            free(task);
        }
    }

    return NULL;
}

/* Stop the thread pool once a condition is met */
void threadpool_stop(os_threadpool_t *tp, int (*processingIsDone)(os_threadpool_t *))
{
    while (!processingIsDone(tp));

    tp->should_stop = 1;

    for (int i = 0; i < tp->num_threads; i++) {
        pthread_join(tp->threads[i], NULL);
    }

    pthread_mutex_destroy(&tp->taskLock);
}
