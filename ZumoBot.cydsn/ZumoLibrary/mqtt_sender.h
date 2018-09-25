#ifndef MQTT_SENDER_H_
#define MQTT_SENDER_H_
    
void send_mqtt(const char *topic, const char *message);
void MQTTSendTask(void *pvParameters);
void MQTTSendTaskInit(void);
int print_mqtt(const char *topic, const char *fmt, ...);
    
    
#endif    