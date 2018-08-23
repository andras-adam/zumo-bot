/**
 * @file    I2C_made.c
 * @brief   Basic methods for I2C communication. For more details, please refer to I2C_made.h file. 
 * @details 
*/
#include "I2C_Common.h"
#include "I2C.h"

/**
* @brief    Function for generic I2C byte write
* @details  Single byte write 
* @param    uint8 device : Slave Device address
* @param    uint8 regAdd : register address to be written to
* @param    uint8 data : value to be written
*/
uint8_t I2C_Write(uint8_t device, uint8_t regAdd, uint8_t data){

    uint8 write_buf[2] = {regAdd, data};
 
    I2C_MasterClearStatus(); // clear any pending status flags
    I2C_MasterWriteBuf(device, write_buf, 2, I2C_MODE_COMPLETE_XFER); // one complete byte transfer  
    while(((I2C_MasterStatus()) & I2C_MSTAT_WR_CMPLT) == 0); //Check status and make sure it has completed
    return 0;
}

/**
* @brief    Function for generic I2C single byte read
* @details  Single byte read 
* @param    uint8 device : Slave Device address
* @param    uint8 regAdd : register address to be read from
*/

uint8_t I2C_Read(uint8_t device, uint8_t regAdd){
    
    uint8_t data; // Variable for data to be read
    
    I2C_Read_Multiple(device, regAdd, &data, 1);
    
    return data;
}

/**
* @brief    Function for generic I2C multi byte read
* @details  Multiple byte read 
* @param    uint8 device : Slave Device address
* @param    uint8 startAdd : The address of the first register to be read. Subsequent reads will auto increment this 
* @param    uint8 data : Pointer for value/values to be read
* @param    uint8 num_bytes_to_be_read : number of bytes to be read 
*/

void I2C_Read_Multiple(uint8_t device, uint8_t startAdd, uint8_t* data, uint8_t num_bytes_to_be_read){
    
    I2C_MasterClearStatus(); // clear any pending status flags
    I2C_MasterWriteBuf(device, &startAdd, 1, I2C_MODE_COMPLETE_XFER); // one complete byte transfer to establish address to be read
    while((I2C_MasterStatus() & I2C_MSTAT_WR_CMPLT) == 0); //Check status and make sure it has completed
    I2C_MasterClearStatus(); // clear any pending status flags
    I2C_MasterReadBuf(device, data, num_bytes_to_be_read, I2C_MODE_COMPLETE_XFER);//  complete byte/multi-byte receive
    while((I2C_MasterStatus() & I2C_MSTAT_RD_CMPLT) == 0); //Check status and make sure it has completed

}

