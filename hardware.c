// Arsh & Angie 

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL with LCD/Keyboard Interface
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// PIR sensor D2  ->  trigger message to subscriber "Pills have been taken"

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "gpio.h"
#include "stdio.h"
#include "timer.h"

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

// PIR sensor on PORT D pin 2 (PD2)
#define PIR_PORT PORTD
#define PIR_PIN  2

// Debounce timer
volatile bool debounceActive = false;

// Topic and data publish


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initPIRSensor() // Call in main
{
    enablePort(PIR_PORT);
    selectPinDigitalInput(PIR_PORT, PIR_PIN);
    setPinCommitControl(PIR_PORT, PIR_PIN);
    enablePinPullup(PIR_PORT, PIR_PIN);
    selectPinInterruptRisingEdge(PIR_PORT, PIR_PIN);
    clearPinInterrupt(PIR_PORT, PIR_PIN);
    enablePinInterrupt(PIR_PORT, PIR_PIN);

    // Enable IRQ interrupt in NVIC for Port D (3)
    NVIC_EN0_R |= (1 << 3); 
}

// Callback function for the debounce timer
void debounceTimerCallback()
{
    debounceActive = false; // Reset debounce state
}

void gpioPortDIsr()
{
    // Clear interrupt
    clearPinInterrupt(PIR_PORT, PIR_PIN);

    // Check if debounce is active
    if (debounceActive)
    {
        return; // Ignore interrupt if debounce is active
    }

    // Set debounce state and start a one-shot timer for 200 ms
    debounceActive = true;
    startOneshotTimer(debounceTimerCallback, 0.2); // 200 ms

    // Process the valid trigger
    putsUart0("pill has been taken\n"); // Replace with publish message
    // publishMqtt("pill/status", "taken"); 
}