#include "report.h"
#include <inttypes.h>
#include <string.h>

static void print_separator(FILE *output, size_t max_width) {
    fprintf(output, "+");
    for (size_t i = 0; i < max_width + 2; i++) {
        fprintf(output, "-");
    }

    fprintf(output, "+------------+----------------------+--------------------------------+\n");
}

static size_t get_max_length(size_t current, const char *text){
    size_t length = 0;
    for (size_t i = 0; text[i]!='\0'; i++) {
        if (((unsigned char)text[i] & 0xC0)!= 0x80) {
            length++;
        }
    }

    if (length > current) {
        return length;
    }

    return current;
}

void print_report(FILE *output, const CsvTable *table, char *name){
    fprintf(output, "REPORT Файл\n");
    fprintf(output, "FILE: %s\n", name);
    fprintf(output, "Rows: %zu | Columns: %zu\n\n", table->row_count, table->column_count);

    size_t max_width = strlen("Column");

    for (size_t i = 0; i < table->column_count; i++) {
        max_width = get_max_length(max_width, table->header_names[i]);
    }

    print_separator(output, max_width);

    fprintf(output, "| %-*s | %-10s | %-20s | %-30s |\n", (int)max_width,"Column", "Type", "Statistic", "Value");

    print_separator(output, max_width);

    for (size_t i = 0; i < table->column_count; i++) {
        const char *name = table->header_names[i];
        ColumnType type = table->column_types[i];
        const ColumnStats *stats = &table->column_stats[i];

        if (type == TYPE_STRING) {
            fprintf(output, "| %-s", name);
            for (size_t i = get_max_length(0,name); i < max_width; i++) {
                fprintf(output, " ");
            }
            fprintf(output," | %-10s | %-20s | %30zu |\n", type_to_string(type), "NULLS", stats->nulls);
            fprintf(output, "| %-*s | %-10s | %-20s | %30zu |\n", (int)max_width,"", "", "MIN_LENGTH", stats->min_length);
            fprintf(output, "| %-*s | %-10s | %-20s | %30zu |\n", (int)max_width,"", "", "MAX_LENGTH", stats->max_length);
            fprintf(output, "| %-*s | %-10s | %-20s | %30.6Lf |\n", (int)max_width,"", "", "MEAN_LENGTH", stats->total_length / stats->non_nulls);
            fprintf(output, "| %-*s | %-10s | %-20s | %30zu |\n", (int)max_width,"", "", "EMPTY", stats->empty);
            print_separator(output, max_width);
        } else if (type == TYPE_INT) {
            fprintf(output, "| %-s", name);
            for (size_t i = get_max_length(0,name); i < max_width; i++) {
                fprintf(output, " ");
            }
            fprintf(output," | %-10s | %-20s | %30zu |\n", type_to_string(type), "NULLS", stats->nulls);
            fprintf(output, "| %-*s | %-10s | %-20s | %30" PRId64 " |\n", (int)max_width,"", "", "MIN", stats->int_min);
            fprintf(output, "| %-*s | %-10s | %-20s | %30" PRId64 " |\n", (int)max_width,"", "", "MAX", stats->int_max);
            fprintf(output, "| %-*s | %-10s | %-20s | %30.6Lf |\n", (int)max_width,"", "", "MEAN", stats->int_sum / stats->int_count);
            fprintf(output, "| %-*s | %-10s | %-20s | %30zu |\n", (int)max_width,"", "", "POSITIVE", stats->int_positive);
            fprintf(output, "| %-*s | %-10s | %-20s | %30zu |\n", (int)max_width,"", "", "NEGATIVE", stats->int_negative);
            fprintf(output, "| %-*s | %-10s | %-20s | %30zu |\n", (int)max_width,"", "", "EVEN", stats->int_even);
            print_separator(output, max_width);
        } else if (type == TYPE_DOUBLE) {
            fprintf(output, "| %-s", name);
            for (size_t i = get_max_length(0,name); i < max_width; i++) {
                fprintf(output, " ");
            }
            fprintf(output," | %-10s | %-20s | %30zu |\n", type_to_string(type), "NULLS", stats->nulls);
            fprintf(output, "| %-*s | %-10s | %-20s | %30.6Lf |\n", (int)max_width,"", "", "MIN", stats->double_min);
            fprintf(output, "| %-*s | %-10s | %-20s | %30.6Lf |\n", (int)max_width,"", "", "MAX", stats->double_max);
            fprintf(output, "| %-*s | %-10s | %-20s | %30.6Lf |\n", (int)max_width,"", "", "MEAN", stats->double_sum / stats->double_count);
            fprintf(output, "| %-*s | %-10s | %-20s | %30zu |\n", (int)max_width,"", "", "POSITIVE", stats->double_positive);
            fprintf(output, "| %-*s | %-10s | %-20s | %30zu |\n", (int)max_width,"", "", "NEGATIVE", stats->double_negative);
            fprintf(output, "| %-*s | %-10s | %-20s | %30zu |\n", (int)max_width,"", "", "INTEGER", stats->double_integer);
            print_separator(output, max_width);
        }else {
            fprintf(output, "| %-s", name);
            for (size_t i = get_max_length(0,name); i < max_width; i++) {
                fprintf(output, " ");
            }
            fprintf(output," | %-10s | %-20s | %30zu |\n", type_to_string(type), "ROWS", table->row_count);
            print_separator(output, max_width);
        }
        
    }

}