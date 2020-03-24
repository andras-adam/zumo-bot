/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/*Usage Example at the bottom of this file!!!*/
#include "FreeRTOS.h"
#include "task.h"


#include"LSM303D.h"
#include"I2C.h"
#include"I2C_Common.h"
#include "zumo_config.h"

#if ZUMO_SIMULATOR == 0

uint8_t LSM303D_Start(void){
    
    I2C_Start();
    I2C_Write(LSM303D, CTRL_1, 0x77);           // set accelerometer & magnetometer into active mode
    I2C_Write(LSM303D, CTRL_7, 0x83);
    
    if(I2C_Read(LSM303D, WHO_AM_I) == DEV_ID){

        return 1;

    }
    else {
    
        return 0;
    
    }
}



void LSM303D_Read_Acc(struct accData_* data){
    
    uint8_t xyz[6];
    
    I2C_Read_Multiple(LSM303D, OUT_X_L_A | MULTIPLE_READ, xyz, 6);
    
    data->accX = (int16)(xyz[1] << 8 | xyz[0]);
    data->accY = (int16)(xyz[3] << 8 | xyz[2]);
    data->accZ = (int16)(xyz[5] << 8 | xyz[4]);
}
#else
uint8_t LSM303D_Start(void)
{
    return 1;
}

void LSM303D_Read_Acc(struct accData_* data){
    
    data->accX = 0;
    data->accY = 0;
    data->accZ = 0;
}

#endif

#if 0
/* Example of how to use this Accelerometer!!!*/

int main(void) {
    
    struct accData_ data;
    
    CyGlobalIntEnable; 
    
    UART_1_Start();
    
    printf("Accelerometer test...\n");
    if(!LSM303D_Start()){
    
        printf("LSM303D failed to initialize!!! Program is Ending!!!\n");
        return 0;
    
    }
    else
        printf("Device Ok...");
        
   
    for(;;)
    {
        LSM303D_Read_Acc(&data);
        printf("%10d %10d %10d\n",data.accX, data.accY, data.accZ);
        
    }
 }   

#endif
/* [] END OF FILE */
