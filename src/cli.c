#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define VERSION "1.0.0"
#define DEFAULT_TOP_N 10

/* ---------- Helpers ---------- */

static void print_usage(const char *program_name) {
    printf("Usage: %s <log_file> [options]\n", program_name);
    printf("\nOptions:\n");
    printf("  --errors-only             Show only error-related statistics\n");
    printf("  --top-errors N            Show top N most frequent errors (default: %d)\n",
           DEFAULT_TOP_N);
    printf("  --output text|json|csv    Output format (default: text)\n");
    printf("  --group-by minute|hour    Aggregate counts by time bucket\n");
    printf("  --help                    Show this help message\n");
    printf("  --version                 Show version information\n");

    printf("\nExamples:\n");
    printf("  %s server.log\n", program_name);
    printf("  %s server.log --errors-only --top-errors 5\n", program_name);
    printf("  %s server.log --group-by hour --output json\n", program_name);
}

/*
 * Parses a positive integer argument safely.
 * Returns 1 on success, 0 on failure.
 */
static int parse_positive_size(const char *arg, size_t *out) {
    char *end = NULL;
    errno = 0;

    long val = strtol(arg, &end, 10);

    if (errno != 0 || end == arg || *end != '\0' || val <= 0) {
        return 0;
    }

    *out = (size_t)val;
    return 1;
}

/* ---------- Public API ---------- */

CliResult parse_cli(int argc, char **argv, CliOptions *out) {
    if (!out) return CLI_ERROR;

    /* Defaults */
    out->filename      = NULL;
    out->errors_only   = false;
    out->top_n         = DEFAULT_TOP_N;
    out->output_format = OUTPUT_TEXT;
    out->group_by      = GROUP_BY_NONE;

    if (argc < 2) {
        print_usage(argv[0]);
        return CLI_ERROR;
    }

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return CLI_EXIT;
        }

        else if (strcmp(argv[i], "--version") == 0) {
            printf("loganalyzer version %s\n", VERSION);
            return CLI_EXIT;
        }

        else if (strcmp(argv[i], "--errors-only") == 0) {
            out->errors_only = true;
        }

        else if (strcmp(argv[i], "--top-errors") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing value for --top-errors\n");
                return CLI_ERROR;
            }

            size_t value;
            if (!parse_positive_size(argv[++i], &value)) {
                fprintf(stderr,
                        "Error: Invalid value for --top-errors: '%s'\n",
                        argv[i]);
                return CLI_ERROR;
            }

            out->top_n = value;
        }

        else if (strcmp(argv[i], "--output") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing value for --output\n");
                return CLI_ERROR;
            }

            const char *fmt = argv[++i];

            if (strcmp(fmt, "text") == 0) {
                out->output_format = OUTPUT_TEXT;
            } else if (strcmp(fmt, "json") == 0) {
                out->output_format = OUTPUT_JSON;
            } else if (strcmp(fmt, "csv") == 0) {
                out->output_format = OUTPUT_CSV;
            } else {
                fprintf(stderr,
                        "Error: Unsupported output format '%s'\n", fmt);
                return CLI_ERROR;
            }
        }

        else if (strcmp(argv[i], "--group-by") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Missing value for --group-by\n");
                return CLI_ERROR;
            }

            const char *grp = argv[++i];

            if (strcmp(grp, "minute") == 0) {
                out->group_by = GROUP_BY_MINUTE;
            } else if (strcmp(grp, "hour") == 0) {
                out->group_by = GROUP_BY_HOUR;
            } else {
                fprintf(stderr,
                        "Error: Unsupported group-by value '%s'\n", grp);
                return CLI_ERROR;
            }
        }

        else if (argv[i][0] == '-') {
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            return CLI_ERROR;
        }

        else {
            /* Positional argument: log file */
            if (out->filename != NULL) {
                fprintf(stderr,
                        "Error: Multiple log files specified ('%s', '%s')\n",
                        out->filename, argv[i]);
                return CLI_ERROR;
            }
            out->filename = argv[i];
        }
    }

    if (!out->filename) {
        fprintf(stderr, "Error: No log file specified\n");
        print_usage(argv[0]);
        return CLI_ERROR;
    }

    return CLI_OK;
}
