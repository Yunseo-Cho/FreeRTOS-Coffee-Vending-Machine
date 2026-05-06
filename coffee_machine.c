/*****************************************************************************
* University of Southern Denmark
* Embedded Programming (EMP)
*
* MODULENAME.: coffee_machine.c
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

/***************************** Include files *******************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "emp_type.h"

//#include "globals.h"

#include "FreeRTOS.h"
#include "rtos_def.h"

#include "lcd_commands.h"
#include "timer.h"

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

/*****************************   Constants   *******************************/
// global struct for products
typedef struct{
    char idx;
    char name[13];
    INT8U price;
} Product;

enum state {IDLE, SELECTION, PAYMENT, PRODUCTION} current_state;
// inputs
extern QueueHandle_t xEnc_rotary_Queue, xEnc_Button_Queue, xKeypad_Queue, xUART_Queue, xSW1_Queue, xSW2_Queue;
// outputs
extern QueueHandle_t xLCD_Queue;

/*****************************   Functions   *******************************/

void intToStr(INT16S N, char *str)
/*****************************************************************************
*   Input    :
*   Output   :
*   Function :
******************************************************************************/
{
    int i = 0;

    // Save the copy of the number for sign
    int sign = N;

    // If the number is negative, make it positive
    if (N < 0)
        N = -N;

    // Extract digits from the number and add them to the
    // string
    while (N > 0) {

        // Convert integer digit to character and store
        // it in the str
        str[i++] = N % 10 + '0';
        N /= 10;
    }

    // If the number was negative, add a minus sign to the
    // string
    if (sign < 0) {
        str[i++] = '-';
    }

    // Null-terminate the string
    str[i] = '\0';

    INT16S j , k;
    // Reverse the string to get the correct order
    for (j = 0, k = i - 1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

void show_timer(INT8U time_100ms)
/*****************************************************************************
*   Input    :
*   Output   :
*   Function :
******************************************************************************/
{
    char time_str[16];
    char display_time_str[16];

    timer_reset();

    while (timer_get_100ms() < time_100ms)
    {
        timer_tick_100ms();
        timer_to_string(time_str);
        strcat(display_time_str,"10");
        strcat(display_time_str,time_str);
        xQueueSendToBack(xLCD_Queue,display_time_str,pdMS_TO_TICKS(QUEUE_MAX_WAIT));

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void run_timer(INT8U time_100ms)
/*****************************************************************************
*   Input    :
*   Output   :
*   Function :
******************************************************************************/
{
    timer_reset();

    while (timer_get_100ms() < time_100ms)
    {
        timer_tick_100ms();

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void blinkReturnAmount( INT16S money )
/*****************************************************************************
*   Input    : Money
*   Output   : None
*   Function : Returns money by blinking the green LED
******************************************************************************/
{
    INT16S i;
    for( i = 0; i < money; i++ )
    {
        GPIO_PORTF_DATA_R &= ~0x08; // Turn on LED
        vTaskDelay(pdMS_TO_TICKS(200));
        GPIO_PORTF_DATA_R |= 0x08; // Turn off LED
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void coffee_machine_task( void *pvParameters )
/*****************************************************************************
*   Input    :
*   Output   :
*   Function : 
******************************************************************************/
{
    Product esp = {'1',"Espresso",esp_price};
    Product lat = {'2',"Latte",lat_price};
    Product fil = {'3',"Filter Coffee",fil_price};

    Product menu[menu_length] = {esp,lat,fil};
    // Selection variables
    uint8_t sel_bev;
    Product cur_bev;

    // Payment variables
    uint8_t method = '0';

    // Card method
    char card_numb[16];
    char card_code[4];
    INT8U card_idx = 0;
    uint8_t pressed_number;
    // Cash method
    INT16S inserted_money = 0;
    INT16S current_money = 0;

    int check = 0;
    current_state = IDLE;

    // Produce variables
    INT8U cup_present = 0;
    INT8U cup_taken = 0;
    INT8U start_production = 0;
    FP32 amountCoffee = 0;
    INT8U coffeeTimer = 1;

    while(1)
    {
        switch (current_state)
        {
            case IDLE:
            {
                // Reset everthing for use.
                method = '0';
                inserted_money = 0;
                current_money = 0;
                start_production = 0;
                amountCoffee = 0;
                coffeeTimer = 1;
                check = 0;
                card_idx = 0;
                pressed_number = 0;
                memset(card_numb, 0, sizeof(card_numb));
                memset(card_code, 0, sizeof(card_code));
                xQueueReset(xEnc_rotary_Queue);
                xQueueReset(xKeypad_Queue);
                xQueueReset(xSW1_Queue);
                xQueueReset(xSW2_Queue);

                GPIO_PORTF_DATA_R |= LED_ALL;

                if (check == 0)
                {
                    xQueueSendToBack(xLCD_Queue,"00""Coffee Machine",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                    xQueueSendToBack(xLCD_Queue,"10""Click to use",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                    check++;
                }
                INT8U wake = 0;
                if (xQueueReceive(xEnc_Button_Queue,&wake,pdMS_TO_TICKS(QUEUE_MAX_WAIT)) == pdPASS)
                {
                    if (wake == 1)
                    {

                        xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                        xQueueReset(xEnc_Button_Queue);
                        current_state = SELECTION;
                    }
                }

                /*
                Checks the UART for commands
                */

                break;
            }

            case SELECTION:
            {
                // print options
                xQueueSendToBack(xLCD_Queue,"00""Product:",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                xQueueSendToBack(xLCD_Queue,"10""1:Eps 2:Lat 3:Fil",pdMS_TO_TICKS(QUEUE_MAX_WAIT));


                if (xQueueReceive(xKeypad_Queue,&sel_bev,pdMS_TO_TICKS(QUEUE_MAX_WAIT)) == pdPASS)
                {

                    INT8U i;
                    INT8U sel_bev_int = sel_bev;
                    for (i = 0; i < menu_length; i++)
                    {
                        if (menu[i].idx == sel_bev_int)
                        {
                            cur_bev = menu[i];
                            current_state = PAYMENT;
                            xQueueReset(xKeypad_Queue);
                            xQueueReset(xEnc_rotary_Queue);
                            xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                        }
                    }
                }
                break;
            }

            case PAYMENT:
            {
                // choose payment - Card or cash
                switch (method)
                {
                    case '0':
                    {
                        xQueueReset(xEnc_rotary_Queue);
                        //LCD write Cash or Card
                        xQueueSendToBack(xLCD_Queue,"00""Select Payment",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                        xQueueSendToBack(xLCD_Queue,"10""1: Cash  2: Card",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                        if (xQueueReceive(xKeypad_Queue,&method,pdMS_TO_TICKS(QUEUE_MAX_WAIT)) == pdPASS)
                        {
                            if (method == '1')
                            {
                                xQueueReset(xLCD_Queue);
                                xQueueReset(xEnc_rotary_Queue);
                                xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                xQueueSendToBack(xLCD_Queue,"00""Cash Chosen",pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                            }
                            else if (method == '2')
                            {
                                xQueueReset(xLCD_Queue);
                                xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                xQueueSendToBack(xLCD_Queue,"00""Card Chosen",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                //lcd wait
                                xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                            }
                        }
                        break;
                    }

                    case '1':
                    {
                        INT8U stop = 0;
                        if (xQueueReceive(xEnc_rotary_Queue, &inserted_money, pdMS_TO_TICKS(QUEUE_MAX_WAIT)) == pdPASS)
                        {
                            char display_money_str[7] = {'\0'};
                            char money[5];
                            current_money = current_money + inserted_money;
                            intToStr(current_money,money);
                            strcat(display_money_str,"10");
                            strcat(display_money_str,money);
                            xQueueSendToBack(xLCD_Queue,display_money_str,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                            xQueueSendToBack(xLCD_Queue,"15""dkk",pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                        }
                        else if (xQueueReceive(xEnc_Button_Queue,&stop,pdMS_TO_TICKS(QUEUE_MAX_WAIT)) == pdPASS)
                        {
                            if (stop == 1)
                            {
                                if (cur_bev.price <= current_money && (cur_bev.idx == '1' || cur_bev.idx == '2'))
                                {
                                    xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                    if (current_money > cur_bev.price)
                                    {
                                        xQueueSendToBack(xLCD_Queue,"00""Returning dkk...",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                        INT16S difference = current_money - cur_bev.price;
                                        blinkReturnAmount( difference );
                                    }
                                    xQueueSendToBack(xLCD_Queue,"10""complete",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                    vTaskDelay(pdMS_TO_TICKS(100));
                                    current_state = PRODUCTION;
                                    inserted_money = 0;
                                    current_money = 0;

                                }
                                else if (cur_bev.idx == '3')
                                {
                                    xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                    current_state = PRODUCTION;
                                }
                                else
                                {
                                    xQueueSendToBack(xLCD_Queue,"00""Insufficient dkk",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                    stop = 0;
                                    vTaskDelay(pdMS_TO_TICKS(200));
                                }
                            }

                        }
                        break;
                    }

                    case '2':
                    {

                        if (xQueueReceive(xKeypad_Queue, &pressed_number, pdMS_TO_TICKS(QUEUE_MAX_WAIT)) == pdPASS)
                        {

                            if (card_idx < 16)
                            {
                                card_numb[card_idx] = pressed_number;
                                card_idx++;
                                char display_card_str[18] = {'\0'};
                                strcat(display_card_str,"00");
                                strcat(display_card_str,card_numb);

                                xQueueSendToBack(xLCD_Queue,display_card_str,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                            }
                            else if (card_idx < 20)
                            {
                                INT8U code_idx = card_idx-16;
                                card_code[code_idx] = pressed_number;
                                card_idx++;
                                char display_code_str[18] = {'\0'};
                                strcat(display_code_str,"10");
                                strcat(display_code_str,card_code);
                                xQueueSendToBack(xLCD_Queue,display_code_str,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                            }

                            if (card_idx == 20)
                            {
                                int last_numb = card_numb[15] - '0';
                                int last_code = card_code[3] - '0';

                                if ((last_numb % 2) == (last_code % 2))
                                {
                                    xQueueSendToBack(xLCD_Queue,"10""valid",pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                    current_state = PRODUCTION;
                                }
                                else
                                {
                                    xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                    xQueueSendToBack(xLCD_Queue,"10""invalid",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                    card_numb[0] = '\0';
                                    card_code[0] = '\0';
                                    card_idx = 0;

                                }
                            }
                        }
                    break;
                    }


                    default:
                    {
                        break;
                    }
                }

                break;
            }

            case PRODUCTION:
            {
                xQueueReset(xSW1_Queue);
                xQueueReset(xSW2_Queue);
                xQueueReset(xEnc_rotary_Queue);
                xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                xQueueSendToBack(xLCD_Queue,"00""Place cup",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                if (xQueueReceive(xSW1_Queue, &cup_present, portMAX_DELAY) == pdPASS && cup_present == 1)
                {
                    xQueueSendToBack(xLCD_Queue,"00""Push SW to Start",pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                    if (xQueueReceive(xSW2_Queue, &start_production, portMAX_DELAY) == pdPASS && start_production == 1)
                    {
                        xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                        switch (cur_bev.idx)
                        {
                            case '1':
                            {
                                //espresso
                                //Grind: 7.5s, yellow led
                                xQueueSendToBack(xLCD_Queue,"00""Grinding... ",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                GPIO_PORTF_DATA_R |= LED_ALL;
                                GPIO_PORTF_DATA_R &= ~LED_Y;
                                vTaskDelay(pdMS_TO_TICKS(7500));


                                //Brew: 14s, red led
                                xQueueSendToBack(xLCD_Queue,"00""Brewing... ",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                GPIO_PORTF_DATA_R |= LED_ALL;
                                GPIO_PORTF_DATA_R &= ~LED_R;
                                vTaskDelay(pdMS_TO_TICKS(14000));


                                xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                GPIO_PORTF_DATA_R |= LED_ALL;
                                while(1)
                                {
                                    xQueueSendToBack(xLCD_Queue,"00""Take the cup",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                    if (xQueueReceive(xSW1_Queue, &cup_taken, portMAX_DELAY) == pdPASS && cup_taken == 1)
                                    {
                                        xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                        xQueueSendToBack(xLCD_Queue,"00""Cup taken",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                        vTaskDelay(1000);
                                        current_state = IDLE;
                                        break;
                                    }
                                }

                                break;
                            }
                            case '2':
                            {
                                //latte
                                //Grind: 7.5s, yellow led
                                xQueueSendToBack(xLCD_Queue,"00""Grinding... ",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                GPIO_PORTF_DATA_R |= LED_ALL;
                                GPIO_PORTF_DATA_R &= ~LED_Y;
                                vTaskDelay(pdMS_TO_TICKS(7500));


                                //Brew: 14s, red led
                                xQueueSendToBack(xLCD_Queue,"00""Brewing... ",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                GPIO_PORTF_DATA_R |= LED_ALL;
                                GPIO_PORTF_DATA_R &= ~LED_R;
                                vTaskDelay(pdMS_TO_TICKS(14000));

                                //froth milk: 6.2s, green led
                                xQueueSendToBack(xLCD_Queue,"00""Frothing milk... ",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                GPIO_PORTF_DATA_R |= LED_ALL;
                                GPIO_PORTF_DATA_R  &= ~LED_G;
                                vTaskDelay(pdMS_TO_TICKS(6200));

                                xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                GPIO_PORTF_DATA_R |= LED_ALL;
                                while(1)
                                {
                                    xQueueSendToBack(xLCD_Queue,"00""Take the cup",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                    if (xQueueReceive(xSW1_Queue, &cup_taken, portMAX_DELAY) == pdPASS && cup_taken == 1)
                                    {
                                        xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                        xQueueSendToBack(xLCD_Queue,"00""Cup taken",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                        vTaskDelay(1000);
                                        current_state = IDLE;
                                        break;
                                    }
                                }

                                break;
                            }
                            case '3':
                            {
                                //filter coffee
                                //LCD: amount, unit price, total price
                                //0.6cl/s for 3s, then 1.45cl/s
                                INT8U inactivity_count = 0;
                                if (method == '1')
                                {
                                    while(1)
                                    {
                                        if (xQueueReceive(xSW2_Queue, &start_production, 0) == pdPASS)
                                        {
                                            if (start_production == 1 )
                                                inactivity_count = 0;
                                        }
                                        else
                                        {
                                            if (start_production == 0 )
                                            {
                                                xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                xQueueSendToBack(xLCD_Queue,"00""Inactive",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                inactivity_count++;
                                                GPIO_PORTF_DATA_R |= LED_Y;
                                                vTaskDelay(pdMS_TO_TICKS(1000));
                                                xQueueSendToBack(xLCD_Queue,CLEAR, pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            }
                                        }

                                        if (inactivity_count >= 5)
                                        {
                                            xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            GPIO_PORTF_DATA_R |= LED_ALL;
                                            while(1)
                                            {
                                                xQueueSendToBack(xLCD_Queue,"00""Take the cup",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                if (xQueueReceive(xSW1_Queue, &cup_taken, portMAX_DELAY) == pdPASS && cup_taken == 1)
                                                {
                                                    xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                    xQueueSendToBack(xLCD_Queue,"00""Cup taken",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                    vTaskDelay(1000);
                                                    current_state = IDLE;
                                                    inactivity_count = 0;
                                                    coffeeTimer = 0;
                                                    amountCoffee = 0;
                                                    break;
                                                }
                                            }
                                            break;
                                        }

                                        if (start_production == 1)
                                        {

                                            GPIO_PORTF_DATA_R &= ~LED_Y;
                                            if (coffeeTimer <= 3000)
                                                amountCoffee += 0.6f/4;
                                            else
                                                amountCoffee += 1.45f/4;
                                            coffeeTimer+=250;

                                            char display_unit_price[7] = {'\0'};
                                            char unit_price[5];
                                            intToStr(cur_bev.price, unit_price);
                                            strcat(display_unit_price, "00");
                                            strcat(display_unit_price, unit_price);
                                            xQueueSendToBack(xLCD_Queue, display_unit_price, pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue, "02""dkk", pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            char display_amount[7] = {'\0'};
                                            char coffeeamount[5];
                                            intToStr(amountCoffee, coffeeamount);
                                            strcat(display_amount, "07");
                                            strcat(display_amount, coffeeamount);
                                            xQueueSendToBack(xLCD_Queue, display_amount, pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue, "09""cl", pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            char display_str[7] = {'\0'};
                                            char money[5];

                                            intToStr((FP32)(amountCoffee * cur_bev.price), money);
                                            strcat(display_str, "10");
                                            strcat(display_str, money);
                                            xQueueSendToBack(xLCD_Queue, display_str, pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue, "15""dkk", pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            vTaskDelay(pdMS_TO_TICKS(250));
                                        }
                                        if (amountCoffee*cur_bev.price >= current_money)

                                        {
                                            xQueueSendToBack(xLCD_Queue,"00""Done - Total:",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            vTaskDelay(pdMS_TO_TICKS(5000));
                                            xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            GPIO_PORTF_DATA_R |= LED_ALL;
                                            while(1)
                                            {
                                                xQueueSendToBack(xLCD_Queue,"00""Take the cup",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                if (xQueueReceive(xSW1_Queue, &cup_taken, portMAX_DELAY) == pdPASS && cup_taken == 1)
                                                {
                                                    xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                    xQueueSendToBack(xLCD_Queue,"00""Cup taken",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                    vTaskDelay(1000);
                                                    current_state = IDLE;
                                                    inactivity_count = 0;
                                                    coffeeTimer = 0;
                                                    amountCoffee = 0;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                else if (method == '2')
                                {
                                    while(1)
                                    {
                                        if (xQueueReceive(xSW2_Queue, &start_production, 0) == pdPASS)
                                        {
                                            if (start_production == 1 )
                                                inactivity_count = 0;
                                        }
                                        else
                                        {
                                            if (start_production == 0 )
                                            {
                                                xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                xQueueSendToBack(xLCD_Queue,"00""Inactive",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                inactivity_count++;
                                                GPIO_PORTF_DATA_R |= LED_Y;
                                                vTaskDelay(pdMS_TO_TICKS(1000));
                                                xQueueSendToBack(xLCD_Queue,CLEAR, pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            }
                                        }

                                        if (inactivity_count >= 5)
                                        {
                                            xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue,"00""Total amount",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue,"10""",pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            char display_str[7] = {'\0'};
                                            char money[5];

                                            intToStr((FP32)(amountCoffee * cur_bev.price), money);
                                            strcat(display_str, "10");
                                            strcat(display_str, money);
                                            xQueueSendToBack(xLCD_Queue, display_str, pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue, "13""dkk", pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            char display_amount[7] = {'\0'};
                                            char coffeeamount[5];
                                            intToStr(amountCoffee, coffeeamount);
                                            strcat(display_amount, "18");
                                            strcat(display_amount, coffeeamount);
                                            xQueueSendToBack(xLCD_Queue, display_amount, pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue, "19""cl", pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            vTaskDelay(pdMS_TO_TICKS(5000));
                                            xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            GPIO_PORTF_DATA_R |= LED_ALL;
                                            while(1)
                                            {
                                                xQueueSendToBack(xLCD_Queue,"00""Take the cup",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                if (xQueueReceive(xSW1_Queue, &cup_taken, portMAX_DELAY) == pdPASS && cup_taken == 1)
                                                {
                                                    xQueueSendToBack(xLCD_Queue,CLEAR,pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                    xQueueSendToBack(xLCD_Queue,"00""Cup taken",pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                                    vTaskDelay(1000);
                                                    current_state = IDLE;
                                                    inactivity_count = 0;
                                                    coffeeTimer = 0;
                                                    amountCoffee = 0;
                                                    break;
                                                }
                                            }
                                            break;
                                        }
                                        if (start_production == 1)
                                        {

                                            GPIO_PORTF_DATA_R &= ~LED_Y;
                                            if (coffeeTimer <= 3000)
                                                amountCoffee += 0.6f/4;
                                            else
                                                amountCoffee += 1.45f/4;
                                            coffeeTimer+=250;

                                            char display_unit_price[7] = {'\0'};
                                            char unit_price[5];
                                            intToStr(cur_bev.price, unit_price);
                                            strcat(display_unit_price, "00");
                                            strcat(display_unit_price, unit_price);
                                            xQueueSendToBack(xLCD_Queue, display_unit_price, pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue, "02""dkk", pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            char display_amount[7] = {'\0'};
                                            char coffeeamount[5];
                                            intToStr(amountCoffee, coffeeamount);
                                            strcat(display_amount, "07");
                                            strcat(display_amount, coffeeamount);
                                            xQueueSendToBack(xLCD_Queue, display_amount, pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue, "09""cl", pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            char display_str[7] = {'\0'};
                                            char money[5];

                                            intToStr((FP32)(amountCoffee * cur_bev.price), money);
                                            strcat(display_str, "10");
                                            strcat(display_str, money);
                                            xQueueSendToBack(xLCD_Queue, display_str, pdMS_TO_TICKS(QUEUE_MAX_WAIT));
                                            xQueueSendToBack(xLCD_Queue, "15""dkk", pdMS_TO_TICKS(QUEUE_MAX_WAIT));

                                            vTaskDelay(pdMS_TO_TICKS(250));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            break;
            }
            default:
            current_state = IDLE;
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(taskDelay));
    }
}
/*


*/
