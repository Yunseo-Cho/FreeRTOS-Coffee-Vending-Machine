/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: rotary_encoder.h
*
* PROJECT....: Assignment 8
*
* DESCRIPTION: ...
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260314  MoH   Module created.
*
*****************************************************************************/

#ifndef ROTARY_ENCODER_H_
#define ROTARY_ENCODER_H_

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
void rot_enc_init(void);


INT8U rotary_read();

INT8U button_read();

/*****************************************************************************
*   Input    : -
*   Output   : -
*   Function : -
******************************************************************************/
void rotary_task(void *pvParameters);
/*****************************************************************************
*   Input    : -
*   Output   : -
*   Function : -
******************************************************************************/

/****************************** End Of Module *******************************/
#endif /*ROTARY_ENCODER_H_*/
