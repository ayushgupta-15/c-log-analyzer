#ifndef AGGREGATOR_H
#define AGGREGATOR_H

#include <stddef.h>
#include "parser.h"
#include "options.h"

typedef struct TimeBucket {
    long long start_unix;
    int total;
    int info;
    int warn;
    int error;
} TimeBucket;

typedef struct {
    char message[MAX_MESSAGE_LEN];
    size_t count;
} ErrorEntry;

typedef struct {
    size_t total_lines;

    size_t info_count;
    size_t warn_count;
    size_t error_total;

    ErrorEntry *error_entries;
    size_t error_unique;
    size_t error_capacity;

    GroupBy group_by;

    TimeBucket *time_buckets;
    size_t time_bucket_count;
    size_t time_bucket_capacity;
} AnalysisResult;

/*
 * Allocates and initializes an AnalysisResult.
 * Caller owns the returned pointer and must call cleanup_analyzer().
 */
AnalysisResult *init_analyzer(GroupBy group_by);

/*
 * Processes a single parsed log entry and updates aggregates.
 */
void process_log_line(AnalysisResult *result, const LogEntry *entry);

/*
 * Writes up to top_n most frequent errors into out.
 * Returns the number of entries written.
 */
size_t get_top_errors(
    const AnalysisResult *result,
    size_t top_n,
    ErrorEntry *out
);

/*
 * Frees all resources owned by AnalysisResult.
 */
void cleanup_analyzer(AnalysisResult *result);

#endif
