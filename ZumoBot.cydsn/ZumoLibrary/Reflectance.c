/**
 * @file    Reflectance.c
 * @brief   Basic methods for operating reflectance sensor. For more details, please refer to Reflectance.h file. 
 * @details included in Zumo shield
*/
#include <stdio.h>
#include <stdbool.h>
#include "project.h"
#include "FreeRTOS.h"
#include "task.h"

#include "Reflectance.h"

static volatile struct sensors_ sensors;
static volatile struct sensors_  digital_sensor_value;
static struct sensors_ threshold = { 10000, 10000, 10000, 10000, 10000, 10000};
static TaskHandle_t ReflectanceHandle = NULL;

/**
* @brief    Reflectance Sensor handler is called from FreeRTOS systick hook
* @details  Measuring each sensors' time to recognition of white or black
*/
void ReflectanceTask( void *pvParameters )
{
    (void) pvParameters;
    
    TickType_t prev;
    uint32_t statusR1;
    uint32_t statusR2;
    uint32_t statusR3;
    uint32_t statusL3;
    uint32_t statusL2;
    uint32_t statusL1;
    
    prev = xTaskGetTickCount();
    
    while(1) {
        statusR1 = Timer_R1_ReadStatusRegister();
        statusR2 = Timer_R2_ReadStatusRegister();
        statusR3 = Timer_R3_ReadStatusRegister();
        statusL3 = Timer_L3_ReadStatusRegister();
        statusL2 = Timer_L2_ReadStatusRegister();
        statusL1 = Timer_L1_ReadStatusRegister();

        if(statusR1 & Timer_R1_STATUS_CAPTURE) {
            sensors.r1 = Timer_R1_ReadPeriod() - Timer_R1_ReadCapture();
        }
        else {
            sensors.r1 = Timer_R1_ReadPeriod() - Timer_R1_ReadCounter();
        }
        
        if(statusL1 & Timer_L1_STATUS_CAPTURE) {
            sensors.l1 = Timer_L1_ReadPeriod() - Timer_L1_ReadCapture();
        }
        else {
            sensors.l1 = Timer_L1_ReadPeriod() - Timer_L1_ReadCounter();
        }

        if(statusR2 & Timer_R2_STATUS_CAPTURE) {
            sensors.r2 = Timer_R2_ReadPeriod() - Timer_R2_ReadCapture();
        }
        else {
            sensors.r2 = Timer_R2_ReadPeriod() - Timer_R2_ReadCounter();
        }
        
        if(statusL2 & Timer_L2_STATUS_CAPTURE) {
            sensors.l2 = Timer_L2_ReadPeriod() - Timer_L2_ReadCapture();
        }
        else {
            sensors.l2 = Timer_L2_ReadPeriod() - Timer_L2_ReadCounter();
        }
        
        if(statusR3 & Timer_R3_STATUS_CAPTURE) {
            sensors.r3 = Timer_R3_ReadPeriod() - Timer_R3_ReadCapture();
        }
        else {
            sensors.r3 = Timer_R3_ReadPeriod() - Timer_R3_ReadCounter();
        }
        
        if(statusL3 & Timer_L3_STATUS_CAPTURE) {
            sensors.l3 = Timer_L3_ReadPeriod() - Timer_L3_ReadCapture();
        }
        else {
            sensors.l3 = Timer_L3_ReadPeriod() - Timer_L3_ReadCounter();
        }
        
        
        Timer_Reset_Write(1);
        
        R1_SetDriveMode(PIN_DM_STRONG);
        R1_Write(1);
        R2_SetDriveMode(PIN_DM_STRONG);
        R2_Write(1);
        R3_SetDriveMode(PIN_DM_STRONG);
        R3_Write(1);
        L3_SetDriveMode(PIN_DM_STRONG);
        L3_Write(1);
        L2_SetDriveMode(PIN_DM_STRONG);
        L2_Write(1);
        L1_SetDriveMode(PIN_DM_STRONG);
        L1_Write(1);
        Timer_Reset_Write(0);

        CyDelayUs(10);
        R1_SetDriveMode(PIN_DM_DIG_HIZ);
        R2_SetDriveMode(PIN_DM_DIG_HIZ);
        R3_SetDriveMode(PIN_DM_DIG_HIZ);
        L3_SetDriveMode(PIN_DM_DIG_HIZ);
        L2_SetDriveMode(PIN_DM_DIG_HIZ);
        L1_SetDriveMode(PIN_DM_DIG_HIZ);
        vTaskDelayUntil(&prev, 1);
    }
}


/**
* @brief    Starting Reflectance Sensor
* @details 
*/
void reflectance_start()
{
    if(ReflectanceHandle == NULL) {
        Refl_led_Write(1);

        Timer_R1_Start();
        Timer_R2_Start();
        Timer_R3_Start();
        Timer_L3_Start();
        Timer_L2_Start();
        Timer_L1_Start();
        // task runs at highest priority and wakes up every millisecond but uses very little CPU time
        xTaskCreate( ReflectanceTask, "Reflectance", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &ReflectanceHandle );
    }
}


/**
* @brief    Read reflectance sensor values
* @details
*/
void reflectance_read(struct sensors_ *values)
{
    *values = sensors;
}


/**
* @brief    Making Reflectance Sensor's period to digital
* @details
*/
void reflectance_digital(struct sensors_ *digital)
{
    //if the results of reflectance_period function is over threshold, set digital_sensor_value to 1, which means it's black
    if(sensors.l3 < threshold.l3)
        digital->l3 = 0;
    else
        digital->l3 = 1;
    
    if(sensors.l2 < threshold.l2)
        digital->l2 = 0;
    else
        digital->l2 = 1;
    
    if(sensors.l1 < threshold.l1)
        digital->l1 = 0;
    else
        digital->l1 = 1;
    
    if(sensors.r1 < threshold.r1)
        digital->r1 = 0;
    else
        digital->r1 = 1;
    
    if(sensors.r2 < threshold.r2)
        digital->r2 = 0;
    else
        digital->r2 = 1;
    
    if(sensors.r3 < threshold.r3)
        digital->r3 = 0;
    else
        digital->r3 = 1;
        
}

void reflectance_set_threshold(uint16_t l3, uint16_t l2, uint16_t l1, uint16_t r1, uint16_t r2, uint16_t r3)
{
    threshold.l3 = l3;
    threshold.l2 = l2;
    threshold.l1 = l1;
    threshold.r3 = r3;
    threshold.r2 = r2;
    threshold.r1 = r1;
}


