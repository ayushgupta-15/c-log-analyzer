#include <stdio.h>
#include <stdlib.h>

#include "cli.h"
#include "utils.h"
#include "parser.h"
#include "aggregator.h"
#include "report.h"

int main(int argc, char *argv[]) {
    CliOptions options;

    /* Parse CLI */
    CliResult cli_result = parse_cli(argc, argv, &options);
    if (cli_result == CLI_EXIT) return 0;
    if (cli_result == CLI_ERROR) return 1;

    /* Open log file */
    FileReader *reader = file_reader_open(options.filename);
    if (!reader) {
        fprintf(stderr, "Error: Could not open file '%s'\n",
                options.filename);
        return 1;
    }

    /* Initialize analyzer */
    AnalysisResult *result = init_analyzer(options.group_by);
    if (!result) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        file_reader_close(reader);
        return 1;
    }

    printf("Analyzing log file: %s\n", options.filename);
    printf("Press Ctrl+C to abort...\n\n");

    /* Process file line by line */
    char *line;
    LogEntry entry;
    size_t processed_lines = 0;

    while ((line = file_reader_read_line(reader)) != NULL) {
        if (parse_log_line(line, &entry) == 0) {
            process_log_line(result, &entry);
            processed_lines++;

            /* Progress indicator */
            if (processed_lines % 10000 == 0) {
                printf("\rProcessed %zu lines...", processed_lines);
                fflush(stdout);
            }
        }
    }

    printf("\rProcessed %zu lines... Done!\n\n", processed_lines);

    /* Generate report */
    if (options.output_format == OUTPUT_TEXT) {
        print_summary(result, options.errors_only);

        if (!options.errors_only || result->error_total > 0) {
            print_top_errors(result, options.top_n);
        }

        print_time_buckets_text(result);

    } else if (options.output_format == OUTPUT_JSON) {
        print_report_json(result,
                          options.errors_only,
                          options.top_n);

    } else if (options.output_format == OUTPUT_CSV) {
        print_report_csv(result,
                         options.errors_only,
                         options.top_n);
    }

    /* Cleanup */
    cleanup_analyzer(result);
    file_reader_close(reader);

    return 0;
}
