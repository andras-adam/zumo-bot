/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include <stdio.h>
#include <stdbool.h>
/* RTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "debug_uart.h"
#include "zumo_config.h"
#include "serial1.h"

#include "esp8266_socket.h"

static int process_command(char *cmd);
static int cmd_help(char *arg);
static int cmd_socket(char *cmd);
static int cmd_connect(char *cmd);
static int cmd_close(char *cmd);
static int cmd_read(char *cmd);
static int cmd_write(char *cmd);
static int cmd_tasklist(char *cmd);
static int cmd_stats(char *cmd);

static const struct cmdtable_ {
  char *name;
  int (*cmd)(char *);
  const char *help;
} cmdtable[] = {
  { "?",     cmd_help, "help [command]\n" },
  { "help",  cmd_help, "help [command]\n" },
  { "socket",  cmd_socket, "socket - starts ESP8266 socket task\n" },
  { "connect",  cmd_connect, "connect - connect to a server\n" },
  { "close",  cmd_close, "close - close connection\n" },
  { "read",  cmd_read, "read from socket\n" },
  { "write",  cmd_write, "write <data> - writes <data> to socket\n" },
  { "list",  cmd_tasklist, "list - list currently running tasks\n" },
  { "stat",  cmd_stats, "stat - print runtime statistics\n" },

  { "", NULL, "" } /* terminating entry - do not remove */
};


static QueueHandle_t xSerialTxQueue = NULL;
static const int uxQueueLength = 256;
static SemaphoreHandle_t u1_mutex = NULL;

void DebugUartTask( void *pvParameters )
{
    (void) pvParameters; //quiet the compiler warning
    char c;
    //while(1) vTaskDelay(100);
    
    while(true) {
    	if( pdTRUE == xQueueReceive( xSerialTxQueue, &c, portMAX_DELAY ) )
    	{
    		/* Picked up a character. */
            xSemaphoreTake(u1_mutex, portMAX_DELAY);
            UART_1_PutChar(c);
            xSemaphoreGive(u1_mutex);
    	}
    }
    
}


void DebugUartTaskInit(void)
{
	/* Configure Tx */
	xSerialTxQueue = xQueueCreate( uxQueueLength, sizeof( signed char ) );
    u1_mutex = xSemaphoreCreateMutex();
}    

#if 0
int _write(int file, const char *ptr, int len)
{
    (void)file; /* Parameter is not used, suppress unused argument warning */
	int n;
	for(n = 0; n < len; n++) {
        if(*ptr == '\n') {
            const char cr = '\r';
    		xQueueSend( xSerialTxQueue, &cr, portMAX_DELAY); //UART_1_PutChar('\r');
        }
		xQueueSend( xSerialTxQueue, ptr++, portMAX_DELAY); //UART_1_PutChar(*ptr++);
	}
	return len;

}
#endif

void ds(const char *s) 
{
	while(*s) {
        if(*s == '\n') {
            const char cr = '\r';
    		xQueueSend( xSerialTxQueue, &cr, portMAX_DELAY); //UART_1_PutChar('\r');
        }
		xQueueSend( xSerialTxQueue, s++, portMAX_DELAY); //UART_1_PutChar(*ptr++);
	}
}

#define CMD_MAX_SIZE 64
void DebugCommandTask( void *pvParameters )
{
    (void) pvParameters; //quiet the compiler warning
    char c;
    char cmd[CMD_MAX_SIZE];
    int pos = 0;

    while(true) {
        xSerial1GetChar(&c,portMAX_DELAY);
        if(c != 0) {
            if(c == '\n' || c == '\r') {
                if(pos > 0) {
		            xQueueSend( xSerialTxQueue, &c, portMAX_DELAY);
                    pos = 0;
                    process_command(cmd);
                }
            }
            else if(pos < CMD_MAX_SIZE - 1) {
                cmd[pos++] = c;
                cmd[pos] = 0;
		        xQueueSend( xSerialTxQueue, &c, portMAX_DELAY);
            }
        }
        else {
            vTaskDelay(1);
        }
    }
    
}

static int cmd_socket(char *cmd)
{
    (void) cmd;
    
    esp_socket(NETWORK_SSID, NETWORK_PASSWORD);
    
    return 0;
}

static int cmd_connect(char *cmd)
{
    (void) cmd;
    
    esp_connect(0, MQTT_BROKER, 5000); // connect to echo server
    
    return 0;
}

static int cmd_close(char *cmd)
{
    (void) cmd;
    
    esp_close(0);
    
    return 0;
}

static int cmd_read(char *cmd)
{
    (void) cmd;
    static char buffer[64];
    
    int count = esp_read(0, buffer, 63);
    if(count > 0) {
        buffer[count] = '\0';
        ds(buffer);
    }
    
    return 0;    
}

static int cmd_write(char *cmd)
{
    esp_write(0, cmd, strlen(cmd));
    return 0;    
}

static char prt_buffer[1024];

static int cmd_tasklist(char *cmd)
{
    (void) cmd;

    vTaskList(prt_buffer);
    ds(prt_buffer);
    
    return 0;
}

static int cmd_stats(char *cmd)
{
    (void) cmd;

    vTaskGetRunTimeStats(prt_buffer);
    ds(prt_buffer);
    
    return 0;
}

static int process_command(char *cmd)
{
  int i;
  char *args, *cmdend;

  /* skip white space in the beginning */
  while((*cmd != '\0') && (*cmd == ' ')) {
    cmd++;
  }
  cmdend = cmd;
  /* skip command */
  while((*cmdend != '\0') && (*cmdend != ' ')) {
    cmdend++;
  }
  args = cmdend;
  if(*args != '\0') {
    /* skip white space */
    while((*args != '\0') && (*args == ' ')) {
      args++;
    }
  }
  *cmdend = '\0';

  if(cmd[0] == '\0') return 0;

  for(i=0; cmdtable[i].cmd != NULL; i++) {
    int k;
    for(k = 0; cmd[k] != '\0' && cmdtable[i].name[k] != '\0'; k++) {
      if(cmdtable[i].name[k] != cmd[k]) break;
    }
    if(cmd[k] == '\0' && cmdtable[i].name[k] == '\0') {
      return (*cmdtable[i].cmd)(args);
    }
  }

  return 0;
}




static void print_command_list(void)
{
  int i;

  ds("commands: \n");
  for(i=0; cmdtable[i].cmd != NULL; i++) {
    ds(cmdtable[i].name);
    if((i>0) && (i%5==0)) ds("\n");
    else ds(" ");
    vTaskDelay(5);
  }
  ds("\n");
}

static int cmd_help(char *arg)
{
  int i;

  /* check argument */
  for(i=0; arg[i] != '\0' && arg[i] != ' '; i++);
  if(i != 0) {
    arg[i] = '\0';
    for(i=0; cmdtable[i].cmd != NULL; i++) {
      int k;
      for(k = 0; arg[k] != '\0' && cmdtable[i].name[k] != '\0'; k++) {
    	if(cmdtable[i].name[k] != arg[k]) break;
      }
      if(arg[k] == '\0' && cmdtable[i].name[k] == '\0') {
	    ds(cmdtable[i].help);
	    ds("\n");
      }
    }
  }
  else {
    ds("help [command]\n");
    print_command_list();
  }

  return 0;
}


/* [] END OF FILE */
