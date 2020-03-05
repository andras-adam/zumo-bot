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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"
#include "serial.h"

#include "esp8266_socket.h"

typedef int EspSocket_t;

static const int EspQueueLength = 32;


#define I_DONT_USE(x) (void) x
#define DEBUGP( ... ) printf( __VA_ARGS__ )

// macro for changing state. Correct operation requires that ctx is a pointer to state machine instance (see state template)
#define TRAN(st) ctx->next_state = st

typedef enum eventTypes { eEnter, eExit, eTick, eReceive, eConnect, eDisconnect, eSend } EventType;

typedef struct event_ {
	EventType ev; // event type (= what happened)
    int length;
	void *data;
    void (*handled)(void *); // pointer for callback
} event;

typedef struct smi_ smi;

typedef void (*smf)(smi *, const event *);  // prototype of state handler function pointer

#define SMI_BUFSIZE 80
#define RC_NOT_AVAILABLE  -1
#define RC_OK     0
#define RC_ERROR  1

struct smi_ {
	smf state;  // current state (function pointer)
	smf next_state; // next state (function pointer)
	SemaphoreHandle_t Access; // interface number
    QueueHandle_t EspEventQ;
    xComPortHandle ComPort;
    int timer;
    int count;
    int pos;
    char buffer[SMI_BUFSIZE];
    char ssid[32]; // SSID
    char pwd[32]; // password
    char sa_data[32]; // ip address
    char sa_port[14]; // port number (string)
};

void stInit(smi *ctx, const event *e);
void stEchoOff(smi *ctx, const event *e);
void stStationModeCheck(smi *ctx, const event *e);
void stStationModeSet(smi *ctx, const event *e);
void stConnectAP(smi *ctx, const event *e);
void stReady(smi *ctx, const event *e);
void stConnectTCP(smi *ctx, const event *e);
void stConnected(smi *ctx, const event *e);
void stSend(smi *ctx, const event *e);
void stReceiveLength(smi *ctx, const event *e);
void stReceiveData(smi *ctx, const event *e);
void stCloseTCP(smi *ctx, const event *e);
void stPassthrough(smi *ctx, const event *e);
void stPassthroughOK(smi *ctx, const event *e);
void stAT(smi *ctx, const event *e);
void stCommandMode(smi *ctx, const event *e);

void vEspSocketTask(void *pvParameters) ;
void vEspEventTask(void *pvParameters) ;
void vEspTickTask( void * pvParameters );


static inline void serial_write_buf(smi *ctx, const char *buf, int len)
{
    while(len-- > 0) {
        xSerialPutChar(ctx->ComPort,*buf++,portMAX_DELAY);
        //UART_1_PutChar(*buf++);
    }
}    

static inline void serial_write_str(smi *ctx, const char *s)
{
    //vSerialPutString(ctx->ComPort, (const signed char *) s, strlen(s));
    while(*s) {
        xSerialPutChar(ctx->ComPort,*s++,portMAX_DELAY);
        //UART_1_PutChar(*s++);
    }
}    

static inline bool serial_get(smi *ctx, char *p)
{
    return xSerialGetChar(ctx->ComPort, (signed char *) p, 10);
}


smi EspSocketInstance;

static void port2str(int i, char *str)
{
    int m = 10000;
    i %= m; // limit integer size to max 5 digits.
    while(i / m == 0) m/=10;
    while(m > 0) {
        *str++ = '0' + i / m;
        i %= m;
        m /= 10;
    }
    *str='\0';
}

