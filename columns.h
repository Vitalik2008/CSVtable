#ifndef COLUMNS_H
#define COLUMNS_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
        TYPE_NULL,
        TYPE_INT,
        TYPE_DOUBLE,
        TYPE_STRING
} ColumnType;

typedef struct {
    size_t nulls;
    size_t non_nulls;

    size_t min_length;
    size_t max_length;
    size_t empty;
    long double total_length;

    size_t int_count;
    int64_t int_min;
    int64_t int_max;
    long double int_sum;
    size_t int_positive;
    size_t int_negative;
    size_t int_even;

    size_t double_count;
    long double double_min;
    long double double_max;
    long double double_sum;
    size_t double_positive;
    size_t double_negative;
    size_t double_integer;

} ColumnStats;
ColumnType detect_type(const char *value, int was_quoted);
ColumnType merge_types(ColumnType current_type, ColumnType new_type);
void update_column_type(ColumnType *column_types, int index, const char *value, int was_quoted);
const char *type_to_string(ColumnType type);

void cleanup(FILE *table, char *field, char **header_names, int header_count, ColumnType *column_types, ColumnStats *column_stats);

#endif