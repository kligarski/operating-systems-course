#define _POSIX_C_SOURCE 200809L

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#define USERNAME_LENGTH 16 // with null-byte
#define MAX_CLIENTS 10
#define MAX_MSG_LENGTH 256 // with null-byte

void error_exit(const char *msg);

typedef unsigned char operation_t;
#define LIST 0
#define TO_ALL 1
#define TO_ONE 2
#define STOP 3
#define ALIVE 4
#define INIT 5

struct list_data
{
    char usernames[MAX_CLIENTS][USERNAME_LENGTH];
};

struct to_all_data
{
    char msg[MAX_MSG_LENGTH];
};

struct to_one_data
{
    char username[USERNAME_LENGTH];
    char msg[MAX_MSG_LENGTH];
};

struct init_data
{
    char username[USERNAME_LENGTH];
};

struct chat_msg
{
    operation_t operation;
    time_t time;
    union
    {
        struct to_all_data to_all;
        struct to_one_data to_one;
        struct list_data list;
        struct init_data init;
    } data;
};

typedef unsigned char assigned_t;
#define UNASSIGNED 0
#define ANONYMOUS 1
#define ASSIGNED 2

#define UNASSIGNED_USERNAME "\0"
#define ANONYMOUS_USERNAME "Anonymous"

struct client_data
{
    assigned_t assigned;
    pthread_t client_thread;
    size_t client_id;
    int client_fd;
    struct sockaddr_in addr;
    char username[USERNAME_LENGTH];
    time_t last_activity;    // last activity from client
    time_t last_active_ping; // last time the client was sent an ACTIVE ping
};

#define INACTIVITY_CHECK_INTERVAL 15 // how often should the server check clients' inactivity
#define INACTIVITY_TIME 15           // after how much inactivity time should the server send a ping
#define RESPONSE_TIME 5              // how quickly should the server check whether the client responded
