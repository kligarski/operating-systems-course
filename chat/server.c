#include "chat.h"

struct client_data clients[MAX_CLIENTS];
pthread_mutex_t clients_mut;

// DOESN'T LOCK THE MUTEX!
int find_client_fd(const char *username)
{
    for (size_t i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i].assigned == ASSIGNED && strncmp(username, (char *)clients[i].username, USERNAME_LENGTH) == 0)
        {
            return clients[i].client_fd;
        }
    }
    return -1;
}

void *alive_thread(void *data)
{
    int clients_pinged;
    time_t current_time;
    while (1)
    {
        clients_pinged = 0;
        current_time = time(NULL);

        pthread_mutex_lock(&clients_mut);

        for (size_t i = 0; i < MAX_CLIENTS; ++i)
        {
            if (clients[i].assigned == UNASSIGNED)
            {
                continue;
            }

            if (clients[i].last_active_ping > clients[i].last_activity && current_time - clients[i].last_active_ping > RESPONSE_TIME)
            {
                printf("Disconnecting %s due to inactivity.\n", clients[i].username);
                clients[i].assigned = UNASSIGNED;
                pthread_cancel(clients[i].client_thread);
                shutdown(clients[i].client_fd, SHUT_RDWR);
                close(clients[i].client_fd);
            }
            else if (current_time - clients[i].last_activity > INACTIVITY_TIME)
            {
                struct chat_msg ping;
                ping.operation = ALIVE;
                ping.time = current_time;
                write(clients[i].client_fd, &ping, sizeof(ping));
                clients[i].last_active_ping = current_time;
                ++clients_pinged;
            }
        }

        pthread_mutex_unlock(&clients_mut);

        if (clients_pinged == 0)
        {
            sleep(INACTIVITY_CHECK_INTERVAL);
        }
        else
        {
            sleep(RESPONSE_TIME + 1);
        }
    }

    return NULL;
}

