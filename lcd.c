/*****************************************************************************
* University of Southern Denmark
* Embedded C Programming (ECP)
*
* MODULENAME.: lcd.c
*
* PROJECT....:
*
* DESCRIPTION: See module specification file (.h-file).
*
* Change Log:
******************************************************************************
* Date    Id    Change
* YYMMDD
* --------------------
* 260314      Module created.
*
*****************************************************************************/

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tm4c123gh6pm.h"
#include "emp_type.h"

#include "lcd.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lcd_commands.h"

/*****************************    Defines    *******************************/

#define LCD_PORTC GPIO_PORTC_DATA_R
#define LCD_PORTD GPIO_PORTD_DATA_R

#define LCD_D4 (LCD_PORTC & (1 << 4))
#define LCD_D5 (LCD_PORTC & (1 << 5))
#define LCD_D6 (LCD_PORTC & (1 << 6))
#define LCD_D7 (LCD_PORTC & (1 << 7))

#define LCD_RS (1 << 2)
#define LCD_E (1 << 3)

#define QUEUE_MAX_WAIT 5

#define STRING_LENGTH 16
/*****************************   Constants   *******************************/
/*****************************   Variables   *******************************/
extern QueueHandle_t xLCD_Queue;
extern QueueHandle_t xKeypad_Queue; //TEMP
uint8_t ch; //TEMP
char keypad_ch[STRING_LENGTH];
/*****************************   Functions   *******************************/

void lcd_init(void)
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : Initialize the gpio pins for the lcd display
******************************************************************************/
{
    INT8S dummy;
    // Enable the GPIO port that is used for the on-board LED.
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC | SYSCTL_RCGC2_GPIOD;

    // Do a dummy read to insert a few cycles after enabling the peripheral.
    dummy = SYSCTL_RCGC2_R;

    GPIO_PORTC_DIR_R |= 0xF0;
    GPIO_PORTC_DEN_R |= 0xF0;

    GPIO_PORTD_DIR_R |= LCD_RS | LCD_E;
    GPIO_PORTD_DEN_R |= LCD_RS | LCD_E;

    lcd_startup_sequence();
}

void lcd_startup_sequence(void) 
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : Send a startup sequence to the LCD display
******************************************************************************/
{
    // ----- Commands -----
    // 0x30, Reset
    // 0x20, Set 4bit interface
    // 0x28, 2 lines Display
    // 0x0C, Display ON, Cursor OFF, Blink OFF
    // 0x06, Cursor Increment
    // 0x01, Clear Display
    // 0x02, Home
    // 0xFF, stop

    // Startup sequence
    write4(0x30);
    write4(0x30);
    write4(0x30);

    write4(0x20);

    command(0x28);
    command(0x0C);
    command(0x01);
    command(0x06);
}

// Enable pulse
void pulseEnable(void)
/*****************************************************************************
*   Input    : 
*   Output   : 
*   Function : Read the values (LCD)
******************************************************************************/
{
    LCD_PORTD |= LCD_E;
    volatile int i = 0;
    for(i = 0;i<50;i++);
    LCD_PORTD &= ~LCD_E;
}

// Communicating functions
void write4(INT16U data)
/*****************************************************************************
*   Input    : Data
*   Output   :
*   Function : sends data to LCD display
******************************************************************************/
{
    LCD_PORTC &= ~0xF0;
    LCD_PORTC |= (data << 4);
    pulseEnable();
}

void command(INT16U cmd)
/*****************************************************************************
*   Input    : Command
*   Output   :
*   Function : Init command mode and send commands
******************************************************************************/
{
    LCD_PORTD &= ~LCD_RS; 
    write4(cmd >> 4);
    write4(cmd);
}

// ===== SEND BYTE =====

void SetCursor(INT8U row, INT8U col)
/*****************************************************************************
*   Input    : Row and Column
*   Output   :
*   Function : Sets the LCD cursor to the row and column
******************************************************************************/
{
    if(row == 1)              // Bottom part of display
        row = 0x40;
    else
        row = 0x00;           // Top part of display
    INT8U pos_address = row + col;

    command(0x80 | pos_address);
}

void WriteChar(INT16U d)
/*****************************************************************************
*   Input    : integer value of char
*   Output   :
*   Function : Writes char to LCD
******************************************************************************/
{
    LCD_PORTD |= LCD_RS; // Data mode
    write4(d >> 4);
    write4(d);
}

void ClearDisplay(void)
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : Clear LCD
******************************************************************************/
{
    SetCursor( 0,0 );
    WriteString("                   ");
    SetCursor( 1,0 );
    WriteString("                   ");
}

void WriteString(char *string)
/*****************************************************************************
*   Input    : array of chars
*   Output   :
*   Function : Displays a string to LCD
******************************************************************************/
{
    int i;
    /* Write each character of the string
     * Using writechar function
     * */
    for (i = 0; i < strlen(string); ++i)
    {
        WriteChar(string[i]);
    }
}

void lcd_task( void *pvParameters)
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : Displays info from queues
******************************************************************************/
{
    vTaskDelay( pdMS_TO_TICKS(500) );
    ClearDisplay();
    INT8U row = 0;
    INT8U col = 0;
    char str_to_print[16];
    while(1)
    {
        char ch[18];

        if (xQueueReceive(xLCD_Queue, &ch, pdMS_TO_TICKS(QUEUE_MAX_WAIT)) == pdPASS)
        {
            if (strcmp(ch, CLEAR) == 0)
            {
                ClearDisplay();
            }
            else
            {
                // '1' '0' "hello"
                row = ch[0] - '0';
                col = ch[1] - '0';
                SetCursor ((INT8U)row,(INT8U)col);

                strcpy(str_to_print,ch+2);
                WriteString (str_to_print);
            }
            /*
            if (strcmp(ch, STARTSCREEN) == 0)
            {
                ClearDisplay();
                SetCursor( 0,0 );
                WriteString("Coffee Machine");
                SetCursor( 1,0 );
                WriteString("Click to use");
            }
            else if (strcmp(ch, SEL_PRODUCT) == 0)
            {
                ClearDisplay();
                SetCursor( 0,0 );
                WriteString("Product:");
                SetCursor( 1,0 );
                WriteString("1: Lat 2: Eps 3: Fil");
            }
            else if (strcmp(ch, SEL_PAYMENT) == 0)
            {
                ClearDisplay();
                SetCursor( 0,0 );
                WriteString("1: Cash  2: Card?");
            }
            else
            {
                ClearDisplay();
                SetCursor(0,0);
                WriteString(ch);
            }
            */

        }
        vTaskDelay( pdMS_TO_TICKS(50) );
    }
}

// 13"Hello"
// row = ch[1]
// col = ch[2]
// setcursor(row,col)
// string = ch[3;end-1]
// writestring(string)

/*
uint8_t ch;

if (xQueueReceive(xKeypad_Queue, &ch, pdMS_TO_TICKS(QUEUE_MAX_WAIT)) == pdPASS)
{
    ClearDisplay();
    char str_ch[1] = (char)ch;
    SetCursor( 0,0 );
    WriteString(str_ch);
}*/

/****************************** End Of Module *******************************/




