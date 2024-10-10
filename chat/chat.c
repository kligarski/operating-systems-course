#include "chat.h"

void error_exit(const char *msg)
{
    fprintf(stderr, "%s", msg);
    fprintf(stderr, "\n");
    exit(1);
}