void smi_init(smi *ctx) 
{
    memset(ctx, 0, sizeof(smi));
    ctx->state = stInit;
    ctx->next_state = stInit;
}

    
int esp_socket(const char *ssid, const char *password)
{
    smi_init(&EspSocketInstance);
    
    EspSocketInstance.EspEventQ = xQueueCreate( EspQueueLength, sizeof( event ) );
    EspSocketInstance.Access = xSemaphoreCreateMutex();
    EspSocketInstance.ComPort = xSerialPortInitMinimal(115200, 256);
    
    strcpy(EspSocketInstance.ssid, ssid);
    strcpy(EspSocketInstance.pwd, password);
    
    ( void ) xTaskCreate( vEspSocketTask, "ESP_sock", configMINIMAL_STACK_SIZE * 6, &EspSocketInstance, tskIDLE_PRIORITY + 2, NULL );
  	( void ) xTaskCreate( vEspEventTask, "ESP_rxe", configMINIMAL_STACK_SIZE, &EspSocketInstance, tskIDLE_PRIORITY + 2, NULL );
  	( void ) xTaskCreate( vEspTickTask, "ESP_tick", configMINIMAL_STACK_SIZE, &EspSocketInstance, tskIDLE_PRIORITY + 2, NULL );
    
    while(EspSocketInstance.state != stReady) {
        vTaskDelay(50);
    }
   
    return 0;    
}



int esp_connect(int sockfd, const char *addr, int port)
{
    I_DONT_USE(sockfd);

    strcpy(EspSocketInstance.sa_data,addr);
    port2str(port, EspSocketInstance.sa_port);

    const event e = { eConnect, 0, NULL, NULL };
    
    xQueueSendToBack(EspSocketInstance.EspEventQ, &e, portMAX_DELAY);

    int rc = -1;
    if(xSemaphoreTake(EspSocketInstance.Access, configTICK_RATE_HZ * 3) == pdTRUE) {
        xSemaphoreGive(EspSocketInstance.Access);
        rc = 0;
    }
    
    return rc;
}

int esp_read(int sockfd, void *data, int length) 
{
    I_DONT_USE(sockfd);
    int count = 0;
    
    if(xSemaphoreTake(EspSocketInstance.Access, configTICK_RATE_HZ / 10) == pdTRUE) {
        // read data
        while(count < length && serial_get(&EspSocketInstance, data)) {
            ++count;
            ++data;
        }
        xSemaphoreGive(EspSocketInstance.Access);
    }
    
    return count;
}

int esp_write(int sockfd, void *data, int length) 
{
    I_DONT_USE(sockfd);

    if(xSemaphoreTake(EspSocketInstance.Access, configTICK_RATE_HZ / 10) == pdTRUE) {
        // write data
        serial_write_buf(&EspSocketInstance, data, length);
        xSemaphoreGive(EspSocketInstance.Access);
    }
    
    return length;
}

int esp_close(int sockfd)
{
    return esp_shutdown(sockfd, -1);
}

int esp_shutdown(int sockfd, int how)
{
    I_DONT_USE(sockfd);
    I_DONT_USE(how);
    const event e = { eDisconnect, 0, NULL, NULL };
    
    xQueueSendToBack(EspSocketInstance.EspEventQ, &e, portMAX_DELAY);

    return 0;
}


static const event evEnter = { eEnter, 0, NULL, NULL };
static const event evExit = { eExit, 0, NULL, NULL };

#if 0
/* this is state template */
void stStateTemplate(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
		break;
	case eExit:
		break;
	case eTick:
		break;
	case eReceive:
		break;
	default:
		break;
	}
}
#endif

void init_counters(smi *ctx) {
    ctx->count = 0;
    ctx->pos = 0;
    ctx->timer = 0;
}

/* Read and store characters upto specified length. 
 * Returns true when specified amount of characters have been accumulated. */
void sm_flush(smi *ctx) 
{
    //DEBUGP("flush: %d\n", (int)xSerialRxWaiting(ctx->ComPort));
    while(serial_get(ctx, ctx->buffer));
}


/* Read and store characters upto specified length. 
 * Returns true when specified amount of characters have been accumulated. */
bool sm_read_buffer(smi *ctx, int count) 
{
    while(ctx->pos < (SMI_BUFSIZE - 1) && ctx->pos < count && serial_get(ctx, ctx->buffer + ctx->pos)) {
        //putchar(ctx->buffer[ctx->pos]); // debugging
        ++ctx->pos;
    }
    return (ctx->pos >= count);
}


