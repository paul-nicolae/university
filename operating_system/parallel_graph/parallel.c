/* SPDX-License-Identifier: BSD-3-Clause */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "os_list.h"

#define MAX_TASK 100
#define MAX_THREAD 4

int sum = 0;
os_graph_t *graph;
os_threadpool_t *threadpool;

pthread_mutex_t *sum_nodes;
pthread_mutex_t *mutexes;

void process_node(void *arg)
{
    int *nodeId = (int *)arg;
    os_node_t *node = graph->nodes[*nodeId];

    pthread_mutex_lock(sum_nodes);
    sum += node->nodeInfo;
    pthread_mutex_unlock(sum_nodes);

    for (int i = 0; i < node->cNeighbours; i++) {
        pthread_mutex_lock(&mutexes[node->neighbours[i]]);
        if (graph->visited[node->neighbours[i]] == 0) {
            graph->visited[node->neighbours[i]] = 1;

            // every task should have is own argument,
            // that's why we always allocate memory for index
            int *index = (int *)malloc(sizeof(int));
            *index = node->neighbours[i];

            // create a new task
            os_task_t *new_task = task_create(index, process_node);

            // add the new created task in queue
            add_task_in_queue(threadpool, new_task);
        }
        pthread_mutex_unlock(&mutexes[node->neighbours[i]]);
    }
}

void traverse_graph()
{   
    for (int i = 0; i < graph->nCount; i++) {
        pthread_mutex_lock(&mutexes[i]);
        if (graph->visited[i] == 0) {
            graph->visited[i] = 1;

            // every task should have is own argument,
            // that's why we always allocate memory for index
            int *index = (int *)malloc(sizeof(int));
            *index = i;

            // create a new task
            os_task_t *new_task = task_create(index, process_node);

            // add the new created task in queue
            add_task_in_queue(threadpool, new_task);
        }
        pthread_mutex_unlock(&mutexes[i]);
    }
}

int processing_is_done(os_threadpool_t *tp)
{   
    // verify if we still have unvisited nodes
    for (int i = 0; i < graph->nCount; i++) {
        if (graph->visited[i] == 0) return 0;
    }

    // verify if we still have tasks in queue
    return tp->tasks == NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./main input_file\n");
        exit(1);
    }

    FILE *input_file = fopen(argv[1], "r");

    if (input_file == NULL) {
        printf("[Error] Can't open file\n");
        return -1;
    }

    graph = create_graph_from_file(input_file);
    if (graph == NULL) {
        printf("[Error] Can't read the graph from file\n");
        return -1;
    }

    // with this mutex we are avoiding race condition over sum
    sum_nodes = malloc(sizeof(pthread_mutex_t));
    if (!sum_nodes) exit(1);
    pthread_mutex_init(sum_nodes, NULL);

    // every node of the graph should have a mutex for better concurency
    mutexes = malloc(sizeof(pthread_mutex_t) * graph->nCount);
    if (!mutexes) exit(1);

    for (int i = 0; i < graph->nCount; i++) {
        pthread_mutex_init(&mutexes[i], NULL);
    }

    threadpool = threadpool_create(MAX_TASK, MAX_THREAD);

    traverse_graph();

    threadpool_stop(threadpool, processing_is_done);

    pthread_mutex_destroy(sum_nodes);

    for (int i = 0; i < graph->nCount; i++) {
        pthread_mutex_destroy(&mutexes[i]);
    }

    free(sum_nodes);
    free(mutexes);

    free(threadpool->threads);
    free(threadpool);

    printf("%d", sum);
    return 0;
}
