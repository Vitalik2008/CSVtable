#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "columns.h"

static int int64_in_range(const char *value) {
    const char *digits = value;
    int negative = 0;

    if (*digits=='-' ||  *digits=='+') {
        negative=(*digits == '-');
        digits++;
    }

    while (*digits=='0' && digits[1]!='\0') {
        digits++;
    }

    const char *limit;

    if (negative) {
        limit = "9223372036854775808";
    } else {
        limit = "9223372036854775807";   
    }

    size_t dig_length = strlen(digits);
    size_t lim_length = strlen(limit);

    if (dig_length<lim_length) return 1;
    if (dig_length>lim_length) return 0;

    return (strcmp(digits, limit))<=0;
}

ColumnType detect_type(const char *value, int was_quoted) {
    int is_int= 1;
    int is_double = 1;
    int ch_dot=0;
    int dig_bef_dot=0;
    int dig_aft_dot=0;


    if ( value[0]=='\0' && was_quoted==0) {
        return TYPE_NULL;
    }
    if (value[0]=='\0' && was_quoted==1) {
        return TYPE_STRING;
    }

    for (int i =0; value[i]!='\0'; i++) {
        if(i==0) {
            if (((value[i]>='0' && value[i] <= '9') || (value[i]=='-' || value[i]=='+'))==0) {
                is_int=0;
                break;
            }
        }
        else if ((value[i]>='0' && value[i] <= '9')==0) {
            is_int=0;
            break;
        }
    }
    if ((value[0]=='+' || value[0]=='-')&& value[1]=='\0'){
        is_int=0;
    }
    if (is_int == 1 && int64_in_range(value) == 0) {
        is_int=0;
    }
    if (is_int==0) {
        for (int i=0; value[i]!='\0'; i++) {
            if (i==0) {
                if (((value[i]>='0' && value[i] <= '9') || (value[i]=='-' || value[i]=='+'))==0) {
                    is_double=0;
                    break;
                }
            }
            else if (value[i]=='.') {
                if (ch_dot==0) {
                    ch_dot=1;
                }else {
                    is_double=0;
                    break;
                }
            }
            else if ((value[i]>='0' && value[i] <= '9')==0) {
                is_double=0;
                break;
            }

            if (ch_dot==1 && (value[i]>='0' && value[i] <= '9')) {
                dig_aft_dot++;
            }
            if (ch_dot==0 && (value[i]>='0' && value[i] <= '9')) {
                dig_bef_dot++;
            }
        }
    }

    if (is_int==1) {
        return TYPE_INT;
    }
    if (is_double==1 && dig_aft_dot>0 && dig_bef_dot>0) {
        return TYPE_DOUBLE;
    }
    return TYPE_STRING;
}

ColumnType merge_types(ColumnType current_type, ColumnType new_type) {
    if (current_type==TYPE_NULL) {
        current_type=new_type;
        return current_type;
    }
    if (new_type==TYPE_STRING) {
        current_type = TYPE_STRING;
        return current_type;
    }
    if(current_type==TYPE_INT && new_type==TYPE_DOUBLE) {
        current_type=TYPE_DOUBLE;
        return current_type;
    }
    return current_type;
}

const char *type_to_string(ColumnType type) {
    switch (type)
    {
    case TYPE_NULL:
        return "NULL";
    
    case TYPE_INT:
        return "INT";
    
    case TYPE_DOUBLE:
        return "DOUBLE";
    
    case TYPE_STRING:
        return "STRING";
    
    default:
        return "UNKNOWN";
    }
}

void update_column_type(ColumnType *column_types, int index, const char *value, int was_quoted) {
    ColumnType old_type = column_types[index];
    ColumnType new_type = detect_type(value, was_quoted);

    column_types[index] = merge_types(old_type,new_type);
}
