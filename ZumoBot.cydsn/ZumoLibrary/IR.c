/**
 * @file IR.c
 * @brief Basic methods for operating IR receiver. For more details, please refer to IR.h file. 
 * @details part number: TSOP-2236
*/

#include "zumo_config.h"

#if ZUMO_SIMULATOR == 0

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "project.h"
#include "IR.h"

#define IRB_SIZE 64

static QueueHandle_t ir_q;

CY_ISR(IR_capture_isr)
{
    static uint32_t old;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t status = Timer_IR_ReadStatusRegister();
    if(status & Timer_IR_STATUS_CAPTURE) {
        uint32_t capture = Timer_IR_ReadCapture();
        uint32_t value  = (old - capture) & 0xFFFFFFUL;
        old = capture;
        //Pin_1_Write(IR_receiver_Read()); // for debugging with scope
        if(IR_receiver_Read()==0) { // signal was low --> we captured falling edge
            value |= IR_SIGNAL_HIGH;
        }
        xQueueSendToBackFromISR(ir_q, &value, &xHigherPriorityTaskWoken);
    }
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

bool IR_get(uint32_t *value, uint32_t wait) {
    return xQueueReceive(ir_q, value, wait);
}

void IR_flush(void)
{
    uint32_t tmp;
    while(IR_get(&tmp, 0)); // flush IR rcv buffer
}


void IR_wait(void)
{
    int count = 0;
    IR_flush();

    while(true) {
        uint32_t tmp;
        if(IR_get(&tmp, configTICK_RATE_HZ / 5)) {
            if(!(tmp & IR_SIGNAL_HIGH)) {
                tmp &= IR_SIGNAL_MASK;
                if(tmp > IR_LOWER_LIMIT) {
                    count += tmp;
                    if(count>20000) break;
                }
                else {
                    count = 0;
                }
            }
        }
        else {
            count = 0;
        }
    }
}    

/**
* @brief    Starting Ultra Sonic Sensor
* @details  
*/
void IR_Start(void)
{
    static bool init = false;
    
    if(!init) {
        init = true;
        ir_q = xQueueCreate(IRB_SIZE, sizeof(uint32_t));
        IR_isr_StartEx(IR_capture_isr);               // Start ultra sonic interrupt
        Timer_IR_Start();                                      // Start Timer
    }
}

#endif

