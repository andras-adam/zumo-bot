/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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

#include <project.h>
#include <stdio.h>

/* RTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "serial.h"
#include "serial1.h"
#include "debug_uart.h"
#include "mqtt_sender.h"
#include "simulator.h"
#include "zumo_config.h"

#ifndef START_MQTT
#define START_MQTT 0
#endif    



extern void zmain(void);

/*
 * Installs the RTOS interrupt handlers and starts the peripherals.
 */
static void prvHardwareSetup( void );
/*---------------------------------------------------------------------------*/

static void start_zmain(void *p) {
    (void) p; // we don't use this parameter
    zmain();
    printf("\n\n\nERROR - ZMAIN ENDED!!!\n\n");
    vTaskSuspend(NULL);
}

int main( void )
{
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
	prvHardwareSetup();

    
    /* Task initializations */
    vSerial1PortInitMinimal(256);
    RetargetInit();
    
    //DebugUartTaskInit();
  	//( void ) xTaskCreate( DebugUartTask, "DbgUart", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL );
  	//( void ) xTaskCreate( DebugCommandTask, "DbgCmd", configMINIMAL_STACK_SIZE * 3, NULL, tskIDLE_PRIORITY + 1, NULL );
    
  	( void ) xTaskCreate( start_zmain, "Zumo", configMINIMAL_STACK_SIZE * 10, NULL, tskIDLE_PRIORITY + 1, NULL );
#if START_MQTT == 1
    MQTTSendTaskInit();
  	( void ) xTaskCreate( MQTTSendTask, "MQTT_send", configMINIMAL_STACK_SIZE * 10, NULL, tskIDLE_PRIORITY + 2, NULL );
#endif    
#if ZUMO_SIMULATOR == 1  
    SimulatorTaskInit();
  	( void ) xTaskCreate( SimulatorTask, "Simulator", configMINIMAL_STACK_SIZE * 10, NULL, tskIDLE_PRIORITY + 2, NULL );
#endif    
    
	/* Will only get here if there was insufficient memory to create the idle
    task.  The idle task is created within vTaskStartScheduler(). */
	vTaskStartScheduler();

	/* Should never reach here as the kernel will now be running.  If
	vTaskStartScheduler() does return then it is very likely that there was
	insufficient (FreeRTOS) heap space available to create all the tasks,
	including the idle task that is created within vTaskStartScheduler() itself. */
	for( ;; );
}
/*---------------------------------------------------------------------------*/


#if 0
void __malloc_lock (struct _reent *reent) {
    (void) reent;
    vTaskSuspendAll();
}

void __malloc_unlock (struct _reent *reent) {
    (void) reent;
    xTaskResumeAll();
}
#endif

void prvHardwareSetup( void )
{
/* Port layer functions that need to be copied into the vector table. */
extern void xPortPendSVHandler( void );
extern void xPortSysTickHandler( void );
extern void vPortSVCHandler( void );
extern cyisraddress CyRamVectors[];

	/* Install the OS Interrupt Handlers. */
	CyRamVectors[ 11 ] = ( cyisraddress ) vPortSVCHandler;
	CyRamVectors[ 14 ] = ( cyisraddress ) xPortPendSVHandler;
	CyRamVectors[ 15 ] = ( cyisraddress ) xPortSysTickHandler;

	/* Start-up the peripherals. */
	UART_1_Start();
    UART_2_Start();

}
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
    (void) pxTask;
    (void) pcTaskName;
	/* The stack space has been execeeded for a task, considering allocating more. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*---------------------------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* The heap space has been execeeded. */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}
/*---------------------------------------------------------------------------*/
