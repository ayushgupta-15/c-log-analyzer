# C Log Analyzer

A modular, high-performance C CLI tool for parsing and analyzing structured log files.
Designed with streaming I/O, clean architecture, and extensibility in mind.

## Features

Parses logs in the format:
`YYYY-MM-DD HH:MM:SS LEVEL message`

Streaming, line-by-line processing (handles large log files efficiently)

Counts total log lines and occurrences of:

`INFO`

`WARN`

`ERROR`

Tracks unique error messages and their frequencies

Displays top-N most frequent errors

Optional error-only analysis mode

Time-based aggregation:

Group by minute or hour

Multiple output formats:

Human-readable text

JSON (machine-readable)

CSV (spreadsheet-friendly)

ANSI colorized output (Linux terminals)

## Build
```bash
make
```


This produces the loganalyzer binary.

Debug build (with sanitizers)
```bash
make debug
```

## Usage
```bash
./loganalyzer <log_file> [options]
```

## Options

- `--errors-only`
Show only error-related statistics

- `--top-errors N`
Show top N most frequent error messages (default: 10)

- `--output text|json|csv`
Output format (default: text)

- `--group-by minute|hour`
Aggregate counts by time bucket

- `--help`
Show help message

- `--version`
Show version information

## Examples
```bash
./loganalyzer server.log

./loganalyzer server.log --errors-only --top-errors 5

./loganalyzer server.log --group-by hour --output json
```

## Log Format

Each log entry must follow this structure:

`YYYY-MM-DD HH:MM:SS LEVEL message`


Example:

```
2025-01-01 12:30:35 INFO Service started
2025-01-01 12:31:10 WARN Disk space low
2025-01-01 12:31:12 ERROR Database connection failed
```

## Project Layout
```
src/        Implementation files
include/    Header files
obj/        Compiled object files
logs/       Sample logs (optional)
Makefile    Build rules
```

## Architecture Overview

The project follows a modular design with clear separation of concerns:

CLI – argument parsing and validation

Utils – buffered file reading abstraction

Parser – converts raw log lines into structured entries

Aggregator – maintains counters, error frequencies, and time buckets

Report – renders results in text, JSON, or CSV

This structure makes the tool easy to extend with new analytics or formats.

## Notes & Design Decisions

Lines shorter than the timestamp length or with unknown log levels are skipped

Error message uniqueness is tracked via exact string matching

Error and time-bucket aggregation currently use linear scans
(documented tradeoff; can be optimized with hash tables if needed)

Designed to use constant memory growth relative to file size

## Tools & Technologies

Language: C (C99)

Build system: Make

Tooling:

gcc

valgrind

AddressSanitizer (debug builds)

## Future Improvements (Planned)

Hash table–based error aggregation

Multi-threaded parsing (producer–consumer model)

Compressed log support (.gz)

Real-time log monitoring (tail -f style)
