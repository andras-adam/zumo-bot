#ifndef DEBUG_UART_H_
#define DEBUG_UART_H_

void DebugUartTask( void *pvParameters );
void DebugUartTaskInit(void);
void DebugCommandTask( void *pvParameters );

#endif