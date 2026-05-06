/*****************************************************************************
* University of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME.: main.c
*
* PROJECT....: EMP
*
* DESCRIPTION: Assignment Final, main module. No main.h file.
*
* Change Log:
*****************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260420  MoH   Module created.
*
*****************************************************************************/


/***************************** Include files *******************************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"

#include "systick_frt.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "rtos_def.h"

#include "status_led.h"

#include "lcd.h"
#include "rotary_encoder.h"
#include "keypad_driver.h"
#include "coffee_machine.h"
#include "switch.h"
#include "interface.h"

/*****************************    Defines    *******************************/

/*****************************   Constants   *******************************/

/*****************************   Variables   *******************************/
QueueHandle_t xSW1_Queue, xSW2_Queue, xEnc_rotary_Queue, xEnc_Button_Queue, xKeypad_Queue, xLCD_Queue , xInterface_Queue;


/*****************************   Functions   *******************************/
void red_led_init(void)
{
    volatile int dummy;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
    dummy = SYSCTL_RCGC2_R;  // dummy read to allow peripheral to stabilize

    GPIO_PORTF_DIR_R |= 0x02;  // PF2 as output
    GPIO_PORTF_DEN_R |= 0x02;  // Enable digital function
    GPIO_PORTF_PUR_R &= ~0x02; // Disable pull-up
    GPIO_PORTF_DATA_R |= 0x02; // Turn off LED
}


void yellow_led_init(void)
{
    volatile int dummy;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
    dummy = SYSCTL_RCGC2_R;  // dummy read to allow peripheral to stabilize

    GPIO_PORTF_DIR_R |= 0x04;  // PF2 as output
    GPIO_PORTF_DEN_R |= 0x04;  // Enable digital function
    GPIO_PORTF_PUR_R &= ~0x04; // Disable pull-up
    GPIO_PORTF_DATA_R |= 0x04; // Turn off LED
}

void green_led_init(void)
{
    volatile int dummy;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOF;
    dummy = SYSCTL_RCGC2_R;  // dummy read to allow peripheral to stabilize

    GPIO_PORTF_DIR_R |= 0x08;  // PF2 as output
    GPIO_PORTF_DEN_R |= 0x08;  // Enable digital function
    GPIO_PORTF_PUR_R &= ~0x08; // Disable pull-up
    GPIO_PORTF_DATA_R |= 0x08; // Turn off LED
}

static void setupHardware(void)
/*****************************************************************************
*   Input    :  -
*   Output   :  -
*   Function :
*****************************************************************************/
{
    // Put hardware configuration and initialization in here
    // Warning: If you do not initialize the hardware clock, the timings will be inaccurate
    init_systick();
    status_led_init();

    // Initialize physical inputs
    keypad_init();
    rot_enc_init();
    switch_init();

    // Initialize phyiscal outputs
    lcd_init();
    green_led_init();
    yellow_led_init();
    red_led_init();

}

int main(void)
 {
    setupHardware();

    // Queues
    xEnc_rotary_Queue = xQueueCreate(QUEUE_LENGTH,sizeof(INT16S));
    xEnc_Button_Queue  = xQueueCreate(QUEUE_LENGTH,sizeof(INT8U));
    xKeypad_Queue  = xQueueCreate(QUEUE_LENGTH,sizeof(uint8_t));
    xLCD_Queue     = xQueueCreate(QUEUE_LENGTH,sizeof(char)*LCD_POS_TO_SEND+sizeof(char)*LCD_CHARS_TO_SEND);
    xSW1_Queue     = xQueueCreate(QUEUE_LENGTH,sizeof(INT8U));
    xSW2_Queue     = xQueueCreate(QUEUE_LENGTH,sizeof(INT8U));
    xInterface_Queue = xQueueCreate(QUEUE_LENGTH,sizeof(char)*24);

    // Tasks
    xTaskCreate( status_led_task, "Status_led", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL );
    xTaskCreate( rotary_task, "Rotary", USERTASK_STACK_SIZE, NULL, LOW_PRIO, NULL );
    xTaskCreate( keypad_task, "Keypad", USERTASK_STACK_SIZE, NULL, HIGH_PRIO, NULL );
    xTaskCreate( lcd_task , "Lcd" , USERTASK_STACK_SIZE*2, NULL, MED_PRIO, NULL );
    xTaskCreate( coffee_machine_task , "Coffe_Machine" , USERTASK_STACK_SIZE*2, NULL, MED_PRIO, NULL );
    xTaskCreate( switch_task , "Switch" , USERTASK_STACK_SIZE, NULL, HIGH_PRIO, NULL );
    xTaskCreate( interface_task , "interface" , USERTASK_STACK_SIZE*8, NULL, HIGH_PRIO, NULL );


    // ---- Task Todo list ----
    // Rotary_task -- Handling user input from the rotary encoder
    // Keypad_task -- Handling user input from the keypad
    // UART_task -- remote control of the coffee machine

    // Coffee_machine_task -- Handling the state machine of the coffee machine
        // Payment_task -- Handling payment and change // LED green display change
        // Product_task -- Selecting product and calculating price
        // Production_task -- Making the coffee

    // LCD_task -- Displaying interface
    vTaskStartScheduler();

    return 0;
}
