/**
 * @file    LSM303D.h
 * @brief   LSM303D header file
 * @details Contains register address definitions, function declarations, and data struct for the LSM303 Accelerometer
*/

#ifndef LSM303D_H_
#define LSM303D_H_

/***Required header includes***/
#include<stdint.h>

/***Device Address***/    
#define LSM303D             0x1D

/***Register address definitions***/
#define DEV_ID              0x49
    
#define REFERENCE_X         0x1C
#define REFERENCE_Y         0x1D
#define REFERENCE_Z         0x1E

#define CTRL_0              0x1F
#define CTRL_1              0x20
#define CTRL_2              0x21
#define CTRL_3              0x22
#define CTRL_4              0x23
#define CTRL_5              0x24
#define CTRL_6              0x25
#define CTRL_7              0x26

#define STATUS_A            0x27

#define MULTIPLE_READ       0x80

#define OUT_X_L_A           0x28            // Accelerometer output
#define OUT_X_H_A           0x29
#define OUT_Y_L_A           0x2A
#define OUT_Y_H_A           0x2B
#define OUT_Z_L_A           0x2C
#define OUT_Z_H_A           0x2D

#define FIFO_CTRL           0x2E
#define FIFO_SRC            0x2F

#define IG_CFG1             0x30
#define IG_SRC1             0x31
#define IG_THS1             0x32
#define IG_DUR1             0x33
#define IG_CFG2             0x34
#define IG_SRC2             0x35
#define IG_THS2             0x36
#define IG_DUR2             0x37

#define CLICK_CFG           0x38
#define CLICK_SRC           0x39
#define CLICK_THS           0x3A
#define TIME_LIMIT          0x3B
#define TIME_LATENCY        0x3C    
#define TIME_WINDOW         0x3D

#define ACT_THS             0x3E
#define ACT_DUR             0x3F

#define WHO_AM_I            0x0F

#define TEMP_OUT_L          0x05
#define TEMP_OUT_H          0x06

/***Struct used for storing raw acceration data***/    
struct accData_{

    int16_t accX;
    int16_t accY;
    int16_t accZ;

};

/***Function prototypes for LSM303D accelerometer***/
uint8_t LSM303D_Start(void);
void LSM303D_Read_Acc(struct accData_* data);

#endif
/* [] END OF FILE */
