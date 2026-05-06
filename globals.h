#ifndef GLOBALS_H_
#define GLOBALS_H_

#include "emp_type.h"

// global struct for products
typedef struct{
    char idx;
    char name[14];
    INT8U price;
} Product;

#define espIdx '1'
#define latIdx '2'
#define filIdx '3'


//void intToStr(int N, char *str);

#endif //GLOBALS_H_
