#include "printers.h"
#include <unistd.h>

int create_and_get_queue_fd(void) {
    int queue_fd = shm_open(QUEUE_SM_NAME, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (queue_fd == -1) {
        error_exit("creating queue");
    }

    return queue_fd;
}

void resize_queue(int queue_fd) {
    if (ftruncate(queue_fd, sizeof(queue_t)) == -1) {
        error_exit("resizing queue");
    }
}

sem_t* create_semaphore(const char* name, unsigned int value) {
    sem_t* sem_fd = sem_open(name, O_CREAT | O_RDONLY, S_IRUSR | S_IWUSR, value);
    if (sem_fd == SEM_FAILED) {
        error_exit("creating semaphore");
    }

    return sem_fd;
}

int main(void) {
    int queue_fd = create_and_get_queue_fd();
    resize_queue(queue_fd);

    queue_t* queue = get_queue(queue_fd);
    memset(queue, 0, sizeof(queue_t));
    
    sem_unlink(Q_TAKEN_SEM_NAME);
    sem_unlink(Q_EMPTY_SEM_NAME);
    sem_unlink(Q_ENTRY_SEM_NAME);

    sem_t* taken = create_semaphore(Q_TAKEN_SEM_NAME, 0);
    sem_t* empty = create_semaphore(Q_EMPTY_SEM_NAME, QUEUE_SIZE);
    sem_t* entry = create_semaphore(Q_ENTRY_SEM_NAME, 1);

    int taken_v, empty_v, entry_v;
    while (1) {
        // Clear terminal
        printf("\033[H\033[J");

        sem_getvalue(taken, &taken_v);
        sem_getvalue(empty, &empty_v);
        sem_getvalue(entry, &entry_v);

        printf("TAKEN semaphore: %d\n", taken_v);
        printf("EMPTY semaphore: %d\n", empty_v);
        printf("ENTRY semaphore: %d\n\n\n", entry_v);

        q_print(queue);

        usleep(10000);
    }

    return 0;
}