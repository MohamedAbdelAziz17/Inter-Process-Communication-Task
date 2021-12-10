/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "lpc21xx.h"
#include "semphr.h"
#include "queue.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )


/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/
/*Global Variables*/
QueueHandle_t xQueue;		

	char txString[6] = "";

	char txEvent_1 = 0;

	char txEvent_2 = 0;


/* Task to be created. */
void Read_Button1_Task( void * pvParameters )
{
				
	pinState_t button_1_state = PIN_IS_LOW; 

    for( ;; )
    {
			 /* Task code goes here. */
			/*	send event of a button 1	*/
			button_1_state = GPIO_read(PORT_0 , PIN0);
			
			if(button_1_state == 1)
			{
				txEvent_1 = 1;
				
				xQueueSend( xQueue,
                 ( void * ) &txEvent_1,
                   portMAX_DELAY);
			}
			
			else
			{
				txEvent_1 = 0;
				
				xQueueSend( xQueue,
                 ( void * ) &txEvent_1 ,
                   portMAX_DELAY);
			}
					
			vTaskDelay(20);
		}
}

void Read_Button2_Task( void * pvParameters )
{
			pinState_t button_2_state = PIN_IS_LOW; 
			
    for( ;; )
    {
        /* Task code goes here. */
			/*	send event of a button 2	*/
			button_2_state = GPIO_read(PORT_0 , PIN1);
			
			if(button_2_state == 1)
			{
				txEvent_2 = 1;
				
				xQueueSend( xQueue,
                 ( void * ) &txEvent_2,
                   portMAX_DELAY);
			}
			
			else
			{
				txEvent_2 = 0;
				
				xQueueSend( xQueue,
                 ( void * ) &txEvent_2 ,
                   portMAX_DELAY);
			}
			
			vTaskDelay(20);
	
		}
}

void Periodic_SendString_Task( void * pvParameters )
{
		 char data[6] = "STRING";
	
		strcpy(txString, data);

    for( ;; )
    {			
        /* Task code goes here. */
			/*	send this string every 100ms	*/
				xQueueSend( xQueue,
                 ( void * ) &txString,
                   portMAX_DELAY);
					
			vTaskDelay(100);
	
		}
}

void UART_Consumer_Task( void * pvParameters )
{
		/* copy of recieved data from xQueue */
		char rxString;  	
	  char rxEvent1;
		char rxEvent2;

    for( ;; )
    {
        /* Task code goes here. */
			/*	send string to UART 	*/
			if( xQueueReceive( xQueue,
                         &( rxString ),
                         portMAX_DELAY == pdTRUE ) )
			{
				vSerialPutString((signed char*)(txString) ,8);
			}
			
			/*	send event 2 to UART 	*/
			if( xQueueReceive( xQueue,
                         &( rxEvent2 ),
                         portMAX_DELAY == pdTRUE ) )
			{
				xSerialPutChar(rxEvent2);
			}
			
			/*	send event 1 to UART	*/
			if( xQueueReceive( xQueue,
                         &( rxEvent1 ),
                         portMAX_DELAY == pdTRUE ) )
			{
					xSerialPutChar(rxEvent1);
			}
			
			
		}
}

		
		
/* Handlers	*/
TaskHandle_t Read_Button1_Task_Handler				  = NULL;
TaskHandle_t Read_Button2_Task_Handler 					= NULL;
TaskHandle_t Periodic_SendString_Task_Handler 	= NULL;
TaskHandle_t UART_Consumer_Task_Handler 				= NULL;


/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();
	
	xQueue =	xQueueCreate(8, sizeof( char ) );
	  
    /* Create Tasks here */

				xTaskCreate(
                    Read_Button1_Task,       /* Function that implements the task. */
                    "Read_Button1_Task",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    2,/* Priority at which the task is created. */
                    &Read_Button1_Task_Handler 
										);      /* Used to pass out the created task's handle. */

				xTaskCreate(
                    Read_Button2_Task,       /* Function that implements the task. */
                    "Read_Button2_Task",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    2,/* Priority at which the task is created. */
                    &Read_Button2_Task_Handler 
										);      /* Used to pass out the created task's handle. */
										
				xTaskCreate(
                    Periodic_SendString_Task,       /* Function that implements the task. */
                    "Task_1",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    3,/* Priority at which the task is created. */
                    &Periodic_SendString_Task_Handler 
										);      /* Used to pass out the created task's handle. */
				xTaskCreate(
                    UART_Consumer_Task,       /* Function that implements the task. */
                    "Task_1",          /* Text name for the task. */
                    100,      /* Stack size in words, not bytes. */
                    ( void * ) 0,    /* Parameter passed into the task. */
                    1,/* Priority at which the task is created. */
                    &UART_Consumer_Task_Handler 
										);      /* Used to pass out the created task's handle. */
																
	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


