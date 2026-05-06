/*
 * interface.c
 *
 *  Created on: 1 May 2026
 *      Author: jonat
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "uart0.h"
#include "emp_type.h"
#include "FreeRTOS.h"
#include "rtos_def.h"

#include "task.h"
#include "queue.h"

#include "globals.h"

#define SALESLENGTH 32
#define METHOD_LENGTH 17


extern QueueHandle_t xInterface_Queue;
//extern Product lat , esp , fil;

typedef struct {
    INT32U time;
    Product product;
    char method[METHOD_LENGTH];
    FP32 amount;
} Transaction;

int split(char* str, char delim, char* tokens[], int max_tokens) {
    int count = 0;
    tokens[count++] = str;

    while (*str && count < max_tokens) {
        if (*str == delim) {
            *str = '\0';
            tokens[count++] = str + 1;
        }
        str++;
    }
    return count;
}

BOOLEAN isOnlyDigits(char* text)
{
    INT8U idx;
    for ( idx = 0 ; idx < METHOD_LENGTH ; idx++ )
    {
        if ( text[idx] == '\0' )
        {
            continue;
        }

        if( !isdigit(text[idx]) )
        {
            return 0;
        }
    }
    return 1;
}

void write_string(char *str)
{
    INT8U idx = 0;
    for (idx = 0 ; idx < strlen(str) ; idx ++)
    {
        while(uart0_tx_rdy() == 0){}
        uart0_putc(str[idx]);
    }
}


void interface_task ( void *pvParameters )
{
    uart0_init(115200,8,1,'n');

    Transaction sales[SALESLENGTH];
    INT8U saleIdx = 0;
    INT32U time_since_boot;

    char transaction_info[24];
    Product purchased_beverage;
    char trans_method[17];
    char* endptr;
    FP32 amount;

    char uart_string[96];

    char uart_rx_string[24];
    char uart_rx_curchar;
    INT16U uart_rx_ArrIdx = 0;

    BOOLEAN flag = 0;


    while(1)
    {
        // for each transaction print "(time of day(operating time)) , (product type) , method , price , amount/(cl)"
        if (xQueueReceive(xInterface_Queue,&transaction_info,pdMS_TO_TICKS(QUEUE_MAX_WAIT)) == pdPASS)
        {
            // receive data
            time_since_boot = xTaskGetTickCount() * portTICK_PERIOD_MS;

            char* trans_tok[8];
            INT8U n_tok_trans = split(transaction_info,' ',trans_tok,3);

            switch (trans_tok[0][0])
            {
            case '1':
                purchased_beverage = esp;
                break;
            case '2':
                purchased_beverage = lat;
                break;
            case '3':
                purchased_beverage = fil;
                break;
            default:
                break;
            }            

            strcpy(trans_method,trans_tok[1]);

            amount = strtof(trans_tok[2],&endptr);

            Transaction purchase;
            purchase.time    = time_since_boot;
            purchase.product = purchased_beverage;
            purchase.amount  = amount;
            strcpy(purchase.method, trans_method);

            sales[saleIdx] = purchase;
            saleIdx++;

            snprintf(uart_string, sizeof(uart_string), "time: %lu, method: %s, drink: %s, total price: %.2f, Units: %.2f\n",
                                                        purchase.time,
                                                        purchase.method,
                                                        purchase.product.name,
                                                        purchase.product.price * purchase.amount,
                                                        purchase.amount);

            write_string(uart_string);
        }



        // set price
        while(uart0_rx_rdy() != 0)
        {
            flag = 1;
            uart_rx_curchar = uart0_getc();
            uart_rx_string[uart_rx_ArrIdx] = uart_rx_curchar;

            uart_rx_ArrIdx++;
        }

        if (uart0_rx_rdy() == 0 && flag == 1)
        {
            flag = 0;
            uart_rx_ArrIdx = 0;
            //write_string(uart_rx_string);

            //** Set price to value **//
            char* uart_tok[8];
            INT8U n_tok_uart = split(uart_rx_string,' ',uart_tok,8);

            if (strcmp(uart_tok[0],"set") == 0)
            {
                INT8U price = strtol(uart_tok[2],&endptr,10);

                if (strcmp(uart_tok[1],"latte") == 0)
                {
                    lat.price = price;
                    snprintf(uart_string, sizeof(uart_string), "price of %s is set to %d\n",
                                                        lat.name,
                                                        lat.price);
                    write_string(uart_string);
                }
                else if (strcmp(uart_tok[1],"espresso") == 0)
                {
                    esp.price = price;
                    snprintf(uart_string, sizeof(uart_string), "price of %s is set to %d\n",
                                                        esp.name,
                                                        esp.price);
                    write_string(uart_string);
                }
                else if (strcmp(uart_tok[1],"filter") == 0)
                {
                    fil.price = price;
                    snprintf(uart_string, sizeof(uart_string), "price of %s is set to %d\n",
                                                        fil.name,
                                                        fil.price);
                    write_string(uart_string);
                }
            }
            //** End set price to value **//


            //** Query sales **//
            if (strcmp(uart_tok[0],"query") == 0)
            {
                INT8U transIdx = 0;
                if (strcmp(uart_tok[1],"all") == 0)
                {
                    //** Search product 1 **//
                    for (transIdx = 0 ; transIdx < SALESLENGTH ; transIdx++)
                    {
                        if ( sales[transIdx].product.idx == '\0' )
                        {
                            continue;
                        }
                        if (sales[transIdx].product.idx == esp.idx)
                        {
                            snprintf(uart_string, sizeof(uart_string), "time: %lu, method: %s, drink: %s, total price: %.2f, Units: %.2f\n",
                                                                     sales[transIdx].time,
                                                                     sales[transIdx].method,
                                                                     sales[transIdx].product.name,
                                                                     sales[transIdx].product.price * sales[transIdx].amount,
                                                                     sales[transIdx].amount);

                            write_string(uart_string);
                        }
                    }

                    //** Search product 2 **//
                    for (transIdx = 0 ; transIdx < SALESLENGTH ; transIdx++)
                    {
                        if ( sales[transIdx].product.idx == '\0' )
                        {
                            continue;
                        }
                        if (sales[transIdx].product.idx == lat.idx)
                        {
                            snprintf(uart_string, sizeof(uart_string), "time: %lu, method: %s, drink: %s, total price: %.2f, Units: %.2f\n",
                                                                     sales[transIdx].time,
                                                                     sales[transIdx].method,
                                                                     sales[transIdx].product.name,
                                                                     sales[transIdx].product.price * sales[transIdx].amount,
                                                                     sales[transIdx].amount);

                            write_string(uart_string);
                        }
                    }

                    //** Search product 3 **//
                    for (transIdx = 0 ; transIdx < SALESLENGTH ; transIdx++)
                    {
                        if ( sales[transIdx].product.idx == '\0' )
                        {
                            continue;
                        }
                        if (sales[transIdx].product.idx == fil.idx)
                        {
                            snprintf(uart_string, sizeof(uart_string), "time: %lu, method: %s, drink: %s, total price: %.2f, Units: %.2f\n",
                                                                     sales[transIdx].time,
                                                                     sales[transIdx].method,
                                                                     sales[transIdx].product.name,
                                                                     sales[transIdx].product.price * sales[transIdx].amount,
                                                                     sales[transIdx].amount);

                            write_string(uart_string);
                        }
                    }
                    //** End search **//
                }


                if (strcmp(uart_tok[1],"cash") == 0)
                {
                    FP32 cashRev = 0;
                    for (transIdx = 0 ; transIdx < SALESLENGTH ; transIdx++)
                    {
                        if (strcmp(sales[transIdx].method,"CASH") == 0)
                        {
                            cashRev += sales[transIdx].product.price * sales[transIdx].amount;
                        }
                    }
                    snprintf(uart_string, sizeof(uart_string), "Cash sales: %.2f\n",cashRev);
                    write_string(uart_string);
                }

                if (strcmp(uart_tok[1],"card") == 0)
                {
                    FP32 cardRev = 0;
                    for (transIdx = 0 ; transIdx < SALESLENGTH ; transIdx++)
                    {
                        if (isOnlyDigits(sales[transIdx].method))
                        {
                            cardRev += sales[transIdx].product.price * sales[transIdx].amount;
                        }
                    }
                    snprintf(uart_string, sizeof(uart_string), "Card sales: %.2f\n",cardRev);
                    write_string(uart_string);
                }
            }

            if (strcmp(uart_tok[1],"uptime") == 0)
            {
                time_since_boot = xTaskGetTickCount() * portTICK_PERIOD_MS;
                snprintf(uart_string, sizeof(uart_string), "Total uptime: %d [ms]\n",time_since_boot);
                write_string(uart_string);
            }

            //** End query sales **//

            memset(uart_rx_string, 0, sizeof(uart_rx_string));
            memset(uart_string, 0, sizeof(uart_string));
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