/* Read an integer.
 * Consumes characters until a non-nondigit is received. The nondigit is also consumed. */
bool sm_read_int(smi *ctx, int *value) 
{
    bool result = false;
    while(ctx->pos < (SMI_BUFSIZE - 1) && serial_get(ctx, ctx->buffer + ctx->pos)) {
        if(!isdigit(ctx->buffer[ctx->pos])) {
            ctx->buffer[ctx->pos] = '\0';
            *value = atoi(ctx->buffer);
            result = true;
            break;
        }
        else {
            ++ctx->pos;
        }
    }
    return result;
}



/* Read and store data until one of the specified strings is received.
 * The matched string is also included in the data .*/
int sm_read_until(smi *ctx, const char **p) 
{
    int result = RC_NOT_AVAILABLE;
    while(result < 0 && ctx->pos < (SMI_BUFSIZE - 1) && serial_get(ctx, ctx->buffer + ctx->pos)) {
        ++ctx->pos;
        ctx->buffer[ctx->pos] = '\0';
        for(int i = 0; result < 0 && p[i] != NULL; ++i) {
            if(strstr(ctx->buffer, p[i]) != NULL) {
                result = i;
            }
        }
    }
    return result;
    
}

/* read and store data until result is received */
int sm_read_result(smi *ctx) 
{
    static const char *result_list[] = { "OK\r\n", "ERROR\r\n", NULL };
    return sm_read_until(ctx, result_list);
}

/* read and consume characters until specified string occurs */
bool sm_wait_for(smi *ctx, const char *p)
{
    bool result = false;
    int len = strlen(p);

    while(sm_read_buffer(ctx, len)) {
        ctx->buffer[ctx->pos] = '\0';
        if(strstr(ctx->buffer, p) != NULL) {
            result = true;
            break;
        }
        else {
            memmove(ctx->buffer, ctx->buffer + 1, ctx->pos);
            --ctx->pos;
        }
    }
    return result;
}

void stInit(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stInit\n");
        sm_flush(ctx);
        init_counters(ctx);
        serial_write_str(ctx, "AT\r\n");
		break;
	case eExit:
		break;
	case eTick:
        ++ctx->timer;
        //if(ctx->timer == 5) DEBUGP("[%s]", ctx->buffer);
        if(ctx->timer >= 10) {
            ctx->timer = 0;
            ++ctx->count;
            if(ctx->count < 5) {
                serial_write_str(ctx, "AT\r\n");
            }
            else {
                DEBUGP("Error: Module not responding\n");
                TRAN(stAT);
            }
        }
		break;
    case eReceive:
        if(sm_wait_for(ctx, "OK\r\n")) {
            TRAN(stEchoOff);
        }
        break;
	default:
		break;
	}
}

void stAT(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stAT\n");
        init_counters(ctx);
		break;
	case eExit:
		break;
	case eTick:
        ++ctx->timer;
        if(ctx->timer == 10) serial_write_str(ctx, "+++");
        if(ctx->timer == 25) TRAN(stInit);
		break;
    case eReceive:
        break;
	default:
		break;
	}
}

void stEchoOff(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stEchoOff\n");
        sm_flush(ctx);
        init_counters(ctx);
        serial_write_str(ctx, "ATE0\r\n");
		break;
	case eExit:
		break;
	case eTick:
        ++ctx->timer;
        if(ctx->timer >= 10) {
            ++ctx->count;
            if(ctx->count < 3) {
                serial_write_str(ctx, "ATE0\r\n");
            }
            else {
                DEBUGP("Error: setting local echo off failed\n");
                TRAN(stInit);
            }
        }
		break;
    case eReceive:
        if(sm_wait_for(ctx, "OK\r\n")) {
            TRAN(stStationModeCheck);
        }
        break;
	default:
		break;
	}
}


