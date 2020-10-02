#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "project.h"
#include "serial1.h"

#include "zumo_config.h"

#include "simulator.h"
#include "Motor.h"
#include "Reflectance.h"
#include "Ultra.h"
#include "IR.h"
#include <stdio.h>

static QueueHandle_t motor_q;

void SimulatorTaskInit(void)
{
    motor_q = xQueueCreate(3, sizeof(SimData));
}

#if ZUMO_SIMULATOR == 1
static void send(const char *data, int len) ;
    
#define RUN_TIMEOUT 5
#define STOP_TIMEOUT 25

static const char stopped[5] = {0,255,0,0,RUN_TIMEOUT};

static volatile int motor_started;    
/*
 * Motor simulator
 */
void SetMotors(uint8 left_dir, uint8 right_dir, uint8 left_speed, uint8 right_speed, uint32 delay)
{
    SimData sim = { left_dir, right_dir, left_speed, right_speed, delay };
    if(!motor_started) {
        printf("You haven't started motors\n");
    }
    if(delay < 2) {
        sim.delay = 2;
    }
    xQueueSend(motor_q, &sim, portMAX_DELAY);
    vTaskDelay(delay);
}


/**
* @brief    Starting motor sensors
* @details  
*/
void motor_start()
{
    motor_started = 1;
}


/**
* @brief    Stopping motor sensors
* @details
*/
void motor_stop()
{
    motor_started = 0;
    send(stopped, 5);
}

/*
 * Reflectance sensor simulator
 */

static volatile struct sensors_ sensors;
static struct sensors_ threshold = { 10000, 10000, 10000, 10000, 10000, 10000};
static volatile int reflectance_started;

/**
* @brief    Starting Reflectance Sensor
* @details 
*/
void reflectance_start()
{
    reflectance_started = 1;
}

/**
* @brief    Read reflectance sensor values
* @details
*/
void reflectance_read(struct sensors_ *values)
{
    if(!reflectance_started) {
        printf("You haven't started reflectance sensor\n");
    }
    *values = sensors;
}

/**
* @brief    Making Reflectance Sensor's period to digital
* @details
*/
void reflectance_digital(struct sensors_ *digital)
{
    if(!reflectance_started) {
        printf("You haven't started reflectance sensor\n");
    }
    //if the results of reflectance_period function is over threshold, set digital_sensor_value to 1, which means it's black
    if(sensors.L3 < threshold.L3)
        digital->L3 = 0;
    else
        digital->L3 = 1;
    
    if(sensors.L2 < threshold.L2)
        digital->L2 = 0;
    else
        digital->L2 = 1;
    
    if(sensors.L1 < threshold.L1)
        digital->L1 = 0;
    else
        digital->L1 = 1;
    
    if(sensors.R1 < threshold.R1)
        digital->R1 = 0;
    else
        digital->R1 = 1;
    
    if(sensors.R2 < threshold.R2)
        digital->R2 = 0;
    else
        digital->R2 = 1;
    
    if(sensors.R3 < threshold.R3)
        digital->R3 = 0;
    else
        digital->R3 = 1;
        
}

void reflectance_set_threshold(uint16_t L3, uint16_t L2, uint16_t L1, uint16_t R1, uint16_t R2, uint16_t R3)
{
    threshold.L3 = L3;
    threshold.L2 = L2;
    threshold.L1 = L1;
    threshold.R3 = R3;
    threshold.R2 = R2;
    threshold.R1 = R1;
}

/*
 * Retarget printf
 */

static xSemaphoreHandle writeMutex;

void RetargetInit(void)
{
    writeMutex = xSemaphoreCreateMutex();;
}

int _write(int file, char *ptr, int len)
{
    (void)file; /* Parameter is not used, suppress unused argument warning */
    
    if(xSemaphoreTake(writeMutex, portMAX_DELAY) == pdFALSE) return 0;
    
	int n;
	for(n = 0; n < len; n++) {
        if(*ptr == '\n') xSerial1PutChar('\r', portMAX_DELAY);
		xSerial1PutChar(*ptr++, portMAX_DELAY);
	}
    
    xSemaphoreGive(writeMutex);

    return len;
}

