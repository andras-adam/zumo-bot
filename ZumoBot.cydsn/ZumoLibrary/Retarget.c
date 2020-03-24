#include "zumo_config.h"

#if ZUMO_SIMULATOR == 0   

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "project.h"
#include "serial1.h"

static xSemaphoreHandle readMutex;

void RetargetInit(void)
{
    readMutex = xSemaphoreCreateMutex();;
}


/* Don't remove the functions below */
int _write(int file, char *ptr, int len)
{
    (void)file; /* Parameter is not used, suppress unused argument warning */
	int n;
	for(n = 0; n < len; n++) {
        if(*ptr == '\n') xSerial1PutChar('\r', portMAX_DELAY);
		xSerial1PutChar(*ptr++, portMAX_DELAY);
	}
	return len;
}

#if 1
int _read (int file, char *ptr, int count)
{
    int chs = 0;
    char ch;
    static char buf[128];
    static unsigned int cnt = 0;
    static unsigned int head = 0;
    static unsigned int tail = 0;
    static unsigned int lf = 0;
 
    (void)file; /* Parameter is not used, suppress unused argument warning */
    
    /* When _impure_ptr is switched on per task basis then scanf doesn't automatically flush buffers. 
     * This needs to be investigated further - may require digging into newlib/newlibnano sources 
    */
    fflush(stdout); /* krl: kludge to flush buffers on read - see note above */
    
    if(xSemaphoreTake(readMutex, portMAX_DELAY) == pdFALSE) return 0;
    
    while(count > 0) {
        //ch = UART_1_GetChar();
        if(xSerial1GetChar(&ch, 10)) {
            xSerial1PutChar(ch, portMAX_DELAY);
            if(ch == '\r') { /* convert CR to LF for processing, putty sends CR when Enter is pressed */
                ch = '\n';
                xSerial1PutChar(ch, portMAX_DELAY);
            }
            if(ch == 127) { /* backspace handling */
                if(cnt > 0) {
                    cnt--;
                    head = (head - 1) & 0x7F;
                }
            }
            else { /* other characters: store and check for LF */
                buf[head] = ch;
                head = (head + 1) & 0x7F;
                cnt++;
                if(ch == '\n' || cnt >= 128) lf++;
            }
        }
        if(lf > 0) {
            if(cnt > 0) {
                if(buf[tail] == '\n') {
                    lf--;
                    count = 0;
                }
                *ptr++ = buf[tail];
                tail = (tail + 1) & 0x7F;
                cnt--;
                chs++;
                count--;
            }
            else {
                lf = 0;
            }
        }
    }
    
    xSemaphoreGive(readMutex);
    
    return chs;
}
#else
int _read (int file, char *ptr, int count)
{
    char ch;
    int chs = 0;
 
    (void)file; /* Parameter is not used, suppress unused argument warning */
    while(count > 0) {
        //ch = UART_1_GetChar();
        if(xSerial1GetChar(&ch, 10)) {
            xSerial1PutChar(ch, portMAX_DELAY);
            if(ch == '\r') { /* convert CR to LF for processing, putty sends CR when Enter is pressed */
                ch = '\n';
                xSerial1PutChar(ch, portMAX_DELAY);
            }
            *ptr++ = ch;
            count--;
            chs++;
        }
    }
    return chs;
}    
#endif
#endif