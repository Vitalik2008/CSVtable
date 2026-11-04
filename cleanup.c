#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "columns.h"

void cleanup(FILE *table, char *field, char **header_names, int header_count, ColumnType *column_types, ColumnStats *column_stats) {
    if ( header_names!= NULL) {
        for (int i=0; i <header_count; i++) {
            free(header_names[i]);
        }
    }
    free(header_names);
    free(column_types);
    free(column_stats);
    free(field);
    if (table!=NULL) {
        fclose(table);
    }
}