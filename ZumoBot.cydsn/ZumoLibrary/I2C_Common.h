/**
 * @file    I2C_made.h
 * @brief   I2C header file
 * @details If you want to use I2C methods, Include I2C_made.h file.
*/

#include<stdint.h>

uint8_t I2C_Write(uint8_t device, uint8_t regAdd, uint8_t data);
uint8_t I2C_Read(uint8_t device, uint8_t regAdd);
void I2C_Read_Multiple(uint8_t device, uint8_t start_regAdd, uint8_t* data, uint8_t size);

