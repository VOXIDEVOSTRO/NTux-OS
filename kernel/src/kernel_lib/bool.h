#ifndef BOOL_H
#define BOOL_H

typedef enum {
    FALSE = 0,
    TRUE  = 1
} boolean;

#define bool   boolean
#define true   TRUE
#define false  FALSE
#define YES    TRUE
#define NO     FALSE
#define OK     TRUE
#define FAIL   FALSE
#define is(x)          (x)
#define not(x)         (!(x))
#define and            &&
#define or             ||

void print_bool(const char* name, boolean value);
const char* boolean_to_string(boolean b);

#endif