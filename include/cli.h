#ifndef CLI_H
#define CLI_H

#include <stddef.h>
#include <stdbool.h>
#include "options.h"

typedef struct {
    const char *filename;
    bool errors_only;
    size_t top_n;
    OutputFormat output_format;
    GroupBy group_by;
} CliOptions;

typedef enum {
    CLI_OK = 0,    // Continue execution
    CLI_EXIT = 1,  // Help/version printed, exit normally
    CLI_ERROR = 2  // Invalid arguments
} CliResult;

/*
 * Parses command-line arguments into CliOptions.
 * Returns CLI_EXIT if the program should terminate early
 * (e.g., --help or --version).
 */
CliResult parse_cli(int argc, char **argv, CliOptions *out);

#endif
