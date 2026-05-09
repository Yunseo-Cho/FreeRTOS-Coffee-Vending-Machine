/*
 * interface.h
 *
 *  Created on: 1 May 2026
 *      Author: jonat
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#define TASKDELAY 10

#define UART_BAUDRATE 115200
#define UART_DATABITS 8
#define UART_STOPBITS 1

#define TRANS_METHOD_LENGTH 17

#define UART_SEND_LENGTH 96
#define UART_RECEIVE_LENGTH 24


#define MAXTOKENS   8
// Receive from queue idx
#define PRODUCT_IDX 0
#define METHOD_IDX  1
#define AMOUNT_IDX  2

// Receive from UART idx
#define CMD_IDX 0
#define CMD2_IDX 1
#define PRODUCT_NAME_IDX 1
#define NEW_PRICE_IDX 2


#define BASE10 10


// Functions
void intToStr(int N, char *str);
void write_string(char *str);
void interface_task ( void *pvParameters );

#endif /* INTERFACE_H_ */
