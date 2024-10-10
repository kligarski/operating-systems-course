#include "printers.h"

int main(void) {
    int queue_fd = get_queue_fd();
    queue_t* queue = get_queue(queue_fd);
    
    sem_t* taken = get_semaphone(Q_TAKEN_SEM_NAME);
    sem_t* empty = get_semaphone(Q_EMPTY_SEM_NAME);
    sem_t* entry = get_semaphone(Q_ENTRY_SEM_NAME);

    char doc[DOC_SIZE];
    while (1) {
        sem_wait(taken);
        sem_wait(entry);

        q_delete(queue, doc);

        sem_post(entry);
        sem_post(empty);

        for (size_t i = 0; i < DOC_SIZE; ++i) {
            sleep(1);
            printf(" %c\n", doc[i]);
        }
        printf("---\n");
    }
}