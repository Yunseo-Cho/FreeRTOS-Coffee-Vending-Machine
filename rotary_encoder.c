/*****************************************************************************
* University of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME.: rotary.c
*
* PROJECT....: EMP
*
* DESCRIPTION: Assignment 8 - Rotary encoder
*
* Change Log:
*****************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260314  MoH   Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "rtos_def.h"
#include "rotary_encoder.h"
#include "queue.h"
#include "tm4c123gh6pm.h"
#include "emp_type.h"
/*****************************    Defines    *******************************/
#define ENCODER  GPIO_PORTA_DATA_R
#define ENC_A    ((ENCODER >> 5) & 1)   // PA5 - DIGI A
#define ENC_B    ((ENCODER >> 6) & 1)   // PA6 - DIGI B
#define ENC_P2   ((ENCODER >> 7) & 1)   // PA7 - DIGI P2

#define DKK_5 5 
#define DKK_20 20 
#define CIRCLE_ANGLE 360
/*****************************   Constants   *******************************/
/*****************************   Variables   *******************************/
extern QueueHandle_t xEnc_rotary_Queue , xEnc_Button_Queue;


/*****************************   Functions   *******************************/
void rot_enc_init(void)
{
    volatile int dummy;
    SYSCTL_RCGC2_R  |=  SYSCTL_RCGC2_GPIOA;

    // Do a dummy read to insert a few cycles after enabling the peripheral.
    dummy = SYSCTL_RCGC2_R;

    GPIO_PORTA_DIR_R &= ~0xE0;       // PA5, PA6, PA7 as INPUT
    GPIO_PORTA_DEN_R |=  0xE0;       // Enable digital function
    GPIO_PORTA_PUR_R |=  0xE0;       // Enable pull-up resistors
}


INT8U rotary_read()
{
    return (ENC_A << 1) | ENC_B;
}

INT8U button_read()
{
    return (ENC_P2);
}


void rotary_task(void *pvParameters)
{
    INT8U flag = 0;

    INT8U P = button_read();
    INT8U pre_P = P;

    INT8U AB = rotary_read();
    INT8U pre_AB = AB;
    INT8U A = (AB >> 1) & 0x01;
    INT8U B = AB & 0x01;

    INT16S coin = 0;

    while (1){

        P = button_read();
        if (P != pre_P)
        {
            xQueueOverwrite(xEnc_Button_Queue, &P);
            GPIO_PORTF_DATA_R ^= 0x02;  // Turn ON LED
            pre_P = P;
        }

        AB = rotary_read();
        A = (AB & 0x02) >> 1;
        B = (AB & 0x01);

        if(AB != pre_AB)// if previous value != current value
        {
            if (flag == 0)
            {
                flag = 1;
                // Calculate the XOR with ^ operator
                INT8U YY = AB ^ pre_AB;

                if (A == B)
                {
                    // Check for CW (clock wise)
                    if(YY == 0x01) // YY = [01]
                        {
                        coin = coin + DKK_20;
                        }
                    // Check for CCW (counter clock wise)
                    else if(YY == 0x02) // YY = [10]
                        {
                        coin = coin + DKK_5;
                        }
                    // Check for spike/error
                    else
                        {

                        }
                }
                else
                {
                    // Check for CW (clock wise)
                    if (YY == 0x02) // YY = [10]
                        {
                        coin = coin + DKK_20;
                        }
                    // Check for CCW (counter clock wise)
                    else if (YY == 0x01) // YY = [01]
                        {
                        coin = coin + DKK_5;
                        }
                    // Check for spike/error
                    else
                    {

                    }
                }

                /*
                if (coin >= CIRCLE_ANGLE)
                {
                    coin = coin - CIRCLE_ANGLE;
                }
                else if (coin <= -CIRCLE_ANGLE)
                {
                    coin = coin + CIRCLE_ANGLE;
                }
                */




            xQueueSend(xEnc_rotary_Queue, &coin, 0);

            }
            else if (flag == 1)
            {
                flag = 0;
            }
            pre_AB = AB;
            coin = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