void *client_thread(void *data)
{
    struct client_data *client_data = (struct client_data *)data;

    pthread_mutex_lock(&clients_mut);
    int assigned = client_data->assigned;
    int client_id = client_data->client_id;
    int client_fd = client_data->client_fd;
    char username[USERNAME_LENGTH];
    strncpy((char *)username, client_data->username, USERNAME_LENGTH);
    pthread_mutex_unlock(&clients_mut);

    while (1)
    {
        struct chat_msg chat_message;
        ssize_t read_bytes = read(client_fd, &chat_message, sizeof(chat_message));
        if (read_bytes == sizeof(chat_message))
        {
            pthread_mutex_lock(&clients_mut);
            clients[client_id].last_activity = time(NULL);
            pthread_mutex_unlock(&clients_mut);

            if (chat_message.operation == INIT)
            {
                int invalid = 0;
                if (strnlen(chat_message.data.init.username, USERNAME_LENGTH) < USERNAME_LENGTH)
                {
                    pthread_mutex_lock(&clients_mut);

                    if (clients[client_id].assigned == ANONYMOUS)
                    {
                        for (size_t i = 0; i < MAX_CLIENTS; ++i)
                        {
                            if (clients[i].assigned == ASSIGNED && strncasecmp(chat_message.data.init.username, clients[i].username, USERNAME_LENGTH) == 0)
                            {
                                fprintf(stderr, "Client tried assigning exitsing username.\n");
                                invalid = 1;
                                break;
                            }
                        }

                        if (!invalid)
                        {
                            strncpy((char *)clients[client_id].username, chat_message.data.init.username, USERNAME_LENGTH);
                            strncpy((char *)username, chat_message.data.init.username, USERNAME_LENGTH);
                            clients[client_id].assigned = ASSIGNED;
                            assigned = ASSIGNED;
                        }
                    }
                    else
                    {
                        fprintf(stderr, "Assigned client sent INIT.\n");
                        invalid = 1;
                    }

                    pthread_mutex_unlock(&clients_mut);
                }
                else
                {
                    fprintf(stderr, "Received invalid client username.\n");
                    invalid = 1;
                }

                if (invalid)
                {
                    struct chat_msg reply;
                    reply.operation = INIT;
                    strncpy((char *)reply.data.init.username, "ERROR", USERNAME_LENGTH);
                    reply.time = time(NULL);
                    if (write(client_fd, &reply, sizeof(reply)) != sizeof(reply) && errno != EINTR)
                    {
                        error_exit("Invalid write.");
                    }
                }
            }
            else if (chat_message.operation == LIST)
            {
                struct chat_msg reply;
                reply.operation = LIST;

                pthread_mutex_lock(&clients_mut);

                for (size_t i = 0; i < MAX_CLIENTS; ++i)
                {
                    size_t username_len = strnlen(clients[i].username, USERNAME_LENGTH);
                    if (clients[i].assigned == ASSIGNED && username_len > 0 && username_len < USERNAME_LENGTH)
                    {
                        strncpy((char *)reply.data.list.usernames[i], clients[i].username, USERNAME_LENGTH);
                    }
                    else
                    {
                        strncpy((char *)reply.data.list.usernames[i], UNASSIGNED_USERNAME, USERNAME_LENGTH);
                    }
                }

                pthread_mutex_unlock(&clients_mut);
                reply.time = time(NULL);
                if (write(client_fd, &reply, sizeof(reply)) != sizeof(reply) && errno != EINTR)
                {
                    error_exit("Invalid write.");
                }
            }
            else if (chat_message.operation == TO_ALL)
            {
                if (assigned == ASSIGNED)
                {
                    struct chat_msg reply;
                    reply.operation = TO_ONE;
                    reply.time = chat_message.time;
                    strncpy((char *)&reply.data.to_one.username, username, USERNAME_LENGTH);
                    strncpy((char *)&reply.data.to_one.msg, chat_message.data.to_all.msg, MAX_MSG_LENGTH);

                    pthread_mutex_lock(&clients_mut);

                    for (size_t i = 0; i < MAX_CLIENTS; ++i)
                    {
                        if (clients[i].assigned == ASSIGNED && clients[i].client_id != client_id)
                        {
                            if (write(clients[i].client_fd, &reply, sizeof(reply)) != sizeof(reply) && errno != EINTR)
                            {
                                error_exit("Invalid write.");
                            }
                        }
                    }

                    pthread_mutex_unlock(&clients_mut);
                }
                else
                {
                    fprintf(stderr, "Unassigned client tried sending a message.\n");
                }
            }
            else if (chat_message.operation == TO_ONE)
            {
                if (assigned == ASSIGNED)
                {
                    char receiver_username[USERNAME_LENGTH];
                    strncpy((char *)receiver_username, chat_message.data.to_one.username, USERNAME_LENGTH);

                    struct chat_msg reply;
                    reply.operation = TO_ONE;
                    reply.time = chat_message.time;
                    strncpy((char *)&reply.data.to_one.username, username, USERNAME_LENGTH);
                    strncpy((char *)&reply.data.to_one.msg, chat_message.data.to_one.msg, MAX_MSG_LENGTH);

                    pthread_mutex_lock(&clients_mut);

                    int receiver_fd = find_client_fd((char *)receiver_username);
                    if (receiver_fd != -1)
                    {
                        if (write(receiver_fd, &reply, sizeof(reply)) != sizeof(reply) && errno != EINTR)
                        {
                            error_exit("Invalid write.");
                        }
                    }

                    pthread_mutex_unlock(&clients_mut);
                }
                else
                {
                    fprintf(stderr, "Unassigned client tried sending a message.\n");
                }
            }
            else if (chat_message.operation == ALIVE)
            {
                pthread_mutex_lock(&clients_mut);
                clients[client_id].last_active_ping = clients[client_id].last_activity;
                pthread_mutex_unlock(&clients_mut);
            }
            else if (chat_message.operation == STOP)
            {
                pthread_mutex_lock(&clients_mut);
                clients[client_id].assigned = UNASSIGNED;
                shutdown(client_fd, SHUT_RDWR);
                close(client_fd);
                pthread_mutex_unlock(&clients_mut);
                return NULL;
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

// DOESN'T LOCK THE MUTEX!
int find_client_id(void)
{
    for (size_t i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i].assigned == UNASSIGNED)
        {
            return i;
        }
    }
    return -1;
}

int server_fd;
pthread_t alive;

void interrupt(int code)
{
    pthread_cancel(alive);

    for (size_t i = 0; i < MAX_CLIENTS; ++i)
    {
        if (clients[i].assigned != UNASSIGNED)
        {
            pthread_cancel(clients[i].client_thread);
            shutdown(clients[i].client_fd, SHUT_RDWR);
            close(clients[i].client_fd);
        }
    }

    shutdown(server_fd, SHUT_RDWR);
    close(server_fd);

    pthread_mutex_destroy(&clients_mut);

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

    if (argc != 3)
    {
        error_exit("Usage: ./server <IPv4_address> <port_number>");
    }

    if (pthread_mutex_init(&clients_mut, NULL) != 0)
    {
        error_exit("Unable to initialize mutex.");
    }

    char *end;
    short port = strtol(argv[2], &end, 10);
    if (*end != '\0')
    {
        error_exit("Incorrect port number");
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    if (inet_pton(AF_INET, argv[1], &server_address.sin_addr) <= 0)
    {
        error_exit("Incorrect IPv4 address");
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        error_exit("Unable to create sockets");
    }

    int optval = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
    {
        error_exit("Unable to set SO_REUSEADDR");
    }

    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1)
    {
        error_exit("Unable to bind address to a socket");
    }

    if (listen(server_fd, MAX_CLIENTS) == -1)
    {
        error_exit("Unable to make the socket a passive socket");
    }

    for (size_t i = 0; i < MAX_CLIENTS; ++i)
    {
        clients[i].assigned = UNASSIGNED;
    }

    if (pthread_create(&alive, NULL, alive_thread, NULL) != 0)
    {
        error_exit("Unable to create inactivity check thread.");
    }

    atexit(end_routine);

    while (1)
    {
        int client_fd, client_id;

        struct sockaddr_in client_address;
        socklen_t addr_size = sizeof(struct sockaddr_in);

        if ((client_fd = accept(server_fd, (struct sockaddr *)&client_address, &addr_size)) != -1)
        {
            pthread_mutex_lock(&clients_mut);

            client_id = find_client_id();

            if (client_id != -1)
            {
                clients[client_id].assigned = ANONYMOUS;
                clients[client_id].addr = client_address;

                time_t current_time = time(NULL);
                clients[client_id].last_active_ping = current_time;
                clients[client_id].last_activity = current_time;
                strncpy(clients[client_id].username, ANONYMOUS_USERNAME, USERNAME_LENGTH);
                clients[client_id].client_id = client_id;
                clients[client_id].client_fd = client_fd;

                if (pthread_create(&clients[client_id].client_thread, NULL, client_thread, (void *)&clients[client_id]) != 0)
                {
                    error_exit("Unable to create sender thread.");
                }
            }
            else
            {
                fprintf(stderr, "Server full!\n");
                close(client_fd);
            }

            pthread_mutex_unlock(&clients_mut);
        }
        else if (client_fd == -1 && errno == EINTR)
        {
            break;
        }
        else
        {
            continue;
        }
    }
    return 0;
}