/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: timer.c
*
* PROJECT....: ECP
*
* DESCRIPTION: See module specification file (.h-file).
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 090215  MoH   Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "emp_type.h"

#include "timer.h"

/*****************************    Defines    *******************************/
/*****************************   Constants   *******************************/
/*****************************   Variables   *******************************/
static INT8U timer_100ms = 0;

/*****************************   Functions   *******************************/
void timer_reset(void)
{
    timer_100ms = 0;
}

void timer_tick_100ms(void)
{
    if (timer_100ms < 255)
    {
        timer_100ms++;
    }
}

INT8U timer_get_100ms(void)
{
    return timer_100ms;
}

void timer_to_string(char *str)
{
    INT8U sec = timer_100ms / 10;
    INT8U dec = timer_100ms % 10;

    str[0] = (sec / 10) + '0';
    str[1] = (sec % 10) + '0';
    str[2] = '.';
    str[3] = dec + '0';
    str[4] = ' ';
    str[5] = 's';
}

