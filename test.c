#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_SIZE 4096
#define MAX_NAME_LEN 128
#define INIT_CAPACITY 2

// 1. Перечисление типов данных
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING
} DataType;

// 2. Обновленная структура столбца, которая хранит и данные, и свой тип
typedef struct {
    char name[MAX_NAME_LEN]; 
    char **data;             // Сами данные храним в виде строк
    int size;                
    int capacity;            
    DataType type;           // 👈 Сюда запишем автоматически определенный тип
} Column;

typedef struct {
    Column *columns;
    int cols_count;
    int rows_count;
} Table;

// Функция автоматического определения типа ячейки
DataType detect_type(const char *str) {
    if (str == NULL || strlen(str) == 0) return TYPE_STRING;
    int i = 0;
    if (str[i] == '-') {
        i = 1;
        if (str[i] == '\0') return TYPE_STRING;
    }
    int has_dot = 0;
    for (; str[i] != '\0'; i++) {
        if (str[i] == '.') {
            if (has_dot) return TYPE_STRING;
            has_dot = 1;
        } else if (!isdigit((unsigned char)str[i])) {
            return TYPE_STRING;
        }
    }
    return has_dot ? TYPE_FLOAT : TYPE_INT;
}

void add_cell_to_column(Column *col, const char *text) {
    if (col->size >= col->capacity) {
        col->capacity *= 2;
        col->data = (char **)realloc(col->data, col->capacity * sizeof(char *));
        if (col->data == NULL) {
            printf("Ошибка выделения памяти через realloc!\n");
            exit(1);
        }
    }
    col->data[col->size] = (char *)malloc((strlen(text) + 1) * sizeof(char));
    strcpy(col->data[col->size], text);
    col->size++;
}

Table* load_full_table(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Ошибка: Не удалось открыть файл %s\n", filename);
        return NULL;
    }

    char buffer[MAX_LINE_SIZE];
    
    if (fgets(buffer, sizeof(buffer), file) == NULL) {
        fclose(file);
        return NULL;
    }
    buffer[strcspn(buffer, "\n")] = '\0';

    Table *table = (Table *)malloc(sizeof(Table));
    table->cols_count = 0;
    table->rows_count = 0;

    char temp_line[MAX_LINE_SIZE];
    strcpy(temp_line, buffer);
    char *token = strtok(temp_line, ",");
    while (token != NULL) {
        table->cols_count++;
        token = strtok(NULL, ",");
    }

    table->columns = (Column *)malloc(table->cols_count * sizeof(Column));

    strcpy(temp_line, buffer);
    token = strtok(temp_line, ",");
    for (int i = 0; i < table->cols_count; i++) {
        strncpy(table->columns[i].name, token, MAX_NAME_LEN - 1);
        table->columns[i].name[MAX_NAME_LEN - 1] = '\0';
        table->columns[i].size = 0;
        table->columns[i].capacity = INIT_CAPACITY;
        table->columns[i].data = (char **)malloc(INIT_CAPACITY * sizeof(char *));
        table->columns[i].type = TYPE_INT; // Изначально предполагаем INT, потом понизим если надо
        token = strtok(NULL, ",");
    }

    // Читаем все строки
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        strcpy(temp_line, buffer);
        token = strtok(temp_line, ",");
        
        for (int i = 0; i < table->cols_count; i++) {
            const char *val = (token == NULL) ? "" : token;
            add_cell_to_column(&table->columns[i], val);
            
            // 👈 АВТООПРЕДЕЛЕНИЕ: Проверяем тип текущей ячейки
            DataType cell_type = detect_type(val);
            
            // Логика «понижения» типа: если в столбце для целых чисел 
            // встретилась хоть одна дробь — весь столбец становится FLOAT. 
            // А если встретился текст — весь столбец становится STRING.
            if (cell_type == TYPE_STRING) {
                table->columns[i].type = TYPE_STRING;
            } else if (cell_type == TYPE_FLOAT && table->columns[i].type != TYPE_STRING) {
                table->columns[i].type = TYPE_FLOAT;
            }

            if (token != NULL) token = strtok(NULL, ",");
        }
        table->rows_count++;
    }

    fclose(file);
    return table;
}

// Модуль аналитики (Уровень 4): Считает статистику по столбцу
void print_column_stats(Column *col) {
    printf("Статистика для столбца '%s' (Тип: ", col->name);
    
    if (col->type == TYPE_STRING) {
        printf("Текст)\n");
        printf("  Всего записей: %d\n", col->size);
        printf("  (Математический анализ для текстовых данных не применим)\n\n");
        return;
    }

    if (col->type == TYPE_INT) printf("Целые числа)\n");
    if (col->type == TYPE_FLOAT) printf("Дробные числа)\n");

    if (col->size == 0) {
        printf("  Столбец пуст.\n\n");
        return;
    }

    // Инициализируем переменные для расчета математики
    double min_val = atof(col->data[0]);
    double max_val = atof(col->data[0]);
    double sum = 0;

    for (int i = 0; i < col->size; i++) {
        double val = atof(col->data[i]); // Переводим текст в число для расчетов
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
        sum += val;
    }

    double mean = sum / col->size;

    // Выводим красивый отчет
    printf("  Минимум (MIN): %.2f\n", min_val);
    printf("  Максимум (MAX): %.2f\n", max_val);
    printf("  Сумма (SUM):    %.2f\n", sum);
    printf("  Среднее (MEAN): %.2f\n\n", mean);
}

void free_table(Table *table) {
    if (table == NULL) return;
    for (int i = 0; i < table->cols_count; i++) {
        for (int j = 0; j < table->columns[i].size; j++) {
            free(table->columns[i].data[j]);
        }
        free(table->columns[i].data);
    }
    free(table->columns);
    free(table);
}

int main() {
    system("chcp 65001 > nul"); 

    Table *my_table = load_full_table("04_002_basic.csv");
    
    if (my_table != NULL) {
        printf("=== ТАБЛИЦА УСПЕШНО ЗАГРУЖЕНА ===\n");
        printf("Столбцов: %d, Строк: %d\n\n", my_table->cols_count, my_table->rows_count);
        
        // Выводим саму таблицу
        for (int i = 0; i < my_table->cols_count; i++) printf("%-15s\t", my_table->columns[i].name);
        printf("\n--------------------------------------------------\n");
        for (int j = 0; j < my_table->rows_count; j++) {
            for (int i = 0; i < my_table->cols_count; i++) {
                printf("%-15s\t", my_table->columns[i].data[j]);
            }
            printf("\n");
        }
        printf("--------------------------------------------------\n\n");

        // ВЫЗЫВАЕМ АНАЛИТИКУ ДЛЯ КАЖДОГО СТОЛБЦА
        printf("=== АНАЛИТИЧЕСКИЙ ОТЧЕТ ===\n");
        for (int i = 0; i < my_table->cols_count; i++) {
            print_column_stats(&my_table->columns[i]);
        }

        free_table(my_table);
    }
    return 0;
}
