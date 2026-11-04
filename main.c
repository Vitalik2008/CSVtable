#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "columns.h"
#include "csv_parser.h"
#include "stats.h"


void print_quoted_name(const char *field) {
    printf("\"");

    for (int i = 0; field[i]!='\0'; i++){
        if (field[i]=='"'){
            printf("\"");
        }

        printf("%c", field[i]);
    }
    printf("\"");
}

int main(int argc, char *argv[]) {
    system("chcp 65001 > nul"); 
    if (argc>=2 && strcmp(argv[1],"--help")==0){
        if(argc!=2) {
            fprintf(stderr, "Лишние аргументы после --help\n");
            return 2;
        }

        printf("csvstat --help\n");
        printf("csvstat schema <csv-file> [options]\n");
        printf("csvstat stats <csv-file> [options]\n");
        printf("csvstat report <csv-file> <output-file> [options]\n");

        return 0;
    }
    if (argc < 3){
        fprintf(stderr, "Недостаточно аргументов\n");
        return 2;
    }

    if (strcmp(argv[1],"schema")!=0 && strcmp(argv[1],"stats")!=0) {
        fprintf(stderr, "Неизвестная команда: %s\n", argv[1]);
        return 2;
    }
    FILE *input = fopen(argv[2], "r");

    if (input == NULL) {
        perror("Не удалось открыть файл");
        return 1;
    }

    CsvTable table;


    int status = csv_parser(input, &table);

    fclose(input);

    if (status!=0) {
        return 1;
    }

    if (strcmp(argv[1],"schema")==0) {
        printf("rows %zu\n", table.row_count);
        printf("columns %zu\n", table.column_count);

        for (size_t i =0; i < table.column_count; i++) {
            printf("%zu ", i);
            print_quoted_name(table.header_names[i]);
            printf(" %s\n",type_to_string(table.column_types[i]));
        }
    } else if (strcmp(argv[1], "stats")==0) {
        print_stats(&table);
    }

    



    cleanup(NULL, NULL, table.header_names, table.header_count, table.column_types, table.column_stats);
    return 0;

}