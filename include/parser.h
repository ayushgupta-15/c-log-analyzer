#ifndef PARSER_H
#define PARSER_H

#define MAX_MESSAGE_LEN 1024
#define TIMESTAMP_LEN   19  // "YYYY-MM-DD HH:MM:SS"

typedef enum {
    LOG_LEVEL_UNKNOWN = -1,
    LOG_LEVEL_INFO    = 0,
    LOG_LEVEL_WARN    = 1,
    LOG_LEVEL_ERROR   = 2
} LogLevel;

typedef struct {
    char timestamp[TIMESTAMP_LEN + 1];  // null-terminated
    LogLevel level;
    char message[MAX_MESSAGE_LEN];
    long long timestamp_unix;
} LogEntry;

/*
 * Parses a single log line into LogEntry.
 * Expected format:
 * YYYY-MM-DD HH:MM:SS LEVEL message
 *
 * Returns 0 on success, non-zero on failure.
 */
int parse_log_line(const char *line, LogEntry *entry);

#endif
