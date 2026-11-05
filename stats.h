#ifndef STATS_H
#define STATS_H

#include <stdio.h>
#include "columns.h"

typedef struct CsvTable CsvTable;


void print_stats(FILE *output, const CsvTable *table);
void update_basic_stats(ColumnStats *stats, const char *value, int was_quoted);
void update_int_stats(ColumnStats *stats, const char *value);
void update_double_stats(ColumnStats *stats, const char *value);

#endif