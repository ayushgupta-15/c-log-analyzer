#include "report.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __linux__
#define COLOR_INFO  "\033[1;32m"
#define COLOR_WARN  "\033[1;33m"
#define COLOR_ERROR "\033[1;31m"
#define COLOR_RESET "\033[0m"
#else
#define COLOR_INFO  ""
#define COLOR_WARN  ""
#define COLOR_ERROR ""
#define COLOR_RESET ""
#endif

/* ---------- Text Summary ---------- */

void print_summary(const AnalysisResult *result, bool errors_only) {
    if (!result) return;

    if (errors_only) {
        printf(COLOR_ERROR "Error Summary\n" COLOR_RESET);
        printf("------------------\n");
        printf("Total Errors : %zu\n", result->error_total);
    } else {
        printf("Log Summary\n");
        printf("------------------\n");
        printf("Total lines : %zu\n", result->total_lines);
        printf(COLOR_INFO  "INFO  : %zu\n" COLOR_RESET, result->info_count);
        printf(COLOR_WARN  "WARN  : %zu\n" COLOR_RESET, result->warn_count);
        printf(COLOR_ERROR "ERROR : %zu\n" COLOR_RESET, result->error_total);
    }
}

/* ---------- Top Errors (Text) ---------- */

void print_top_errors(const AnalysisResult *result, size_t top_n) {
    if (!result || top_n == 0) return;

    if (result->error_unique == 0) {
        printf("\nNo errors found.\n");
        return;
    }

    size_t n = result->error_unique < top_n
                   ? result->error_unique
                   : top_n;

    ErrorEntry *top_errors = malloc(n * sizeof(ErrorEntry));
    if (!top_errors) return;

    get_top_errors(result, n, top_errors);

    printf("\nTop %zu Errors:\n", n);
    printf("------------------\n");

    for (size_t i = 0; i < n; i++) {
        printf("%zu. %s (%zu occurrences)\n",
               i + 1,
               top_errors[i].message,
               top_errors[i].count);
    }

    free(top_errors);
}

/* ---------- Time Buckets (Text) ---------- */

static void print_time_bucket_label(long long start_unix, GroupBy group_by) {
    time_t t = (time_t)start_unix;
    struct tm tm_value;

#if defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    localtime_r(&t, &tm_value);
#else
    struct tm *tmp = localtime(&t);
    if (!tmp) return;
    tm_value = *tmp;
#endif

    char buf[32];
    if (group_by == GROUP_BY_HOUR) {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:00", &tm_value);
    } else {
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm_value);
    }

    printf("%s", buf);
}

void print_time_buckets_text(const AnalysisResult *result) {
    if (!result) return;
    if (result->group_by == GROUP_BY_NONE) return;
    if (result->time_bucket_count == 0) return;

    printf("\nTime Buckets (%s):\n",
           result->group_by == GROUP_BY_HOUR ? "hour" : "minute");
    printf("-----------------------------------\n");

    for (size_t i = 0; i < result->time_bucket_count; i++) {
        print_time_bucket_label(result->time_buckets[i].start_unix,
                                result->group_by);
        printf(" | total=%d info=%d warn=%d error=%d\n",
               result->time_buckets[i].total,
               result->time_buckets[i].info,
               result->time_buckets[i].warn,
               result->time_buckets[i].error);
    }
}

/* ---------- JSON Helpers ---------- */

static void print_json_escaped(const char *s) {
    for (const char *p = s; p && *p; p++) {
        switch (*p) {
            case '\\': printf("\\\\"); break;
            case '"':  printf("\\\""); break;
            case '\n': printf("\\n");  break;
            case '\r': printf("\\r");  break;
            case '\t': printf("\\t");  break;
            default:   putchar(*p);    break;
        }
    }
}

/* ---------- JSON Report ---------- */

void print_report_json(
    const AnalysisResult *result,
    bool errors_only,
    size_t top_n
) {
    if (!result) return;

    printf("{");

    /* Summary */
    printf("\"summary\":{");
    if (errors_only) {
        printf("\"total_errors\":%zu", result->error_total);
    } else {
        printf("\"total_lines\":%zu,", result->total_lines);
        printf("\"info\":%zu,", result->info_count);
        printf("\"warn\":%zu,", result->warn_count);
        printf("\"error\":%zu", result->error_total);
    }
    printf("}");

    /* Top errors */
    if (!errors_only || result->error_total > 0) {
        size_t n = result->error_unique < top_n
                       ? result->error_unique
                       : top_n;

        printf(",\"top_errors\":[");

        if (n > 0) {
            ErrorEntry *top_errors = malloc(n * sizeof(ErrorEntry));
            if (top_errors) {
                get_top_errors(result, n, top_errors);
                for (size_t i = 0; i < n; i++) {
                    if (i > 0) printf(",");
                    printf("{\"message\":\"");
                    print_json_escaped(top_errors[i].message);
                    printf("\",\"count\":%zu}", top_errors[i].count);
                }
                free(top_errors);
            }
        }
        printf("]");
    }

    /* Time buckets */
    if (result->group_by != GROUP_BY_NONE &&
        result->time_bucket_count > 0) {

        printf(",\"time_buckets\":[");
        for (size_t i = 0; i < result->time_bucket_count; i++) {
            if (i > 0) printf(",");
            printf("{\"start_unix\":%lld,", result->time_buckets[i].start_unix);
            printf("\"total\":%d,\"info\":%d,\"warn\":%d,\"error\":%d}",
                   result->time_buckets[i].total,
                   result->time_buckets[i].info,
                   result->time_buckets[i].warn,
                   result->time_buckets[i].error);
        }
        printf("]");
    }

    printf("}\n");
}

/* ---------- CSV Report ---------- */

void print_report_csv(
    const AnalysisResult *result,
    bool errors_only,
    size_t top_n
) {
    if (!result) return;

    printf("metric,value\n");

    if (errors_only) {
        printf("total_errors,%zu\n", result->error_total);
    } else {
        printf("total_lines,%zu\n", result->total_lines);
        printf("info,%zu\n", result->info_count);
        printf("warn,%zu\n", result->warn_count);
        printf("error,%zu\n", result->error_total);
    }

    /* Top errors */
    if (!errors_only || result->error_total > 0) {
        size_t n = result->error_unique < top_n
                       ? result->error_unique
                       : top_n;

        if (n > 0) {
            ErrorEntry *top_errors = malloc(n * sizeof(ErrorEntry));
            if (!top_errors) return;

            get_top_errors(result, n, top_errors);

            printf("\nerror_message,count\n");
            for (size_t i = 0; i < n; i++) {
                printf("\"");
                print_json_escaped(top_errors[i].message);
                printf("\",%zu\n", top_errors[i].count);
            }

            free(top_errors);
        }
    }

    /* Time buckets */
    if (result->group_by != GROUP_BY_NONE &&
        result->time_bucket_count > 0) {

        printf("\nstart_unix,total,info,warn,error\n");
        for (size_t i = 0; i < result->time_bucket_count; i++) {
            printf("%lld,%d,%d,%d,%d\n",
                   result->time_buckets[i].start_unix,
                   result->time_buckets[i].total,
                   result->time_buckets[i].info,
                   result->time_buckets[i].warn,
                   result->time_buckets[i].error);
        }
    }
}
