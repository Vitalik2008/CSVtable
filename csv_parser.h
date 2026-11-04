#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <stdio.h>
#include <stddef.h>
#include "columns.h"
#include "stats.h"

typedef struct CsvTable
{
    char **header_names;
    ColumnType *column_types;
    ColumnStats *column_stats;

    size_t header_count;
    size_t column_count;
    size_t row_count;
} CsvTable;

int csv_parser(FILE *input, CsvTable *result);

#endif