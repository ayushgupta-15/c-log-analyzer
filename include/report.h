#ifndef REPORT_H
#define REPORT_H

#include <stddef.h>
#include <stdbool.h>
#include "aggregator.h"

/*
 * Prints a human-readable text summary.
 */
void print_summary(const AnalysisResult *result, bool errors_only);

/*
 * Prints the top N most frequent error messages (text output).
 */
void print_top_errors(const AnalysisResult *result, size_t top_n);

/*
 * Prints the full report in JSON format.
 */
void print_report_json(
    const AnalysisResult *result,
    bool errors_only,
    size_t top_n
);

/*
 * Prints the full report in CSV format.
 */
void print_report_csv(
    const AnalysisResult *result,
    bool errors_only,
    size_t top_n
);

/*
 * Prints time-based aggregation buckets in text format.
 */
void print_time_buckets_text(const AnalysisResult *result);

#endif
