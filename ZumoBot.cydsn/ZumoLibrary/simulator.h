#ifndef SIMULATOR_H_
#define SIMULATOR_H_
#include "project.h"
    
void SimulatorTaskInit(void);
void SimulatorTask( void *pvParameters );

typedef struct {
    uint8 ld;
    uint8 rd;
    uint8 ls;
    uint8 rs;
    uint32 delay;
} SimData;

#endif
