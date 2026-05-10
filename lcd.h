/*****************************************************************************
* University of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME.: lcd.h
*
* PROJECT....: EMP
*
* DESCRIPTION: Functions to control the LCD
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260314  MoH   Module created.
*
*****************************************************************************/
#ifndef LCD_H_
#define LCD_H_

/***************************** Include files *******************************/
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"
#include "FreeRTOS.h"

/*****************************    Defines    *******************************/
#define TASKDELAY 50


/*****************************   Constants   *******************************/

/*****************************   Functions   *******************************/

void lcd_init(void);
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : Initialize the gpio pins for the lcd display
******************************************************************************/

void lcd_startup_sequence(void);
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : Send a startup sequence to the LCD display
******************************************************************************/

void enablePulse(void);
/*****************************************************************************
*   Input    : 
*   Output   : 
*   Function : Read the values (LCD)
******************************************************************************/


void write4(INT8U data);
/*****************************************************************************
*   Input    : Data to send to LCD (4 bits)
*   Output   :
*   Function : sends data to LCD display
******************************************************************************/

void command(INT8U cmd);
/*****************************************************************************
*   Input    : Command to send to LCD
*   Output   :
*   Function : Init command mode and send commands
******************************************************************************/

void WriteChar(INT8U d);
/*****************************************************************************
*   Input    : integer value of char
*   Output   :
*   Function : Writes char to LCD
******************************************************************************/

void ClearDisplay(void);
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : Clear LCD
******************************************************************************/

void WriteString(char *string);
/*****************************************************************************
*   Input    : array of chars
*   Output   :
*   Function : Displays a string to LCD
******************************************************************************/

void SetCursor(INT8U row, INT8U col);
/*****************************************************************************
*   Input    : Row and Column
*   Output   :
*   Function : Sets the LCD cursor to the row and column
******************************************************************************/


void lcd_task( void *pvParameters );
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : Displays info from queues
******************************************************************************/


/****************************** End Of Module *******************************/


#endif /* LCD_H_ */
