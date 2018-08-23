#ifndef MQTT_SENDER_H_
#define MQTT_SENDER_H_
    
void send_mqtt(const char *topic, const char *message);
void MQTTSendTask(void *pvParameters);
void MQTTSendTaskInit(void);
#define DEBUGM(topic, message, ... ) send_mqtt(topic, (sprintf( message,  __VA_ARGS__ ), message) )

    
    
#endif    