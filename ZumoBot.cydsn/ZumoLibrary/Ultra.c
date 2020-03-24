/**
 * @file    Ultra.c
 * @brief   Basic methods for operating ultrasonic sensor. For more details, please refer to Ultra.h file. 
 * @details part number: HC-SR04
*/

#include "zumo_config.h"

#if ZUMO_SIMULATOR == 0


#include "FreeRTOS.h"
#include "task.h"

#include "project.h"
#include <stdint.h>
#include <stdbool.h>

#include "Ultra.h"

static TaskHandle_t UltraHandle = NULL;
static volatile int distance;


/**
* @brief    Ultrasonic handler - called from FreeRTOS systick hook
* @details  Counts ticks to next activation
*/
void UltraSonicTask( void *pvParameters )
{
    (void) pvParameters;

    TickType_t prev;
    
    prev = xTaskGetTickCount();
    
    // try to trigger every 25 ms
    while(1) {
        // Trigger High only if echo pulse is not active
        if(Echo_Read() == 0) {
            Trig_Write(1);
            vTaskDelay(1);
            Trig_Write(0);
        }
        vTaskDelayUntil(&prev, 25);
    }
}


/**
* @brief    Ultra Sonic Sensor Interrupt Handler
* @details  Measuring reflecting time to decide distance between Zumobot and obstacle
*/
CY_ISR(ultra_isr_handler)
{
    static uint16_t prev;
    static uint32_t ovf;
    static int echo;
    uint16_t now;
    uint32_t status;
    do {
        status = Timer_ReadStatusRegister();
        if(status & Timer_STATUS_FIFONEMP) {
            now = Timer_ReadCapture();
            if(Echo_Read() == 0) {
                uint16_t time = prev - now;
                if(echo == 1 && ovf <= 1) {
                    distance = time / 58;
                }
                echo = 0;
#if 0                
                // for visual debugging of overly long pulse
                if(ovf > 1) {
                    BatteryLed_Write(!BatteryLed_Read());
                }
#endif                
            }
            else {
                echo = 1;
            }
            prev = now;
            ovf = 0;
        }
        if(status & Timer_STATUS_TC) {
            ++ovf; // used to count overly long pulses
        }
    } while(status & Timer_STATUS_FIFONEMP);
}


/**
* @brief    Starting Ultra Sonic Sensor
* @details  
*/
void Ultra_Start()
{
    if(UltraHandle == NULL) {
        ultra_isr_StartEx(ultra_isr_handler);               // Start ultra sonic interrupt
        Timer_Start();                                      // Start Echo pulse capture timer
        // task runs at highest priority but sleeps most of the time
        xTaskCreate( UltraSonicTask, "Ultra", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &UltraHandle );
    }
}

int Ultra_GetDistance(void)
{
    return distance;       
}    

#endif


