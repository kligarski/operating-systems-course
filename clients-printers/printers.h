#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#define QUEUE_SIZE 3
#define DOC_SIZE 10

#define Q_TAKEN_SEM_NAME "/printers_queue_taken"
#define Q_EMPTY_SEM_NAME "/printers_queue_empty"
#define Q_ENTRY_SEM_NAME "/printers_queue_entry"

#define QUEUE_SM_NAME "/printers_queue"

typedef struct queue_t {
    size_t size;
    size_t head;
    char docs[QUEUE_SIZE][DOC_SIZE];
} queue_t;

void error_exit(const char *err);

int get_queue_fd(void);
queue_t* get_queue(int queue_fd);
sem_t* get_semaphone(const char* name);

int q_empty(queue_t* queue);
int q_full(queue_t* queue);
int q_insert(queue_t* queue, const char* doc);
int q_delete(queue_t* queue, char* doc_dest);
int q_is_in_range(queue_t* queue, size_t i);
void q_print_normal_indexes(queue_t* queue);
void q_print(queue_t* queue);