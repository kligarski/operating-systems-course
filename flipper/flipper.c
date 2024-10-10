#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_BUFF_SIZE 256
#define MAX_PATH_LENGTH 4096

int is_text_file(char *filename) {
    int length = strlen(filename);

    if (length < 4) {
        return 0;
    }

    char extension[5];
    strncpy(extension, filename + length - 4, 5);

    if (strcmp(extension, ".txt") == 0) {
        return 1;
    } else {
        return 0;
    }
}

void reverse_str(char* str, int len) {
    if (len <= 1) {
        return;
    }

    for (int i = 0; i < len / 2; ++i) {
        char tmp = str[i];
        str[i] = str[len - 1 - i];
        str[len - 1 - i] = tmp;
    }
}

int main(int argc, char* argv[]) {
    struct dirent* dir_file;
    char buffer[MAX_BUFF_SIZE];

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_dir> <destination_dir>\n", argv[0]);
        return 1;
    }

    DIR *source = opendir(argv[1]);
    if (!source) {
        fprintf(stderr, "Cannot open source directory.\n");
        return 1;
    }

    int dir_file_desc = mkdir(argv[2], S_IRWXU | S_IRGRP | S_IROTH);
    if (dir_file_desc == -1 && errno != EEXIST) {
        fprintf(stderr, "Cannot create destination directory.\n");
        return 1;
    }
    
    DIR *dest = opendir(argv[2]);
    if (!dest) {
        fprintf(stderr, "Cannot open destination directory.\n");
        return 1;
    }

    while (1) {
        errno = 0;
        dir_file = readdir(source);

        if (dir_file == NULL) {
            break;
        }

        if (is_text_file(dir_file->d_name)) {
            char source_file_path[MAX_PATH_LENGTH];
            char result_file_path[MAX_PATH_LENGTH];

            strcpy(source_file_path, argv[1]);
            strcpy(result_file_path, argv[2]);

            strcat(source_file_path, "/");
            strcat(result_file_path, "/");

            strcat(source_file_path, dir_file->d_name);
            strcat(result_file_path, dir_file->d_name);

            FILE *source_file = fopen(source_file_path, "r");
            if (source_file == NULL) {
                fprintf(stderr, "Cannot open one of the files.\n");
                return 1;
            }
            
            FILE *result_file = fopen(result_file_path, "w");
            if (result_file == NULL) {
                fprintf(stderr, "Cannot create one of the files.\n");
                return 1;
            }

            while (fgets(buffer, MAX_BUFF_SIZE, source_file)) {
                int length = strlen(buffer);
                reverse_str(buffer, length - 1);
                fputs(buffer, result_file);
            }

            if (fclose(result_file) == -1) {
                fprintf(stderr, "Problem with closing one of the files.\n");
            }

            if (fclose(source_file) == -1) {
                fprintf(stderr, "Problem with closing one of the files.\n");
            }
        }
    }

    if (errno != 0) {
        fprintf(stderr, "Problem with reading directory.\n");
        return 1;
    }

    int problem_with_closing = 0;
    if (closedir(dest) == -1) {
        fprintf(stderr, "Problem with closing destination directory.\n");
        problem_with_closing = 1;
    }

    if (closedir(source) == -1) {
        fprintf(stderr, "Problem with closing source directory.\n");
        problem_with_closing = 1;
    }

    return problem_with_closing;
}