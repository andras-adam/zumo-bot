#ifndef ESP8266_H_
#define ESP8266_H_
    
int esp_socket(const char *ssid, const char *password) ;
int esp_connect(int sockfd, const char *addr, int port);
int esp_read(int sockfd, void *data, int length);
int esp_write(int sockfd, void *data, int length);
int esp_close(int sockfd);
int esp_shutdown(int sockfd, int how);

#endif