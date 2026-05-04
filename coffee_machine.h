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
