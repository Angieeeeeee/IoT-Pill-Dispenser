// Publish Scheduling
// Angie & Arsh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: -
// Target uC:       -
// System Clock:    -

// Hardware configuration:
// -

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef PUBLISH_H_
#define PUBLISH_H_

#include <stdint.h>
#include "mqtt.h"
#include "tcp.h"
#include "uart0.h"

typedef struct scheduler
{
    char pillName[20];
    int pillCount;
    int time; // from beginning of week
} scheduler;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void bootTime();
void returnDate();
void initSysTick();
void sysTickISR();
void formatPublishData(char *strData);
void publishSchedule();


#endif