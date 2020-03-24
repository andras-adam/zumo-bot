/**
 * @file    Motor.c
 * @brief   Basic methods for operating motor sensor. For more details, please refer to Motor.h file. 
 * @details included in Zumo shield
*/
#include "FreeRTOS.h"
#include "task.h"
#include "Motor.h"
#include "zumo_config.h"


#if ZUMO_SIMULATOR == 0

void SetMotors(uint8 left_dir, uint8 right_dir, uint8 left_speed, uint8 right_speed, uint32 delay)
{
    MotorDirLeft_Write(left_dir);      // 0 = forward, 1 = backward
    MotorDirRight_Write(right_dir);    // 0 = forward, 1 = backward
    PWM_WriteCompare1(left_speed); 
    PWM_WriteCompare2(right_speed); 
    vTaskDelay(delay);
}
/**
* @brief    Starting motor sensors
* @details  
*/
void motor_start()
{
    PWM_Start();
}


/**
* @brief    Stopping motor sensors
* @details
*/
void motor_stop()
{
    PWM_Stop();
}


#endif

/**
* @brief    Moving motors forward
* @details  giveing same speed to each side of PWM to make motors go forward
* @param    uint8 speed : speed value
* @param    uint32 delay : delay time
*/
void motor_forward(uint8 speed,uint32 delay)
{
    // set LeftMotor forward mode
    // set RightMotor forward mode
    SetMotors(0,0, speed, speed, delay);
}


/**
* @brief    Moving motors to make a turn 
* @details  moving left when l_speed < r_speed or moving right when l_speed > r_speed
* @param    uint8 l_speed : left motor speed value
* @param    uint8 r_speed : right motor speed value
* @param    uint32 delay : delay time
*/
void motor_turn(uint8 l_speed, uint8 r_speed, uint32 delay)
{
    SetMotors(0,0, l_speed, r_speed, delay);
}


/**
* @brief    Moving motors backward
* @details  setting backward mode to each motors and gives same speed to each side of PWM
* @param    uint8 speed : speed value
* @param    uint32 delay : delay time
*/
void motor_backward(uint8 speed,uint32 delay)
{
    // set LeftMotor backward mode
    // set RightMotor backward mode
    SetMotors(1,1, speed, speed, delay);
}
