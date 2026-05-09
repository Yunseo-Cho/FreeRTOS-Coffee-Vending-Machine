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

#define esp_price 15
#define lat_price 27
#define fil_price  3

extern Product esp, lat, fil;


#define TRANSACTION_LENGTH 24

#endif //GLOBALS_H_
