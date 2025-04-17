// Hardware Library
// Arsh & Angie 

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// PIR Sensor

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef HARDWARE_H_
#define HARDWARE_H_

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"
#include "wait.h"
#include "uart0.h"

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initPIRSensor();
void debounceTimerCallback()
void gpioPortDIsr()

#endif
