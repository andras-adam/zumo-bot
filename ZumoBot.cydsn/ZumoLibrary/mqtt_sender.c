/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "MQTTFreeRTOS.h"
#include "MQTTClient.h"
#include "mqtt_sender.h"
#include "zumo_config.h"



typedef struct mqtt_message_ {
    const char *topic;
    char *message;
    void (*free)(void *);
} mqtt_message_t;

#define MAX_MESSAGE 32
#define BUFFER_SIZE 80
static QueueHandle_t msg_q;
static QueueHandle_t buf_q;
static char buffers[MAX_MESSAGE][BUFFER_SIZE];

void MQTTSendTaskInit(void)
{
    msg_q = xQueueCreate(MAX_MESSAGE, sizeof(mqtt_message_t));
    buf_q = xQueueCreate(MAX_MESSAGE, sizeof(char *));
}

#if ZUMO_SIMULATOR == 0
void MQTTSendTask(void *pvParameters)
{
    (void) pvParameters;
	MQTTClient client;
	Network network;
	unsigned char sendbuf[128], readbuf[128];
	int rc = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;
    
    for(int i = 0; i < MAX_MESSAGE; ++i) {
        char *p = buffers[i];
        xQueueSend(buf_q, &p, 0);
    }

	NetworkInit(&network, NETWORK_SSID, NETWORK_PASSWORD);
    
	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	char* address = MQTT_BROKER;
	if ((rc = NetworkConnect(&network, address, 1883)) != 0)
		printf("Return code from network connect is %d\n", rc);

#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&client)) != pdPASS)
		printf("Return code from start tasks is %d\n", rc);
#else
    #error "MQTT_TASK not defined"
#endif

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = MQTT_CLIENT_ID;

	if ((rc = MQTTConnect(&client, &connectData)) != 0)
		printf("Return code from MQTT connect is %d\n", rc);
	else
		printf("MQTT Connected\n");

    while(true)
	{
        mqtt_message_t msg;
        
        if(xQueueReceive(msg_q, &msg, portMAX_DELAY) == pdFALSE) {
            // this should never happen
            vTaskDelay(configTICK_RATE_HZ);
            continue;
        }
        
		MQTTMessage message;

		message.qos = QOS0;
		message.retained = 0;
		message.payload = (void *) msg.message;
		message.payloadlen = strlen(msg.message);

		if ((rc = MQTTPublish(&client, msg.topic, &message)) != 0) {
			printf("Return code from MQTT publish is %d\n", rc);
        }
        if(msg.free != NULL) {
            msg.free(msg.message);
        }
	}

	/* do not return */
}
#else
void MQTTSendTask(void *pvParameters)
{
    (void) pvParameters;
    mqtt_message_t msg;
    
    for(int i = 0; i < MAX_MESSAGE; ++i) {
        char *p = buffers[i];
        xQueueSend(buf_q, &p, 0);
    }

    while(true)
	{
        if(xQueueReceive(msg_q, &msg, portMAX_DELAY) == pdFALSE) {
            // this should never happen
            vTaskDelay(configTICK_RATE_HZ);
        }
        
        /* print message here */
        fputs("MQTT ", stdout);
        fputs(msg.topic, stdout);
        fputs(" ", stdout);
        fputs(msg.message, stdout);
        fputs("\n", stdout);
        
        if(msg.free != NULL) {
            msg.free(msg.message);
        }
	}

	/* do not return */
}
#endif    

#if START_MQTT == 1
static void free_buffer(void *buffer) 
{
    xQueueSend(buf_q, &buffer, 0);
}    
#endif

int print_mqtt(const char *topic, const char *fmt, ...)
{
#if START_MQTT == 1
    va_list argptr;
    int cnt=0;
    char pbuf[BUFFER_SIZE];

    va_start(argptr, fmt);
    cnt = vsnprintf(pbuf, BUFFER_SIZE, fmt, argptr);
    va_end(argptr);

    send_mqtt(topic, pbuf);

    return(cnt);
#else
    (void) topic;
    (void) fmt;
    return 0;
#endif    
}

void send_mqtt(const char *topic, const char *message)
{
#if START_MQTT == 1
    mqtt_message_t msg;
    msg.topic = topic;
    if(xQueueReceive(buf_q, &msg.message, configTICK_RATE_HZ / 10) == pdFALSE) {
        return;
    }
    strncpy(msg.message, message, BUFFER_SIZE - 1);
    msg.message[BUFFER_SIZE - 1] = '\0';
    msg.free = free_buffer;
    if(xQueueSend(msg_q, &msg, configTICK_RATE_HZ / 10) == pdFALSE) {
        free_buffer(msg.message);
    }
#else
    (void) topic;
    (void) message;
#endif    
  
}