void stStationModeCheck(smi *ctx, const event *e)
{
    int rc = -1;
	switch(e->ev) {
	case eEnter:
        DEBUGP("stStationModeCheck\n");
        init_counters(ctx);
        serial_write_str(ctx, "AT+CWMODE_CUR?\r\n");
		break;
	case eExit:
		break;
	case eTick:
		break;
    case eReceive:
        rc = sm_read_result(ctx);
        if(rc == RC_OK) {
            DEBUGP("%d: %s", rc, ctx->buffer);
            if(strstr(ctx->buffer, "+CWMODE_CUR:1\r\n") != NULL) {
                TRAN(stConnectAP);
            }
            else {
                TRAN(stStationModeSet);
            }
        }
        else if(rc == RC_ERROR) {
        }
        break;
	default:
		break;
	}
}


void stStationModeSet(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stStationModeSet\n");
        init_counters(ctx);
        serial_write_str(ctx, "AT+CWMODE_CUR=1\r\n");
		break;
	case eExit:
		break;
	case eTick:
		break;
    case eReceive:
        if(sm_wait_for(ctx, "OK\r\n")) {
            TRAN(stStationModeCheck);
        }
        break;
	default:
		break;
	}
}

static void connect_ssid(smi *ctx) 
{
    serial_write_str(ctx, "AT+CWJAP_CUR=\"");
    serial_write_str(ctx, ctx->ssid);
    serial_write_str(ctx, "\",\"");
    serial_write_str(ctx, ctx->pwd);
    serial_write_str(ctx, "\"\r\n");
}

void stConnectAP(smi *ctx, const event *e)
{
    int rc;
	switch(e->ev) {
	case eEnter:
        DEBUGP("stConnectAP\n");
        init_counters(ctx);
		break;
	case eExit:
		break;
	case eTick:
        // may take upto 7 seconds. do we need a timeout?
        ++ctx->timer;
        if(ctx->timer == 1) {
            connect_ssid(ctx);
        }
        if(ctx->timer >= 70) {
            ctx->timer = 0;
        }
		break;
    case eReceive:
        rc = sm_read_result(ctx);
        if(rc == RC_OK) {
            //DEBUGP("%d: %s", rc, ctx->buffer);
            TRAN(stReady);
        }
        else if(rc == RC_ERROR) {
            // failed: what to do now?
        }
        break;
	default:
		break;
	}
}


void stReady(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stReady\n");
        init_counters(ctx);
		break;
	case eExit:
		break;
	case eTick:
#if 0        
        ++ctx->timer;
        if(ctx->timer == 25) {
            DEBUGP("Connecting...\n");
            TRAN(stConnectTCP);
        }
#endif        
		break;
    case eReceive:
        break;
    case eConnect:
        TRAN(stConnectTCP);
        break;
	default:
		break;
	}
    
}



static void connect_tcp(smi *ctx) 
{
    serial_write_str(ctx, "AT+CIPSTART=\"TCP\",\"");
    serial_write_str(ctx, ctx->sa_data);
    serial_write_str(ctx, "\",");
    serial_write_str(ctx, ctx->sa_port);
    serial_write_str(ctx, "\r\n");
}

void stConnectTCP(smi *ctx, const event *e)
{
    int rc;
	switch(e->ev) {
	case eEnter:
        DEBUGP("stConnectTCP\n");
        init_counters(ctx);
        connect_tcp(ctx);
		break;
	case eExit:
		break;
	case eTick:
		break;
    case eReceive:
        rc = sm_read_result(ctx);
        if(rc == RC_OK) {
            //DEBUGP("%d: %s", rc, ctx->buffer);
            if(strstr(ctx->buffer, "CONNECT\r\n") != NULL) {
                TRAN(stPassthrough);
            }
            else {
                // what else can we get with OK??
            }
        }
        else if(rc == RC_ERROR) {
            // failed: what to do now?
            DEBUGP("Connect failed\n");
            connect_tcp(ctx);
        }
        break;
	default:
		break;
	}
}


