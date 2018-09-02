/**
 * @file    Gyro.c
 * @brief   Basic methods for operating gyroscope. For more details, please refer to Gyro.h file. 
 * @details part number: L3GD20H (included in Zumo shield)
*/
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "I2C_Common.h"
#include "Gyro.h"

    // register addresses
    enum regAddr
    {
       WHO_AM_I       = 0x0F,

       CTRL1          = 0x20, // D20H
       CTRL_REG1      = 0x20, // D20, 4200D
       CTRL2          = 0x21, // D20H
       CTRL_REG2      = 0x21, // D20, 4200D
       CTRL3          = 0x22, // D20H
       CTRL_REG3      = 0x22, // D20, 4200D
       CTRL4          = 0x23, // D20H
       CTRL_REG4      = 0x23, // D20, 4200D
       CTRL5          = 0x24, // D20H
       CTRL_REG5      = 0x24, // D20, 4200D
       REFERENCE      = 0x25,
       OUT_TEMP       = 0x26,
       STATUS         = 0x27, // D20H
       STATUS_REG     = 0x27, // D20, 4200D

       OUT_X_L        = 0x28,
       OUT_X_H        = 0x29,
       OUT_Y_L        = 0x2A,
       OUT_Y_H        = 0x2B,
       OUT_Z_L        = 0x2C,
       OUT_Z_H        = 0x2D,

       FIFO_CTRL      = 0x2E, // D20H
       FIFO_CTRL_REG  = 0x2E, // D20, 4200D
       FIFO_SRC       = 0x2F, // D20H
       FIFO_SRC_REG   = 0x2F, // D20, 4200D

       IG_CFG         = 0x30, // D20H
       INT1_CFG       = 0x30, // D20, 4200D
       IG_SRC         = 0x31, // D20H
       INT1_SRC       = 0x31, // D20, 4200D
       IG_THS_XH      = 0x32, // D20H
       INT1_THS_XH    = 0x32, // D20, 4200D
       IG_THS_XL      = 0x33, // D20H
       INT1_THS_XL    = 0x33, // D20, 4200D
       IG_THS_YH      = 0x34, // D20H
       INT1_THS_YH    = 0x34, // D20, 4200D
       IG_THS_YL      = 0x35, // D20H
       INT1_THS_YL    = 0x35, // D20, 4200D
       IG_THS_ZH      = 0x36, // D20H
       INT1_THS_ZH    = 0x36, // D20, 4200D
       IG_THS_ZL      = 0x37, // D20H
       INT1_THS_ZL    = 0x37, // D20, 4200D
       IG_DURATION    = 0x38, // D20H
       INT1_DURATION  = 0x38, // D20, 4200D

       LOW_ODR        = 0x39  // D20H
    };

#define D20_SA0_HIGH_ADDRESS      0b1101011 // also applies to D20H
void writeReg(uint8 reg, uint8 value)
{
    I2C_Write(D20_SA0_HIGH_ADDRESS, reg, value);
}   

uint8 readReg(uint8 reg)
{
    return I2C_Read(D20_SA0_HIGH_ADDRESS, reg);
}

void read(void)
{
    uint8 data[6];
    int16 x;
    int16 y;
    int16 z;
    I2C_Read_Multiple(D20_SA0_HIGH_ADDRESS, OUT_X_L | 0x80, data, 6);
    x = (int16) ((data[1] << 8) | data[0]);
    y = (int16) ((data[3] << 8) | data[2]);
    z = (int16) ((data[5] << 8) | data[4]);
    
    printf("%d, %d, %d\n", x, y, z);
}

void enableDefault(void)
{
    // 0x00 = 0b00000000
    // Low_ODR = 0 (low speed ODR disabled)
    writeReg(LOW_ODR, 0x00);
  
    // 0x00 = 0b00000000
    // FS = 00 (+/- 250 dps full scale)
    writeReg(CTRL_REG4, 0x00);
  
    // 0x6F = 0b01101111
    // DR = 01 (200 Hz ODR); BW = 10 (50 Hz bandwidth); PD = 1 (normal mode); Zen = Yen = Xen = 1 (all axes enabled)
    writeReg(CTRL_REG1, 0x6F);
}
    
    
uint16 value_convert_gyro(uint16 raw)
{
    float rate_result;
    float G_GAIN = 0.07;                        // FS = 2000 dps
  
    rate_result = (float)raw * G_GAIN;          // degrees per second for Gyroscope
    
    return rate_result;
}


#if 0
void zmain(void *p)
{
    (void) p;
    I2C_Start();
    
    printf("ID: %02X\n", readReg(WHO_AM_I));
    
    enableDefault();
    
    uint8 data[6];
    int16 x;
    int16 y;
    int16 z;
    
    while(1) {
        I2C_Read_Multiple(D20_SA0_HIGH_ADDRESS, OUT_X_L | 0x80, data, 6);
        x = (int16) ((data[1] << 8) | data[0]);
        y = (int16) ((data[3] << 8) | data[2]);
        z = (int16) ((data[5] << 8) | data[4]);
        vTaskDelayUntil(200);
    }
    
}
#endif

#if 0
//gyroscope//
void zmain()
{
    CyGlobalIntEnable; 
    UART_1_Start();
  
    I2C_Start();
  
    uint8 X_L_G, X_H_G, Y_L_G, Y_H_G, Z_L_G, Z_H_G;
    int16 X_AXIS_G, Y_AXIS_G, Z_AXIS_G;
    
    I2C_write(GYRO_ADDR, GYRO_CTRL1_REG, 0x0F);             // set gyroscope into active mode
    I2C_write(GYRO_ADDR, GYRO_CTRL4_REG, 0x30);             // set full scale selection to 2000dps    
    
    for(;;)
    {
        //print out gyroscope output
        X_L_G = I2C_read(GYRO_ADDR, OUT_X_AXIS_L);
        X_H_G = I2C_read(GYRO_ADDR, OUT_X_AXIS_H);
        X_AXIS_G = convert_raw(X_H_G, X_L_G);
        
        
        Y_L_G = I2C_read(GYRO_ADDR, OUT_Y_AXIS_L);
        Y_H_G = I2C_read(GYRO_ADDR, OUT_Y_AXIS_H);
        Y_AXIS_G = convert_raw(Y_H_G, Y_L_G);
        
        
        Z_L_G = I2C_read(GYRO_ADDR, OUT_Z_AXIS_L);
        Z_H_G = I2C_read(GYRO_ADDR, OUT_Z_AXIS_H);
        Z_AXIS_G = convert_raw(Z_H_G, Z_L_G);
     
        // If you want to print value
        printf("%d %d %d \r\n", X_AXIS_G, Y_AXIS_G, Z_AXIS_G);
        vTaskDelay(50);
    }
}   
#endif