int _read (int file, char *ptr, int count)
{
    (void) file;
    (void) ptr;
    (void) count;
    return 0;
}

/*
 * Ultra Sonic Sensor simulator
 */
static volatile int distance;
static volatile int ultra_started;


void Ultra_Start()
{
    ultra_started = 1;
}

int Ultra_GetDistance(void)
{
    if(!ultra_started) {
        printf("You haven't started ultrasonic sensor\n");
    }
    return distance;       
}    


/*
 * IR Sensor simulator
 */
static volatile int ir_value;
static volatile int ir_started;


void IR_Start(void)
{
    ir_started = 1;
}


/* Flush all previously received IR values */
void IR_flush(void)
{
    if(!ir_started) {
        printf("You haven't started IR sensor\n");
    }
    ir_value = 0;
}

/* Wait for a high pulse with length between IR_LOWER_LIMIT and IR_UPPER_LIMIT */
void IR_wait(void)
{
    if(!ir_started) {
        printf("You haven't started IR sensor\n");
    }
    while(!ir_value) {
        vTaskDelay(10);
    }
    IR_flush();
}



/*
 * Simulator send/receive task
 */


static void send(const char *data, int len) 
{
   if(xSemaphoreTake(writeMutex, portMAX_DELAY) == pdFALSE) return;

   while(len-- > 0) {
        xSerial1PutChar(*data++, 0);
    }

    xSemaphoreGive(writeMutex);
}


void SimulatorTask( void *pvParameters )
{
    (void) pvParameters;
    SimData sd = { 0, 0, 0, 0, 0 };
    char data[5] = {0,255,0,0,RUN_TIMEOUT};
    uint32_t delay;
    char sdata[8];
    int pos = 0;
    TickType_t wait_time = RUN_TIMEOUT;

    while(1) {
        // reduce communication interval if speed is zero or motors are stopped
        if((sd.ls == 0 && sd.rs == 0) || !motor_started) {
            wait_time = STOP_TIMEOUT;
        }
        else {
            wait_time = RUN_TIMEOUT;
        }
        if(xQueueReceive(motor_q, &sd, wait_time) == pdTRUE) {
            // motor speed data was updated get speed from the updated data
            delay = sd.delay;
        }
        else {
            // no new speed data was received
            // set new delay, use old motor data
            delay = RUN_TIMEOUT;
        }
        if(motor_started) {
            // motors are running take speed from current settings
            data[0] = (sd.ld ? 0x01 : 0x0) | (sd.rd ? 0x02 : 0x0 );
            data[1] = ~data[0];
            data[2] = sd.ls;
            data[3] = sd.rs;
        }
        else {
            // if motors haven't been started force speed to zero
            data[2] = 0;
            data[3] = 0;
        }
        
        TickType_t LastWakeTime = xTaskGetTickCount();
        while(delay > 0) {
            uint32_t ms = (delay > 60) ? 50 : delay;
            delay -= ms;
            data[4] = ms;
            send(data, 5);
            pos = 0;
            while(pos < 8 && xSerial1GetChar(sdata+pos, 100)) {
                if(sdata[0] < 32) {
                    ++pos;
                }
            }
            if(pos == 8 ) {
                ir_value += sdata[0] & 0x01;
                if(ultra_started) {
                    distance = sdata[1] & 0x7f;
                }
                if(reflectance_started) {
                    sensors.L3 = sdata[2] * 100;
                    sensors.L2 = sdata[3] * 100;
                    sensors.L1 = sdata[4] * 100;
                    sensors.R1 = sdata[5] * 100;
                    sensors.R2 = sdata[6] * 100;
                    sensors.R3 = sdata[7] * 100;
                }
            }
            vTaskDelayUntil( &LastWakeTime, ms );
        }
        
    }
}

#endif