void stPassthrough(smi *ctx, const event *e)
{
    int rc;
	switch(e->ev) {
	case eEnter:
        DEBUGP("stPassthrough\n");
        init_counters(ctx);
        serial_write_str(ctx, "AT+CIPMODE=1\r\n");
		break;
	case eExit:
		break;
	case eTick:
		break;
    case eReceive:
        rc = sm_read_result(ctx);
        if(rc == RC_OK) {
            TRAN(stPassthroughOK);
        }
        else if(rc == RC_ERROR) {
            // failed: what to do now?
        }
        break;
	default:
		break;
	}
    
}

void stPassthroughOK(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stPassthroughOK\n");
        init_counters(ctx);
        serial_write_str(ctx, "AT+CIPSEND\r\n");
		break;
	case eExit:
		break;
	case eTick:
		break;
    case eReceive:
        if(sm_wait_for(ctx, ">")) {
            TRAN(stConnected);
        }
        break;
	default:
		break;
	}
    
}

#if 1
void stConnected(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stConnected\n");
        init_counters(ctx);
        xSemaphoreGive(ctx->Access);
        break;
	case eExit:
        xSemaphoreTake(ctx->Access, portMAX_DELAY);
		break;
	case eTick:
#if 0        
        ++ctx->timer;
        if(ctx->timer == 50) {
            DEBUGP("Count: %d\n", ctx->count);
            ctx->count = 0;
            serial_write_str(ctx, "get\r\n");
        }
        if(ctx->timer > 50)  {
            ctx->timer = 0;
        }
#endif        
		break;
    case eReceive:
#if 0
        ++ctx->count;
        ctx->pos = 0;
        while(ctx->pos < SMI_BUFSIZE - 1 && serial_get(ctx, ctx->buffer + ctx->pos)) {
            ++ctx->pos;
        }
        if(ctx->pos > 0) {
            ctx->buffer[ctx->pos] = '\0';
            DEBUGP("%s", ctx->buffer);
        }
#endif        
        break;
    case eDisconnect:
        TRAN(stCommandMode);
        break;
	default:
		break;
	}
    
}

void stCommandMode(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stCommandMode\n");
        init_counters(ctx);
		break;
	case eExit:
		break;
	case eTick:
        ++ctx->timer;
        if(ctx->timer == 10) serial_write_str(ctx, "+++");
        if(ctx->timer == 25) TRAN(stCloseTCP);
		break;
    case eReceive:
        break;
	default:
		break;
	}
}


void stCloseTCP(smi *ctx, const event *e)
{
    int rc = -1;
	switch(e->ev) {
	case eEnter:
        DEBUGP("stCloseTCP\n");
        init_counters(ctx);
        serial_write_str(ctx, "AT+CIPMODE=0\r\n");;
        break;
	case eReceive:
        rc = sm_read_result(ctx);
        if(rc == RC_OK) {
            if(strstr(ctx->buffer, "CLOSED") != NULL) {
                TRAN(stReady);
            }
            else {
                init_counters(ctx);
                serial_write_str(ctx, "AT+CIPCLOSE\r\n");;
            }
        }
        else if(rc == RC_ERROR) {
        }
		break;
	case eExit:
		break;
	case eTick:
		break;
	default:
		break;
	}
}




#else

// todo: enter passthough mode for easier data handling
void stConnected(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stConnected\n");
        init_counters(ctx);
		break;
	case eExit:
		break;
	case eTick:
        ++ctx->timer;
        if(ctx->timer == 1) serial_write_str(ctx, "AT+CIPSEND=3\r\n");
        if(ctx->timer == 10) serial_write_str(ctx, "get");
        if(ctx->timer >= 50)  {
            ctx->timer = 0;
        }
		break;
    case eReceive:
        if(sm_wait_for(ctx, "+IPD,")) {
            TRAN(stReceiveLength);
        }
        break;
	default:
		break;
	}
    
}

