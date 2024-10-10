#include "printers.h"

void error_exit(const char *err) {
    perror(err);
    exit(1);
}

int get_queue_fd(void) {
    int queue_fd = shm_open(QUEUE_SM_NAME, O_RDWR, 0);
    if (queue_fd == -1) {
        error_exit("opening queue");
    }
    return queue_fd;
}

queue_t* get_queue(int queue_fd) {
    void* mapped_mem = mmap(NULL, sizeof(queue_t), PROT_WRITE, MAP_SHARED, queue_fd, 0);
    if (mapped_mem == MAP_FAILED) {
        error_exit("mapping queue");
    }

    return (queue_t*)mapped_mem;
}

sem_t* get_semaphone(const char* name) {
    sem_t* sem_fd = sem_open(name, S_IRUSR | S_IWUSR);
    if (sem_fd == SEM_FAILED) {
        error_exit("opening semaphore");
    }
    return sem_fd;
}

int q_empty(queue_t* queue) {
    return queue->size == 0;
}

int q_full(queue_t* queue) {
    return queue->size == QUEUE_SIZE;
}

int q_insert(queue_t* queue, const char* doc) {
    if (q_full(queue)) {
        return -1;
    }

    memcpy(queue->docs[(queue->head + queue->size) % QUEUE_SIZE], doc, DOC_SIZE);
    
    ++queue->size;

    return 0;
}

int q_delete(queue_t* queue, char* doc_dest) {
    if (q_empty(queue)) {
        return -1;
    }

    memcpy(doc_dest, queue->docs[queue->head], DOC_SIZE);

    queue->head = (queue->head + 1) % QUEUE_SIZE;
    --queue->size;

    return 0;
}

int q_is_in_range(queue_t* queue, size_t i) {
    if (queue->size == 0) {
        return 0;
    }
    return (i >= queue->head && i < queue->head + queue->size) 
        || (i >= 0 && i < (queue->head + queue->size) % QUEUE_SIZE);
}

void q_print_normal_indexes(queue_t* queue) {
    printf("Size: %ld\n", queue->size);
    printf("Head: %ld\n", queue->head);
    for (size_t i = 0; i < QUEUE_SIZE; ++i) {
        printf("q[%ld]=", i);

        for (size_t j = 0; j < DOC_SIZE; ++j) {
            printf("%c", queue->docs[i][j] == '\0' ? ' ' : queue->docs[i][j]);
        }

        if (!q_is_in_range(queue, i)) {
            printf(" | rubbish value");
        }

        if (i == queue->head) {
            printf(" | HEAD");
        }

        printf("\n");
    }
    printf("\n");
}

void q_print(queue_t* queue) {
    printf("Queue size: %ld/%d\n", queue->size, QUEUE_SIZE);
    for (size_t i = 0; i < queue->size; ++i) {
        printf("%ld: ", i);

        size_t index = (queue->head + i) % QUEUE_SIZE;
        for (size_t j = 0; j < DOC_SIZE; ++j) {
            printf("%c", queue->docs[index][j] == '\0' ? ' ' : queue->docs[index][j]);
        }

        printf("\n");
    }
    printf("\n");
}