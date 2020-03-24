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
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "serial.h"
/*---------------------------------------------------------------------------*/

#define serialSTRING_DELAY_TICKS		( portMAX_DELAY )
/*---------------------------------------------------------------------------*/

CY_ISR_PROTO( vUartRxISR );
CY_ISR_PROTO( vUartTxISR );
/*---------------------------------------------------------------------------*/

static QueueHandle_t xSerialTxQueue = NULL;
static QueueHandle_t xSerialRxQueue = NULL;
static SemaphoreHandle_t xSerialRxEvent = NULL;
/*---------------------------------------------------------------------------*/

xComPortHandle xSerialPortInitMinimal( unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength )
{
    (void) ulWantedBaud;
    /* Create rx event semaphore */
    xSerialRxEvent = xSemaphoreCreateBinary();
    
    /* seems that occasionally receiver gets in a weird state if received data is available when interrupts are enabled */
    while(UART_2_ReadRxStatus() & UART_2_RX_STS_FIFO_NOTEMPTY) UART_2_ReadRxData();

    /* Configure Rx. */
	xSerialRxQueue = xQueueCreate( uxQueueLength, sizeof( signed char ) );	
	isr_U2_RX_ClearPending();
	isr_U2_RX_StartEx(vUartRxISR);

	/* Configure Tx */
#if 1    
	xSerialTxQueue = xQueueCreate( uxQueueLength, sizeof( signed char ) );
	isr_U2_TX_ClearPending() ;
	isr_U2_TX_StartEx(vUartTxISR);
    
	/* Clear the interrupt modes for the Tx for the time being. */
	UART_2_SetTxInterruptMode( 0 );
#endif
	/* Both configured successfully. */
	return ( xComPortHandle )( xSerialTxQueue && xSerialRxQueue );
}
/*---------------------------------------------------------------------------*/

void vSerialPutString( xComPortHandle pxPort, const signed char * const pcString, unsigned short usStringLength )
{
    (void) pxPort;
    unsigned short usIndex = 0;

	for( usIndex = 0; usIndex < usStringLength; usIndex++ )
	{
		/* Check for pre-mature end of line. */
		if( '\0' == pcString[ usIndex ] )
		{
			break;
		}
		
		/* Send out, one character at a time. */
		if( pdTRUE != xSerialPutChar( NULL, pcString[ usIndex ], serialSTRING_DELAY_TICKS ) )
		{
			/* Failed to send, this will be picked up in the receive comtest task. */
		}
	}
}
/*---------------------------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar( xComPortHandle pxPort, signed char *pcRxedChar, TickType_t xBlockTime )
{
    (void) pxPort;
    portBASE_TYPE xReturn = pdFALSE;

	if( pdTRUE == xQueueReceive( xSerialRxQueue, pcRxedChar, xBlockTime ) )
	{
		/* Picked up a character. */
		xReturn = pdTRUE;
	}
	return xReturn;
}
/*---------------------------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar( xComPortHandle pxPort, signed char cOutChar, TickType_t xBlockTime )
{
    (void) pxPort;
    portBASE_TYPE xReturn = pdFALSE;

	/* The ISR is processing characters is so just add to the end of the queue. */
	if( pdTRUE == xQueueSend( xSerialTxQueue, &cOutChar, xBlockTime ) )
	{	
		xReturn = pdTRUE;
	}
	else
	{
		/* The queue is probably full. */
		xReturn = pdFALSE;
	}

	/* Make sure that the interrupt will fire in the case where:
	    Currently sending so the Tx Complete will fire.
	    Not sending so the Empty will fire.	*/
	taskENTER_CRITICAL();
		UART_2_SetTxInterruptMode( UART_2_TX_STS_COMPLETE | UART_2_TX_STS_FIFO_EMPTY );
	taskEXIT_CRITICAL();
	
	return xReturn;
}
/*---------------------------------------------------------------------------*/

signed portBASE_TYPE xSerialWaitForRxEvent( xComPortHandle pxPort, TickType_t xBlockTime )
{
    (void) pxPort;
    portBASE_TYPE xReturn = pdFALSE;

	if( pdTRUE == xSemaphoreTake( xSerialRxEvent, xBlockTime ) )
	{
		/* Got an event. */
		xReturn = pdTRUE;
	}
	return xReturn;
}

signed portBASE_TYPE xSerialRxWaiting(  xComPortHandle pxPort ) 
{
    (void) pxPort;
    return uxQueueMessagesWaiting(xSerialRxQueue);
}

//void dc(char c); // for debugging

CY_ISR(vUartRxISR)
{
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
volatile unsigned char ucStatus = 0;
signed char cInChar = 0;

	/* Read the status to acknowledge. */
	ucStatus = UART_2_ReadRxStatus();

	/* Only interested in a character being received. */
	if( 0 != ( ucStatus & UART_2_RX_STS_FIFO_NOTEMPTY ) )
	{
		/* Get the character. */
		cInChar = UART_2_ReadRxData();
		/* Try to deliver the character. */
		if( pdTRUE != xQueueSendFromISR( xSerialRxQueue, &cInChar, &xHigherPriorityTaskWoken ) )
		{
			/* Run out of space. */
		}
        xSemaphoreGiveFromISR( xSerialRxEvent, &xHigherPriorityTaskWoken); // signal receive event
	}

	/* If we delivered the character then a context switch might be required.
	xHigherPriorityTaskWoken was set to pdFALSE on interrupt entry.  If calling 
	xQueueSendFromISR() caused a task to unblock, and the unblocked task has
	a priority equal to or higher than the currently running task (the task this
	ISR interrupted), then xHigherPriorityTaskWoken will have been set to pdTRUE and
	portEND_SWITCHING_ISR() will request a context switch to the newly unblocked
	task. */
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
/*---------------------------------------------------------------------------*/

CY_ISR(vUartTxISR)
{
BaseType_t xHigherPriorityTaskWoken = pdFALSE;
volatile unsigned char ucStatus = 0;
signed char cOutChar = 0;

	/* Read the status to acknowledge. */
	ucStatus = UART_2_ReadTxStatus();
	
	/* Check to see whether this is a genuine interrupt. */
	if( 0 != ( ucStatus & UART_2_TX_STS_FIFO_EMPTY ) )
	{	
		do {
			if( pdTRUE == xQueueReceiveFromISR( xSerialTxQueue, &cOutChar, &xHigherPriorityTaskWoken ) )
			{
				/* Send the next character. */
				UART_2_PutChar( cOutChar );			
			}
			else
			{
				/* There is no work left so disable the interrupt until the application 
				puts more into the queue. */
				UART_2_SetTxInterruptMode( 0 );
                break;
			}
		}while((UART_2_ReadTxStatus() & UART_2_TX_STS_FIFO_FULL) == 0);
	}

	/* If we delivered the character then a context switch might be required.
	xHigherPriorityTaskWoken was set to pdFALSE on interrupt entry.  If calling 
	xQueueSendFromISR() caused a task to unblock, and the unblocked task has
	a priority equal to or higher than the currently running task (the task this
	ISR interrupted), then xHigherPriorityTaskWoken will have been set to pdTRUE and
	portEND_SWITCHING_ISR() will request a context switch to the newly unblocked
	task. */
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
/*---------------------------------------------------------------------------*/
