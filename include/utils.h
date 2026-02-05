#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stddef.h>

#define BUFFER_SIZE 4096

typedef struct {
    FILE *file;
    char buffer[BUFFER_SIZE];
} FileReader;

/*
 * Opens a file for buffered reading.
 * Returns NULL on failure.
 */
FileReader *file_reader_open(const char *filename);

/*
 * Reads the next line from the file.
 * Returns a pointer to an internal buffer, or NULL on EOF or error.
 * The returned pointer is invalidated by the next call.
 */
char *file_reader_read_line(FileReader *reader);

/*
 * Closes the file and frees associated resources.
 */
void file_reader_close(FileReader *reader);

#endif