void stSend(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stSend\n");
        init_counters(ctx);
        //serial_write_str(ctx, "AT+CIPSEND=3\r\n"); // set data length
		break;
	case eExit:
		break;
	case eTick:
		break;
    case eReceive:
        // wait for OK
        // we may possibly receive +IPD ???
        if(sm_wait_for(ctx, "OK\r\n")) {
            TRAN(stConnectTCP);
        }
        break;
	default:
		break;
	}
}



void stReceiveLength(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stReceiveLength\n");
        init_counters(ctx);
		break;
	case eExit:
		break;
	case eTick:
		break;
	case eReceive:
        if(sm_read_int(ctx, &ctx->count)) {
            TRAN(stReceiveData);
        }
		break;
	default:
		break;
	}
}


void stReceiveData(smi *ctx, const event *e)
{
	switch(e->ev) {
	case eEnter:
        DEBUGP("stReceiveData\n");
        ctx->pos = 0;
        ctx->timer = 0;
        // note: count carries data length so it is not zeroed
         break;
	case eReceive:
        if(sm_read_buffer(ctx, ctx->count)) {
            // handle data
            ctx->buffer[ctx->pos] = '\0';
            DEBUGP("%s", ctx->buffer);
            TRAN(stConnected);
        }
		break;
	case eExit:
		break;
	case eTick:
		break;
	default:
		break;
	}
}
#endif


static void dispatch_event(smi *ctx, const event *e)
{
	ctx->state(ctx, e); // dispatch event to current state
	if(ctx->state != ctx->next_state) { // check if state was changed
		ctx->state(ctx, &evExit); // exit old state (cleanup)
		ctx->state = ctx->next_state; // change state
		ctx->state(ctx, &evEnter); // enter new state
	}
}


void EspTimerCallback( TimerHandle_t xTimer )
{
    I_DONT_USE(xTimer);
}

// Perform an action every 100 milliseconds.
void vEspTickTask( void * pvParameters )
{
    QueueHandle_t eq = ((smi *) pvParameters)->EspEventQ;
    const event e = { eTick, 0, NULL, NULL };    
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = configTICK_RATE_HZ / 10;

    // Initialise the xLastWakeTime variable with the current time.
    xLastWakeTime = xTaskGetTickCount();

    for( ;; )
    {
        // Wait for the next cycle.
        vTaskDelayUntil( &xLastWakeTime, xFrequency );

        // Perform action here.
        xQueueSendToBack(eq, &e, 0);
    }
}

void vEspEventTask(void *pvParameters) {
    QueueHandle_t eq = ((smi *) pvParameters)->EspEventQ;
    xComPortHandle port = ((smi *) pvParameters)->ComPort;
    // todo: fix serial port stuff
    // todo: use direct to task notification instead of binary semaphore??
    const event e = { eReceive, 0, NULL, NULL };
    
    // reset ESP-01
    RST_E_Write(0);
    vTaskDelay(20); 
    RST_E_Write(1);
    
    // todo: proper synchronized start
    vTaskDelay(20); 
    
    while(true) {
#if 1
        if(xSerialWaitForRxEvent( port, 50 ) == pdTRUE) {
            xQueueSendToBack(eq, &e, 0);
        }
        else {
             // if nothing was received but there is data in the buffers then send received data notification
            if(xSerialRxWaiting(port) > 0) {
                xQueueSendToBack(eq, &e, 0);
            }
            else {
            }                
        }
#else
            if(xSerialRxWaiting(port) > 0) {
                xQueueSendToBack(eq, &e, 0);
            }
            vTaskDelay(3);    
#endif    
    }
}

/**
 * Receive events from queue and dispatch them to state machine
 */
void vEspSocketTask(void *pvParameters) {
    smi *ctx = (smi *) pvParameters;
    
	event e;

    xSemaphoreTake(ctx->Access, portMAX_DELAY);
    
	dispatch_event(ctx, &evEnter); // enter initial state

	while (1) {
		// read queue
		xQueueReceive(ctx->EspEventQ, &e, portMAX_DELAY);
		dispatch_event(ctx, &e); // dispatch event to current state
	}
}



/* [] END OF FILE */
