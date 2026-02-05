#include "parser.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

/*
 * Converts timestamp string "YYYY-MM-DD HH:MM:SS" to Unix time.
 * Returns 0 on success, non-zero on failure.
 */
static int parse_timestamp_unix(const char *timestamp, long long *out_unix) {
    if (!timestamp || !out_unix) return -1;

    int year, month, day, hour, minute, second;

    if (sscanf(timestamp,
               "%4d-%2d-%2d %2d:%2d:%2d",
               &year, &month, &day,
               &hour, &minute, &second) != 6) {
        return -1;
    }

    if (year < 1970 ||
        month < 1 || month > 12 ||
        day < 1 || day > 31 ||
        hour < 0 || hour > 23 ||
        minute < 0 || minute > 59 ||
        second < 0 || second > 60) {
        return -1;
    }

    struct tm tm_value;
    memset(&tm_value, 0, sizeof(tm_value));

    tm_value.tm_year = year - 1900;
    tm_value.tm_mon  = month - 1;
    tm_value.tm_mday = day;
    tm_value.tm_hour = hour;
    tm_value.tm_min  = minute;
    tm_value.tm_sec  = second;
    tm_value.tm_isdst = -1;

    time_t t = mktime(&tm_value);
    if (t == (time_t)-1) return -1;

    *out_unix = (long long)t;
    return 0;
}

/*
 * Parses a single log line into LogEntry.
 * Expected format:
 * YYYY-MM-DD HH:MM:SS LEVEL message
 *
 * Returns 0 on success, non-zero on failure.
 */
int parse_log_line(const char *line, LogEntry *entry) {
    if (!line || !entry) return -1;

    memset(entry, 0, sizeof(*entry));

    /* Minimum length: timestamp + space */
    if (strlen(line) < TIMESTAMP_LEN + 1) return -1;

    /* Extract timestamp */
    memcpy(entry->timestamp, line, TIMESTAMP_LEN);
    entry->timestamp[TIMESTAMP_LEN] = '\0';

    if (parse_timestamp_unix(entry->timestamp,
                             &entry->timestamp_unix) != 0) {
        return -1;
    }

    /* Move past timestamp and space */
    const char *p = line + TIMESTAMP_LEN + 1;

    /* Parse log level */
    if (strncmp(p, "INFO ", 5) == 0) {
        entry->level = LOG_LEVEL_INFO;
        p += 5;
    }
    else if (strncmp(p, "WARN ", 5) == 0) {
        entry->level = LOG_LEVEL_WARN;
        p += 5;
    }
    else if (strncmp(p, "ERROR ", 6) == 0) {
        entry->level = LOG_LEVEL_ERROR;
        p += 6;
    }
    else {
        entry->level = LOG_LEVEL_UNKNOWN;
        return -1;
    }

    /* Copy message */
    strncpy(entry->message, p, MAX_MESSAGE_LEN - 1);
    entry->message[MAX_MESSAGE_LEN - 1] = '\0';

    /* Trim trailing newline */
    size_t len = strlen(entry->message);
    if (len > 0 && entry->message[len - 1] == '\n') {
        entry->message[len - 1] = '\0';
    }

    return 0;
}
