/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: button.c
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

#include "switch.h"

/*****************************    Defines    *******************************/
#define BUTTON_TASK_PRIO   2
#define BUTTON_STACK_SIZE  256

#define SW1_MASK  0x10    // PF4
#define SW2_MASK  0x01    // PF0

/*****************************   Constants   *******************************/
/*****************************   Variables   *******************************/
extern QueueHandle_t xSW1_Queue , xSW2_Queue;

/*****************************   Functions   *******************************/
void switch_init(void)
{
    volatile int dummy;

    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
    dummy = SYSCTL_RCGC2_R;

    GPIO_PORTF_LOCK_R = 0x4C4F434B;
    GPIO_PORTF_CR_R |= 0x11;

    GPIO_PORTF_DIR_R &= ~0x11;   // PF4, PF0 input
    GPIO_PORTF_DEN_R |= 0x11;    // digital enable
    GPIO_PORTF_PUR_R |= 0x11;    // pull-up enable
}

INT8U sw1_pressed(void)
{
  return !(GPIO_PORTF_DATA_R & SW1_MASK);
}

INT8U sw2_pressed(void)
{
  return !(GPIO_PORTF_DATA_R & SW2_MASK);
}

void switch_task(void *pvParameters)
{
    GPIO_PORTF_DATA_R |= 0x04;
    vTaskDelay(pdMS_TO_TICKS(500));
    GPIO_PORTF_DATA_R &= ~0x04;

  uint8_t pre_sw1 = 0;
  uint8_t pre_sw2 = 0;
  /*
  while (1)
    {
      uint8_t sw1 = sw1_pressed();
      uint8_t sw2 = sw2_pressed();

      if (sw1_pressed())
      {
          xQueueOverwrite(xSW1_Queue, &sw1);
          pre_sw1 = sw1;
      }

      if (sw2 != pre_sw2)
      {
          xQueueOverwrite(xSW2_Queue, &sw2);
          pre_sw2 = sw2;
      }

      vTaskDelay(pdMS_TO_TICKS(50));
    }
    */

  while (1)
  {
    uint8_t sw1 = sw1_pressed();
    uint8_t sw2 = sw2_pressed();

    if (sw1 != pre_sw1)
    {
        xQueueOverwrite(xSW1_Queue, &sw1);
        pre_sw1 = sw1;
    }

    if (sw2 != pre_sw2)
    {
        xQueueOverwrite(xSW2_Queue, &sw2);
        pre_sw2 = sw2;
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }

}
