#include "utils.h"

#include <stdlib.h>
#include <string.h>

/*
 * Opens a file for buffered reading.
 * Returns NULL on failure.
 */
FileReader *file_reader_open(const char *filename) {
    if (!filename) return NULL;

    FileReader *reader = malloc(sizeof(*reader));
    if (!reader) return NULL;

    reader->file = fopen(filename, "r");
    if (!reader->file) {
        free(reader);
        return NULL;
    }

    return reader;
}

/*
 * Reads the next line from the file.
 * Returns a pointer to an internal buffer,
 * or NULL on EOF or error.
 */
char *file_reader_read_line(FileReader *reader) {
    if (!reader || !reader->file) return NULL;

    if (!fgets(reader->buffer, BUFFER_SIZE, reader->file)) {
        return NULL;  // EOF or error
    }

    return reader->buffer;
}

/*
 * Closes the file and frees resources.
 */
void file_reader_close(FileReader *reader) {
    if (!reader) return;

    if (reader->file) {
        fclose(reader->file);
    }

    free(reader);
}
