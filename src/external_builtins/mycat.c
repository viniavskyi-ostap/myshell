//
// Created by viniavskyi on 22.10.19.
//

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

void print_help() {
    printf("Usage: ./mycat [-h|--help] [-A] <file1> <file2> ... <fileN>\n"
           "-h, --help\t display this help and exit\n"
           "-A\t print hex-codes of invisible characters\n"
           "Running with no command line arguments return successfully immediately.\n"
           "If at least one file path is invalid the program finishes with error immediately.\n"
           "Examples:\n"
           "./mycat --help\n"
           "./mycat -A file1.txt file2.txt\n"
           "Created by Ostap Viniavskyi and Andrii Prysiazhnyk\n");
}

ssize_t readbuffer(int fd, char *buffer, ssize_t buffsize) {
    ssize_t read_bytes = 0;
    while (read_bytes < buffsize) {
        ssize_t current_read = read(fd, buffer + read_bytes, buffsize - read_bytes);
        if (current_read == -1) {
            if (errno == EINTR)continue;
            else return -1;
        } else if (current_read == 0) {
            break;
        } else {
            read_bytes += current_read;
        }
    }
    return read_bytes;
}

int writebuffer(int fd, const char *buffer, ssize_t size) {
    ssize_t written_bytes = 0;
    while (written_bytes < size) {
        ssize_t current_written = write(fd, buffer + written_bytes, size - written_bytes);
        if (current_written == -1) {
            if (errno == EINTR)continue;
            else return -1;
        } else {
            written_bytes += current_written;
        }
    }
    return 0;
}

int is_replaceable(char c) {
    return !(isgraph(c) || isspace(c));
}

ssize_t process_buffer(char *buffer, ssize_t size) {
    static char *hex_letters = "0123456789ABCDEF";
    int special_char_numbers = 0;
    for (size_t i = 0; i < size; ++i)
        if (is_replaceable(buffer[i])) ++special_char_numbers;
    for (ssize_t i = size - 1; i >= 0; --i) {
        if (is_replaceable(buffer[i])) {
            special_char_numbers--;
            buffer[3 * special_char_numbers + i + 3] = hex_letters[(unsigned char) buffer[i] % 16];
            buffer[3 * special_char_numbers + i + 2] = hex_letters[(unsigned char) buffer[i] / 16];
            buffer[3 * special_char_numbers + i + 1] = 'x';
            buffer[3 * special_char_numbers + i] = '\\';
            size += 3;
        } else {
            buffer[3 * special_char_numbers + i] = buffer[i];
        }
    }
    return size;
}

int pipe_file(int read_fd, int write_fd, char *buffer, ssize_t buffsize, int process) {
    ssize_t current_read;
    while ((current_read = readbuffer(read_fd, buffer, buffsize))) {
        if (current_read == -1) return -1;

        if (process)
            current_read = process_buffer(buffer, current_read);

        if (writebuffer(write_fd, buffer, current_read) == -1) return -2;
    }
    return 0;
}

void close_files(int *fds, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        close(fds[i]);
    }
}

void free_resources(char **filenames, int *fds, char *buffer) {
    if (filenames) free(filenames);
    if (fds) free(fds);
    if (buffer) free(buffer);
}

int main(int argc, char **argv) {

    if (argc == 1) {
        return 0;
    }

    const size_t BUFFSIZE = 1024 * 1024;
    int process = 0, files_number = 0, buff_size_multiplier = 1;
    char **filenames = malloc(sizeof(char *) * (argc - 1));
    if (!filenames) {
        fprintf(stderr, "Cannot allocate memory for filenames\n");
        return -1;
    }

//    parse parameters
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_help();
            free_resources(filenames, NULL, NULL);
            return 0;
        } else if (!strcmp(argv[i], "-A")) {
            process = 1;
            buff_size_multiplier = 4;
        } else {
            filenames[files_number] = argv[i];
            files_number++;
        }
    }

    int *fds = malloc(sizeof(int) * files_number);
    if (!fds) {
        fprintf(stderr, "Cannot allocate memory for file descriptors\n");
        free_resources(filenames, NULL, NULL);
        return -1;
    }

    //    open files and retrieve files descriptors
    int fd;
    for (size_t i = 0; i < files_number; ++i) {
        fd = open(filenames[i], O_RDONLY);

        if (fd < 0) {
            fprintf(stderr, "Cannot open file %s\n", filenames[i]);
            close_files(fds, i);
            free_resources(filenames, fds, NULL);
            return -1;
        }
        fds[i] = fd;
    }

    char *buffer = malloc(buff_size_multiplier * BUFFSIZE);
    if (!buffer) {
        fprintf(stderr, "Cannot allocate memory for buffer\n");
        close_files(fds, files_number);
        free_resources(filenames, fds, NULL);
        return -1;
    }
    int status;
    for (int i = 0; i < files_number; ++i) {
        status = pipe_file(fds[i], STDOUT_FILENO, buffer, BUFFSIZE, process);
        if (status){
            close_files(fds, files_number);
            if (status == -1) {
                fprintf(stderr, "Error while reading file %s\n", filenames[i]);
                free_resources(filenames, fds, buffer);
                return -1;
            } else if (status == -2) {
                fprintf(stderr, "Error while writing content of file %s\n", filenames[i]);
                free_resources(filenames, fds, buffer);
                return -2;
            }
        }
    }

    close_files(fds, files_number);
    free_resources(filenames, fds, buffer);
    return 0;
}