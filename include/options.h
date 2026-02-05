#ifndef OPTIONS_H
#define OPTIONS_H

// Output format for analysis results
typedef enum {
    OUTPUT_TEXT = 0,
    OUTPUT_JSON = 1,
    OUTPUT_CSV  = 2
} OutputFormat;

// Time-based aggregation mode
typedef enum {
    GROUP_BY_NONE   = 0,
    GROUP_BY_MINUTE = 1,
    GROUP_BY_HOUR   = 2
} GroupBy;

#endif
