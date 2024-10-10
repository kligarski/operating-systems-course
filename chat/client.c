#include "chat.h"

void *send_thread(void *data)
{
    int sock_fd = *(int *)data;

    char operation[5];
    char client_username[USERNAME_LENGTH];
    char message[MAX_MSG_LENGTH];
    char hopefully_newline;

    struct chat_msg chat_message;
    while (1)
    {
        scanf("%s", operation);

        if (strncasecmp(operation, "LIST", 4) == 0)
        {
            chat_message.operation = LIST;
        }
        else if (strncasecmp(operation, "2ALL", 4) == 0)
        {
            scanf(" %[^\n]%c", message, &hopefully_newline);
            chat_message.operation = TO_ALL;
            strncpy((char *)&chat_message.data.to_all.msg, message, MAX_MSG_LENGTH);
        }
        else if (strncasecmp(operation, "2ONE", 4) == 0)
        {
            scanf("%s", client_username);
            scanf(" %[^\n]%c", message, &hopefully_newline);
            chat_message.operation = TO_ONE;
            strncpy((char *)&chat_message.data.to_one.username, client_username, USERNAME_LENGTH);
            strncpy((char *)&chat_message.data.to_one.msg, message, MAX_MSG_LENGTH);
        }
        else
        {
            fprintf(stderr, "Operation must be LIST, 2ALL or 2ONE.\n");
            continue;
        }

        chat_message.time = time(NULL);
        if (write(sock_fd, &chat_message, sizeof(chat_message)) != sizeof(chat_message) && errno != EINTR)
        {
            error_exit("Invalid write.");
        }
    }

    return NULL;
}

void *receive_thread(void *data)
{
    int sock_fd = *(int *)data;

    struct chat_msg chat_message;
    while (1)
    {
        ssize_t read_bytes = read(sock_fd, &chat_message, sizeof(chat_message));
        if (read_bytes == sizeof(chat_message))
        {
            if (chat_message.operation == LIST)
            {
                printf("Connected clients:\n");
                for (size_t i = 0; i < MAX_CLIENTS; ++i)
                {
                    size_t username_len = strnlen(chat_message.data.list.usernames[i], USERNAME_LENGTH);
                    if (username_len > 0 && username_len < USERNAME_LENGTH)
                    {
                        printf("- %s\n", chat_message.data.list.usernames[i]);
                    }
                }
            }
            else if (chat_message.operation == TO_ONE)
            {
                struct tm *time = localtime(&chat_message.time);

                if (strnlen(chat_message.data.to_one.username, USERNAME_LENGTH) < USERNAME_LENGTH && strnlen(chat_message.data.to_one.msg, MAX_MSG_LENGTH) < MAX_MSG_LENGTH)
                {
                    printf("[%s@%d-%02d-%02d %02d:%02d:%02d] %s\n",
                           chat_message.data.to_one.username,
                           time->tm_year + 1900, time->tm_mon, time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec,
                           chat_message.data.to_one.msg);
                }
            }
            else if (chat_message.operation == ALIVE)
            {
                struct chat_msg chat_message;
                chat_message.operation = ALIVE;
                chat_message.time = time(NULL);
                if (write(sock_fd, &chat_message, sizeof(chat_message)) != sizeof(chat_message))
                {
                    error_exit("Invalid write.");
                }
            }
            else if (chat_message.operation == INIT)
            {
                error_exit("Unable to use this username.");
            }
            else
            {
                fprintf(stderr, "Received invalid message.\n");
            }
        }
        else if (read_bytes == 0 || errno == EINTR)
        {
            break;
        }
        else
        {
            error_exit("Problem with reading a message.");
        }
    }

    return NULL;
}

pthread_t receiver;
pthread_t sender;
int socket_fd;

void interrupt(int code)
{
    pthread_cancel(receiver);
    pthread_cancel(sender);

    struct chat_msg stop_message;
    stop_message.operation = STOP;
    write(socket_fd, &stop_message, sizeof(stop_message));
    shutdown(socket_fd, SHUT_RDWR);
    close(socket_fd);
    _exit(0);
}

void end_routine()
{
    interrupt(SIGINT);
}

int main(int argc, char *argv[])
{
    struct sigaction sig_handler_data;
    sigemptyset(&sig_handler_data.sa_mask);
    sig_handler_data.sa_flags = 0;
    sig_handler_data.sa_handler = interrupt;

    if (sigaction(SIGINT, &sig_handler_data, NULL) != 0)
    {
        error_exit("Unable to set signal handler.");
    }

    sig_handler_data.sa_handler = SIG_IGN;
    if (sigaction(SIGPIPE, &sig_handler_data, NULL) != 0)
    {
        error_exit("Unable to set signal handler.");
    }

    if (argc != 4)
    {
        error_exit("Usage: ./client <username> <IPv4_address> <port_number>");
    }

    if (strnlen(argv[1], USERNAME_LENGTH) == USERNAME_LENGTH)
    {
        error_exit("Username is too long.");
    }

    char *end;
    short port = strtol(argv[3], &end, 10);
    if (*end != '\0')
    {
        error_exit("Incorrect port number");
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[2], &server_address.sin_addr) <= 0)
    {
        error_exit("Incorrect IPv4 address");
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        error_exit("Unable to create sockets");
    }

    if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        error_exit("Unable to connect to server");
    }

    atexit(end_routine);

    struct chat_msg init_message;
    init_message.operation = INIT;
    strncpy(init_message.data.init.username, argv[1], USERNAME_LENGTH);
    init_message.time = time(NULL);

    if (write(socket_fd, &init_message, sizeof(init_message)) != sizeof(init_message) && errno != EINTR)
    {
        error_exit("Invalid write.");
    }

    if (pthread_create(&receiver, NULL, receive_thread, (void *)&socket_fd) != 0)
    {
        error_exit("Unable to create receiver thread.");
    }

    if (pthread_create(&sender, NULL, send_thread, (void *)&socket_fd) != 0)
    {
        error_exit("Unable to create sender thread.");
    }

    if (pthread_join(receiver, NULL) != 0)
    {
        error_exit("Unable to join receiver thread.");
    }

    if (pthread_join(sender, NULL) != 0)
    {
        error_exit("Unable to join sender thread.");
    }

    return 0;
}