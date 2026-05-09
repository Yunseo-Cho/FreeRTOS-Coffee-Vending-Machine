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

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "rtos_def.h"
#include "keypad_driver.h"
#include "task.h"
#include "queue.h"
#include "tm4c123gh6pm.h"
#include "emp_type.h"



/*****************************    Defines    *******************************/
#define TASKDELAY 5
/*****************************   Constants   *******************************/
/*****************************   Variables   *******************************/
extern QueueHandle_t xKeypad_Queue;

/*****************************   Functions   *******************************/

void keypad_init(void)
/*****************************************************************************
*   Input    : -
*   Output   : -
*   Function : -
******************************************************************************/
{
    /******************************************************
     *       EMP        GPIO
     * X1 - KEYB F      PA4
     * X2 - KEYB E      PA3
     * X3 - KEYB D      PA2
     *
     * Y1 - KEYB K      PE3
     * Y2 - KEYB J      PE2
     * Y3 - KEYB H      PE1
     * Y4 - KEYB G      PE0
     ******************************************************/
    volatile int dummy;
    SYSCTL_RCGC2_R  |=  (SYSCTL_RCGC2_GPIOA | SYSCTL_RCGC2_GPIOE);

    // Do a dummy read to insert a few cycles after enabling the peripheral.
    dummy = SYSCTL_RCGC2_R;

    GPIO_PORTA_DIR_R |=  0x1C;       // PA2, PA3, PA4 as OUTPUT
    GPIO_PORTA_DEN_R |=  0x1C;       // Enable digital function
    GPIO_PORTE_DIR_R &= ~0x0F;       // PE0, PE1, PE2, PE3 as INPUT
    GPIO_PORTE_DEN_R |=  0x0F;       // Enable digital function
}


INT8U row(INT8U y)
{
    switch(y)
    {
        case 0x01: return 1;
        case 0x02: return 2;
        case 0x04: return 3;
        case 0x08: return 4;
        default:   return 0;
    }
}

INT8U key_catch(INT8U x, INT8U y)
{
    const INT8U matrix[3][4] = {
        {'*','7','4','1'},
        {'0','8','5','2'},
        {'#','9','6','3'}
    };

    return matrix[x-1][y-1];
}

BaseType_t check_column(INT8U x)
{
    INT8U y = GPIO_PORTE_DATA_R & 0x0F;

    if(y)
    {
        INT8U ch = key_catch(x, row(y));
        xQueueSendToBack(xKeypad_Queue, &ch, pdMS_TO_TICKS(QUEUE_MAX_WAIT));
        return pdTRUE;
    }
    return pdFALSE;
}

void keypad_task(void *pvParameters)
/*****************************************************************************
*   Input    : -
*   Output   : -
*   Function : -
******************************************************************************/
{
    INT8U pressed = 0;

    for(;;)
    {
        if(!pressed)
        {
            GPIO_PORTA_DATA_R &= 0xE3;
            GPIO_PORTA_DATA_R |= 0x10;
            if(check_column(1))
                pressed = 1;

            GPIO_PORTA_DATA_R &= 0xE3;
            GPIO_PORTA_DATA_R |= 0x08;
            if(check_column(2))
                pressed = 1;

            GPIO_PORTA_DATA_R &= 0xE3;
            GPIO_PORTA_DATA_R |= 0x04;
            if(check_column(3))
                pressed = 1;

            GPIO_PORTA_DATA_R &= 0xE3;  // All columns LOW after scanning
        }
        else
        {
            GPIO_PORTA_DATA_R |= 0x1C;  // All columns HIGH to detect any release
            if(!(GPIO_PORTE_DATA_R & 0x0F))
            {
                pressed = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(TASKDELAY));
    }
}
