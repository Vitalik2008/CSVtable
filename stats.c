#include <inttypes.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "columns.h"
#include "stats.h"
#include "csv_parser.h"

static void print_quoted_name(const char *field) {
    printf("\"");

    for (int i = 0; field[i]!='\0'; i++){
        if (field[i]=='"'){
            printf("\"");
        }

        printf("%c", field[i]);
    }
    printf("\"");
}

static int parse_int64(const char *value, int64_t *result) {
    char *end;
    errno=0;

    intmax_t number = strtoimax(value, &end, 10);

    if (errno == ERANGE || *value == '\0' || *end != '\0') {
        return 0;
    }

    if (number < INT64_MIN || number > INT64_MAX) {
        return 0;
    }

    *result = (int64_t)number;
    return 1;
}

static int parse_double(const char *value, long double *result) {
    char *end;
    errno=0;

    long double number = strtold(value, &end);

    if (errno==ERANGE || *value=='\0' || *end != '\0') {
        return 0;
    }

    if (detect_type(value,0) != TYPE_INT && detect_type(value,0) != TYPE_DOUBLE) {
        return 0;
    }

    *result = number;
    return 1;
}

void update_basic_stats(ColumnStats *stats, const char *value, int was_quoted) {
    if (detect_type(value, was_quoted)==TYPE_NULL) {
        stats->nulls++;
        return;
    }

    size_t length  = strlen(value);

    if (stats->non_nulls == 0) {
        stats->min_length=length;
        stats->max_length=length;
    } else {
        if (length < stats->min_length) {
            stats->min_length=length;
        }

        if (length > stats->max_length) {
            stats->max_length=length;
        }
    }

    stats->non_nulls++;
    stats->total_length+=(long double)length;

    if (length==0) {
        stats->empty++;
    }

}
void update_int_stats(ColumnStats *stats, const char *value) {   
    int64_t number;

    if (!parse_int64(value, &number)) {
        return;
    }

    if (stats->int_count == 0) {
        stats->int_min = number;
        stats->int_max = number;
    } else {
        if (number < stats->int_min) {
            stats->int_min = number;
        }

        if (number > stats->int_max) {
            stats->int_max = number;
        }
    }

    stats->int_sum+=number;

    if (number>0){
        stats->int_positive++;
    } else if (number <0) stats->int_negative++;

    if (number%2==0) {
        stats->int_even++;
    }

    stats->int_count++;
}

void update_double_stats(ColumnStats *stats, const char *value) {
    long double number;

    if (!parse_double(value, &number)) {
        return;
    }

    if (stats->double_count==0) {
        stats->double_min=number;
        stats->double_max=number;
    } else {
        if (number<stats->double_min) {
            stats->double_min=number;
        }    

        if (number>stats->double_max) {
            stats->double_max=number;
        }
    }

    stats->double_sum+=number;

    if (number<0) {
        stats->double_negative++;
    } else if (number >0) stats->double_positive++;

    if (number == truncl(number)) {
        stats->double_integer++;
    }

    stats->double_count++;

}

void print_stats(const CsvTable *table) {
    printf("rows %zu\n", table->row_count);
    printf("columns %zu\n", table->column_count);

    for (size_t i = 0; i<table->column_count; i++) {
        ColumnType type = table->column_types[i];
        ColumnStats *stats = &table->column_stats[i];

        printf("column %zu\n", i);
        printf("name ");
        print_quoted_name(table->header_names[i]);
        printf("\ntype %s\n", type_to_string(table->column_types[i]));
        switch (type)
        {
        case TYPE_NULL:
            printf("rows %zu\n\n", table->row_count );
            break;

        case TYPE_INT:
            printf("nulls %zu\n", stats->nulls);
            printf("min %"PRId64"\n", stats->int_min);
            printf("max %"PRId64"\n", stats->int_max);
            printf("mean %.6Lf\n", stats->int_sum / stats->int_count);
            printf("positive %zu\n", stats->int_positive);
            printf("negative %zu\n", stats->int_negative);
            printf("even %zu\n\n", stats->int_even);
            break;
        
        case TYPE_DOUBLE:
            printf("nulls %zu\n", stats->nulls);
            printf("min %.6Lf\n", stats->double_min);
            printf("max %.6Lf\n", stats->double_max);
            printf("mean %.6Lf\n", stats->double_sum/stats->double_count);
            printf("positive %zu\n", stats->double_positive);
            printf("negative %zu\n", stats->double_negative);
            printf("integer %zu\n\n", stats->double_integer);
            break;
        
        case TYPE_STRING:
            printf("nulls %zu\n", stats->nulls);
            printf("min_length %zu\n", stats->min_length);
            printf("max_length %zu\n", stats->max_length);
            printf("mean_length %.6Lf\n", stats->total_length / stats->non_nulls);
            printf("empty %zu\n\n", stats->empty);
            break;
        }
    }
}
