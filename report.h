#ifndef REPORT_H
#define REPORT_H

#include <stdio.h>
#include "csv_parser.h"

void print_report(FILE *output, const CsvTable *table, char *name);

#endif