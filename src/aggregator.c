#include "aggregator.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ---------- Initialization ---------- */

AnalysisResult *init_analyzer(GroupBy group_by) {
    AnalysisResult *result = malloc(sizeof(*result));
    if (!result) return NULL;

    result->total_lines = 0;
    result->info_count  = 0;
    result->warn_count  = 0;
    result->error_total = 0;

    result->error_unique   = 0;
    result->error_capacity = 100;

    result->group_by = group_by;

    result->time_buckets        = NULL;
    result->time_bucket_count   = 0;
    result->time_bucket_capacity = 0;

    result->error_entries =
        calloc(result->error_capacity, sizeof(ErrorEntry));

    if (!result->error_entries) {
        free(result);
        return NULL;
    }

    return result;
}

/* ---------- Time Bucketing ---------- */

static long long bucket_start_unix(long long ts_unix, GroupBy group_by) {
    if (group_by == GROUP_BY_NONE) return ts_unix;

    time_t t = (time_t)ts_unix;
    struct tm tm_value;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    localtime_r(&t, &tm_value);
#else
    struct tm *tmp = localtime(&t);
    if (!tmp) return ts_unix;
    tm_value = *tmp;
#endif

    if (group_by == GROUP_BY_HOUR) {
        tm_value.tm_min = 0;
        tm_value.tm_sec = 0;
    } else if (group_by == GROUP_BY_MINUTE) {
        tm_value.tm_sec = 0;
    }

    time_t bucket = mktime(&tm_value);
    if (bucket == (time_t)-1) return ts_unix;

    return (long long)bucket;
}

/*
 * Adds or updates a time bucket.
 * Linear scan is acceptable for coarse aggregation.
 * Can be optimized with a hashmap if needed.
 */
static void add_time_bucket(AnalysisResult *result, const LogEntry *entry) {
    if (!result || !entry) return;
    if (result->group_by == GROUP_BY_NONE) return;

    long long bucket_start =
        bucket_start_unix(entry->timestamp_unix, result->group_by);

    for (size_t i = 0; i < result->time_bucket_count; i++) {
        TimeBucket *b = &result->time_buckets[i];
        if (b->start_unix == bucket_start) {
            b->total++;
            if (entry->level == LOG_LEVEL_INFO)  b->info++;
            if (entry->level == LOG_LEVEL_WARN)  b->warn++;
            if (entry->level == LOG_LEVEL_ERROR) b->error++;
            return;
        }
    }

    if (result->time_bucket_count >= result->time_bucket_capacity) {
        size_t new_capacity =
            (result->time_bucket_capacity == 0) ? 64
                                                 : result->time_bucket_capacity * 2;

        TimeBucket *new_buckets =
            realloc(result->time_buckets, new_capacity * sizeof(TimeBucket));

        if (!new_buckets) return;  // drop bucket on OOM

        result->time_buckets = new_buckets;
        result->time_bucket_capacity = new_capacity;
    }

    TimeBucket *bucket =
        &result->time_buckets[result->time_bucket_count++];

    bucket->start_unix = bucket_start;
    bucket->total = 1;
    bucket->info  = (entry->level == LOG_LEVEL_INFO);
    bucket->warn  = (entry->level == LOG_LEVEL_WARN);
    bucket->error = (entry->level == LOG_LEVEL_ERROR);
}

/* ---------- Error Aggregation ---------- */

/*
 * NOTE:
 * Linear scan over unique errors.
 * Acceptable for moderate error cardinality.
 * Can be replaced with a hash table if required.
 */
static void add_error_message(AnalysisResult *result, const char *message) {
    for (size_t i = 0; i < result->error_unique; i++) {
        if (strcmp(result->error_entries[i].message, message) == 0) {
            result->error_entries[i].count++;
            return;
        }
    }

    if (result->error_unique >= result->error_capacity) {
        size_t new_capacity = result->error_capacity * 2;
        ErrorEntry *new_entries =
            realloc(result->error_entries, new_capacity * sizeof(ErrorEntry));

        if (!new_entries) return;

        result->error_entries = new_entries;
        result->error_capacity = new_capacity;
    }

    snprintf(
        result->error_entries[result->error_unique].message,
        MAX_MESSAGE_LEN,
        "%s",
        message
    );

    result->error_entries[result->error_unique].count = 1;
    result->error_unique++;
}

/* ---------- Public API ---------- */

void process_log_line(AnalysisResult *result, const LogEntry *entry) {
    if (!result || !entry) return;

    result->total_lines++;

    switch (entry->level) {
        case LOG_LEVEL_INFO:
            result->info_count++;
            break;

        case LOG_LEVEL_WARN:
            result->warn_count++;
            break;

        case LOG_LEVEL_ERROR:
            result->error_total++;
            add_error_message(result, entry->message);
            break;

        default:
            break;
    }

    add_time_bucket(result, entry);
}

/*
 * Returns number of entries written to `out`.
 * Selection sort is used since top_n is small (default <= 10).
 */
size_t get_top_errors(
    const AnalysisResult *result,
    size_t top_n,
    ErrorEntry *out
) {
    if (!result || !out) return 0;

    size_t n = (result->error_unique < top_n)
                   ? result->error_unique
                   : top_n;

    ErrorEntry *temp =
        malloc(result->error_unique * sizeof(ErrorEntry));
    if (!temp) return 0;

    memcpy(
        temp,
        result->error_entries,
        result->error_unique * sizeof(ErrorEntry)
    );

    for (size_t i = 0; i < n; i++) {
        size_t max_idx = i;
        for (size_t j = i + 1; j < result->error_unique; j++) {
            if (temp[j].count > temp[max_idx].count) {
                max_idx = j;
            }
        }
        ErrorEntry swap = temp[i];
        temp[i] = temp[max_idx];
        temp[max_idx] = swap;
    }

    memcpy(out, temp, n * sizeof(ErrorEntry));
    free(temp);

    return n;
}

void cleanup_analyzer(AnalysisResult *result) {
    if (!result) return;

    free(result->error_entries);
    free(result->time_buckets);
    free(result);
}
