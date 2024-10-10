#include "printers.h"

int rand_max(int max) {
    return rand() % max;
}

char get_random_char() {
    return 97 + rand() % 26; 
}

void generate_new_doc(char* doc) {
    for (size_t i = 0; i < DOC_SIZE; ++i) {
        doc[i] = get_random_char();
    }
}

void print_doc(char* doc) {
    for (size_t i = 0; i < DOC_SIZE; ++i) {
        printf("%c", doc[i]);
    }
    printf("\n");
}

int main(void) {
    srand(time(NULL));

    int queue_fd = get_queue_fd();
    queue_t* queue = get_queue(queue_fd);
    
    sem_t* taken = get_semaphone(Q_TAKEN_SEM_NAME);
    sem_t* empty = get_semaphone(Q_EMPTY_SEM_NAME);
    sem_t* entry = get_semaphone(Q_ENTRY_SEM_NAME);

    char doc[DOC_SIZE];
    while (1) {
        generate_new_doc(doc);

        sem_wait(empty);
        sem_wait(entry);

        q_insert(queue, doc);

        printf("Posted: ");
        print_doc(doc);
        
        sem_post(entry);
        sem_post(taken);

        sleep(rand_max(10));
    }
}