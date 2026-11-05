 #include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "csv_parser.h"
#include "columns.h"

typedef enum {
    FIELD_START,
    UNQUOTED,
    QUOTED,
    AFTER_QUOTE
} State;

int csv_parser(FILE *input, CsvTable *result) {

    if (input == NULL || result == NULL) {
        return 1;
    }
    result->header_names = NULL;
    result->column_types = NULL;
    result->column_stats=NULL;
    result->header_count = 0;
    result->column_count = 0;
    result->row_count = 0;

    int cha;
    int pcha=EOF;
    int column_index = 0;
    int fields = 1;

    int length = 0;
    int capacity = 10;

    char *field = malloc(capacity);

    int was_quoted = 0;

    int header_fields = 0;
    int header_progress=0;
    int count_empty_lines = 0;
    int line_has_chars = 0;

    int header_count=0;
    int header_capacity=4;
    char **header_names= malloc(header_capacity * sizeof(char *));

    ColumnType *column_types = NULL;
    ColumnStats *column_stats = NULL;

    State now = FIELD_START;

    if (field == NULL) {
        fprintf(stderr,"Ошибка: не удалось выделить память!\n");
        cleanup(NULL, field, header_names, header_count, column_types, column_stats);
        return 1;
    }
    if (header_names == NULL) {
        fprintf(stderr,"Ошибка: не удалось выделить память!\n");
        cleanup(NULL, field, header_names, header_count, column_types, column_stats);
        return 1;
    }

    field[0]='\0';


    while ((cha=fgetc(input))!= EOF) {
        if (cha!='\n') {
            line_has_chars = 1;
        }
        if (cha == ',' && now!=QUOTED) {
            fields++;
            if (now==UNQUOTED && length>0){
                while(length>0 && (field[length-1]==' ' || field[length-1] == '\t')){
                    length--;
                }
                field[length]='\0';
            }
            if (header_progress==0) {
                if (length==0) {
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    fprintf(stderr,"Ой беда, заголовки не могут быть такими, проверь");
                    return 1;
                }
                for (int i = 0; i < header_count; i++) {
                    if (strcmp(field, header_names[i])==0) {
                        fprintf(stderr,"Беда-беда, заголовки не те, проверь");
                        cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                        return 1;
                    }
                }
                if (header_count > header_capacity - 1) {
                    header_capacity*=2;
                    char **temp = realloc(header_names, header_capacity*sizeof(char *));
                    if (temp==NULL) {
                        fprintf(stderr,"Ошибка в расширении памяти");
                        cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                        return 1;
                    }
                    header_names = temp;
                }
                char *name = malloc(length+1);
                if (name==NULL) {
                    fprintf(stderr,"Ошибка в расширении памяти");
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    return 1;
                }
                strcpy(name,field);
                header_names[header_count] = name;
                header_count++;
            } else {
                if (column_index>=header_fields) {
                    fprintf(stderr,"Ошибочка, иди проверяй");
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    return 1;
                }
                update_column_type(column_types, column_index, field, was_quoted);
                update_basic_stats(&column_stats[column_index], field, was_quoted);
                update_int_stats(&column_stats[column_index], field);
                update_double_stats(&column_stats[column_index], field);
                column_index++;
            }
            length=0;
            field[length] = '\0';
            now=FIELD_START;
            was_quoted=0;
        }
        else if ((now==FIELD_START || now==AFTER_QUOTE) && (cha==' ' || cha=='\t')) {
            pcha=cha;
            continue;
        }
        else if (cha=='"') {
            if (now==QUOTED) {
                now =AFTER_QUOTE;
            }
            else if (now ==UNQUOTED) {
                fprintf(stderr,"Ошибочка, проверь файл");
                cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                return 1;
            }
            else if (now==AFTER_QUOTE) {
                if (length==capacity-1) {
                    capacity*=2;
                    char *temp = realloc(field, capacity);
                    if (temp==NULL) {
                        fprintf(stderr,"Ошибка в расширении памяти");
                        cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                        return 1;
                    }
                    field = temp;
                }
                field[length]=cha;
                length++;
                field[length] ='\0';
                now=QUOTED;
            }
            else if (now == FIELD_START) {
                now = QUOTED;
                was_quoted=1;
            }
        }
        else if (cha=='\n') {
            if (now==UNQUOTED && length>0){
                while(length>0 && (field[length-1]==' ' || field[length-1] == '\t')){
                    length--;
                }
                field[length]='\0';
            }
            if (now==QUOTED) {
                fprintf(stderr,"Ошибочка, проверь файл");
                cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                return 1;
            }
            if (line_has_chars ==0) {
                if (header_progress==0) {
                    fprintf(stderr, "Пустая строка до заголовка");
                    cleanup(NULL, field, header_names, header_count,column_types, column_stats);
                    return 1;
                }

                count_empty_lines++;

                length = 0;
                field[0]='\0';
                fields=1;
                column_index=0;
                now=FIELD_START;
                was_quoted=0;
                line_has_chars=0;

                continue;
            }
            if (count_empty_lines > 0) {
                if (header_fields!=1) {
                    fprintf(stderr, "Ошибка в файле");
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    return 1;
                }
                while (count_empty_lines > 0) {
                    for (int i = 0;  i <header_fields; i++) {
                        column_stats[i].nulls++;
                    }

                    result->row_count++;
                    count_empty_lines--;
                }
            }

            if (header_progress == 0) {
                if (length==0) {
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    fprintf(stderr,"Ой беда, заголовки не могут быть такими, проверь");
                    return 1;
                }
                for (int i = 0; i < header_count; i++) {
                    if (strcmp(field, header_names[i])==0) {
                        fprintf(stderr,"Беда-беда, заголовки не те, проверь");
                        cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                        return 1;
                    }
                }
                if (header_count > header_capacity - 1) {
                    header_capacity*=2;
                    char **temp = realloc(header_names, header_capacity*sizeof(char *));
                    if (temp==NULL) {
                        fprintf(stderr,"Ошибка в расширении памяти");
                        cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                        return 1;
                    }
                    header_names = temp;
                }
                char *name = malloc(length+1);
                if (name==NULL) {
                    fprintf(stderr,"Ошибка в расширении памяти");
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    return 1;
                }
                strcpy(name,field);
                header_names[header_count] = name;
                header_count++;

                header_fields = fields;

                column_types = malloc(header_fields * sizeof(*column_types));
                if (column_types==NULL) {
                    fprintf(stderr,"Ошибка в расширении памяти");
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    return 1;
                }
                for (int i =0; i<header_fields; i++) {
                    column_types[i]=TYPE_NULL;
                }

                column_stats = calloc(header_fields, sizeof(*column_stats));

                if (column_stats == NULL) {
                    fprintf(stderr, "Ошибка в выделении памяти");
                    cleanup(NULL, field, header_names, header_count,
                            column_types, column_stats);
                    return 1;
                }

                header_progress=1;
            } else {
                if (header_fields != fields) {
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    fprintf(stderr,"Ой, ошибочка, проверь файл");
                    return 1;
                }
                update_column_type(column_types, column_index, field, was_quoted);
                update_basic_stats(&column_stats[column_index], field, was_quoted);
                update_int_stats(&column_stats[column_index], field);
                update_double_stats(&column_stats[column_index], field);
                column_index++;
                result->row_count++;
                
            }
            length=0;
            field[length]='\0';
            fields=1;
            column_index=0;
            now=FIELD_START;
            was_quoted=0;
            line_has_chars=0;
        }
        else {
            if (now==AFTER_QUOTE) {
                fprintf(stderr,"Ошибочка, проверь файл");
                cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                return 1;
            }
            if (length==capacity-1) {
                capacity*=2;
                char *temp = realloc(field, capacity);
                if (temp==NULL) {
                    fprintf(stderr,"Ошибка в расширении памяти");
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    return 1;
                }
                field = temp;
            }
            field[length]=cha;
            length++;
            field[length] ='\0';
            if (now==FIELD_START){
                now=UNQUOTED;
            }
            
        }
        pcha=cha;
        
    }
    if (pcha!='\n') {
        if (count_empty_lines > 0) {
            if (header_fields!=1) {
                fprintf(stderr, "Ошибка в файле");
                cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                return 1;
            }
            while (count_empty_lines > 0) {
                for (int i = 0;  i <header_fields; i++) {
                    column_stats[i].nulls++;
                }

                result->row_count++;
                count_empty_lines--;
            }
        }
        if (now==UNQUOTED && length>0){
            while(length>0 && (field[length-1]==' ' || field[length-1] == '\t')){
                length--;
            }
            field[length]='\0';
        }
        if (now==QUOTED) {
            fprintf(stderr,"Ошибочка, проверь файл");
            cleanup(NULL, field, header_names, header_count, column_types, column_stats);
            return 1;
        }
        if (header_progress == 0) {
            if (length==0) {
                cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                fprintf(stderr,"Ой беда, заголовки не могут быть такими, проверь");
                return 1;
            }
            for (int i = 0; i < header_count; i++) {
                if (strcmp(field, header_names[i])==0) {
                    fprintf(stderr,"Беда-беда, заголовки не те, проверь");
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    return 1;
                }
            }
            if (header_count > header_capacity - 1) {
                header_capacity*=2;
                char **temp = realloc(header_names, header_capacity*sizeof(char *));
                if (temp==NULL) {
                    fprintf(stderr,"Ошибка в расширении памяти");
                    cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                    return 1;
                }
                header_names = temp;
            }
            char *name = malloc(length+1);
            if (name==NULL) {
                fprintf(stderr,"Ошибка в расширении памяти");
                cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                return 1;
            }
            strcpy(name,field);
            header_names[header_count] = name;
            header_count++;

            header_fields = fields;

            column_types = malloc(header_fields * sizeof(*column_types));
            if (column_types==NULL) {
                fprintf(stderr,"Ошибка в расширении памяти");
                cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                return 1;
            }
            for (int i =0; i<header_fields; i++) {
                column_types[i]=TYPE_NULL;
            }
            column_stats = calloc(header_fields, sizeof(*column_stats));

            if (column_stats == NULL) {
                fprintf(stderr, "Ошибка в выделении памяти");
                cleanup(NULL, field, header_names, header_count,
                        column_types, column_stats);
                return 1;
            }
            header_progress=1;
        } else {
            if (header_fields != fields) {
                cleanup(NULL, field, header_names, header_count, column_types, column_stats);
                fprintf(stderr,"Ой, ошибочка, проверь файл");
                return 1;
            }
            update_column_type(column_types, column_index, field, was_quoted);
            update_basic_stats(&column_stats[column_index], field, was_quoted);
            update_int_stats(&column_stats[column_index], field);
            update_double_stats(&column_stats[column_index], field);
            column_index++;
            result->row_count++;
        }
        
        was_quoted=0;
    }

    result->column_types=column_types;
    result->column_stats=column_stats;
    result->header_names=header_names;
    result->header_count=header_count;
    result->column_count=header_fields;

    free(field);
    return 0;

}