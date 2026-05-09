#ifndef RTOS_DEF_H_
#define RTOS_DEF_H_

// RTOS Defines

    // Tasks
        #define USERTASK_STACK_SIZE 128

        #define IDLE_PRIO 0
        #define LOW_PRIO  1
        #define MED_PRIO  2
        #define HIGH_PRIO 3


    // Queues
        // Default
            #define QUEUE_LENGTH 20
            #define QUEUE_ITEMSIZE 10 //bytes
            #define QUEUE_MAX_WAIT 5 //ms

        // LCD QUEUE
            #define LCD_POS_TO_SEND 2 // ex. 14 = row 1 (buttom row) col 4
            #define LCD_CHARS_TO_SEND 16

        // Coffe Task QUEUE
            


#endif /* RTOS_DEF_H_ */
