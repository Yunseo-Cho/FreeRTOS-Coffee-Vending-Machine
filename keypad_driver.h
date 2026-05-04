/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: keypad_driver.h
*
* PROJECT....: Final Assignment
*
* DESCRIPTION: ...
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260420  MoH   Module created.
*
*****************************************************************************/

#ifndef KEYPAD_DRIVER_H_
#define KEYPAD_DRIVER_H_

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "tm4c123gh6pm.h"
#include "emp_type.h"
/*****************************    Defines    *******************************/
/*****************************   Constants   *******************************/

/*****************************   Functions   *******************************/

void keypad_init(void);
/*****************************************************************************
*   Input    : 
*   Output   : 
*   Function : 
******************************************************************************/

uint8_t row(uint8_t y);

uint8_t key_catch(uint8_t x, uint8_t y);

BaseType_t check_column(uint8_t x);

void keypad_task(void *pvParameters);
/*****************************************************************************
*   Input    : 
*   Output   : 
*   Function : 
******************************************************************************/

/****************************** End Of Module *******************************/
#endif /*KEYPAD_DRIVER_H_*/
