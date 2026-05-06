/*****************************************************************************
* University of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME.: coffe_machine.h
*
* PROJECT....: EMP
*
* DESCRIPTION: 
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260422  MoH   Module created.
*
*****************************************************************************/
#ifndef COFFEE_MACHINE_H_
#define COFFEE_MACHINE_H_

/***************************** Include files *******************************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*****************************    Defines    *******************************/
#define taskDelay 100 //ms

#define esp_price 15
#define lat_price 27
#define fil_price 3

#define menu_length 3

#define LED_R  0x02   // PF1
#define LED_Y  0x04   // PF2
#define LED_G  0x08   // PF3
#define LED_ALL (LED_R | LED_Y | LED_G)

#define CARD_NUM_MAX 16
#define CARD_CODE_MAX 4
#define CARD_IDX_MAX 20

#define GRINDING_MS 7500
#define BREWING_MS 14000
#define FROTHING_MS 6200
#define SLOW_FILTER_MS 3000
#define SLOW_FILTER_VEL 0.6f
#define FILTER_VEL 1.45f
#define INACTIVITY_TIME 5

#define DISPLAY_LEN 7
#define TEMP_LEN 5

#define SHOW_TEXT 1000
#define SHOW_RESULT 5000
#define INACTIVE_MS 1000

#define FREQ 4
#define PERIOD 250

/*****************************   Constants   *******************************/



/*****************************   Functions   *******************************/

void coffee_machine_task( void *pvParameters );
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : 
******************************************************************************/

/****************************** End Of Module *******************************/


#endif /* COFFE_MACHINE_H_ */
