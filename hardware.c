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
#define PIR; //bitband
// Servo Motor on PORT A pin 6 (PA6)
#define MOTOR_PORT PORTA
#define MOTOR_PIN 6

// Debounce timer
volatile bool debounceActive = false;

// flags
volatile bool pillDispensed = false;

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

void initMotor() // Call in main
{
    enablePort(MOTOR_PORT);
    selectPinDigitalOutput(MOTOR_PORT, MOTOR_PIN);
    setPinCommitControl(MOTOR_PORT, MOTOR_PIN);
    setPinValue(MOTOR_PORT, MOTOR_PIN, LOW); // Set initial state to LOW
}

// when the topic subscribed to announces that a pill should be dispensed 
// the motor should rotate 360 degrees
// Call this function to dispense a pill // edit in MQTT reading published messages to know when to call this function
void dispensePill()
{
    // Rotate motor clockwise
    setPinValue(MOTOR_PORT, MOTOR_PIN, HIGH);
    delay(1000); // Rotate for 1 second (adjust as needed)
    setPinValue(MOTOR_PORT, MOTOR_PIN, LOW); // Stop motor

    pillDispensed = true; // Set flag to indicate pill has been dispensed
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
    // Send message only if PIR sensor is triggered AFTER a pill is dispensed
    if (pillDispensed == true)
    {
        putsUart0("pill has been taken\n"); // Replace with publish message
        // publishMqtt("pill/status", "taken"); 
        pillDispensed = false; // Reset the flag
    }
}

void waitPIRTrigger()
{
    uint32_t startTime = tickCounter;
    uint32_t currTime;

    while (!PIR)
    {
        currTime = tickCounter;
        if ((currTime - startTime) > 60000) // 60 second timeout
        {
            break;
        }
    }

    //publishMqtt("", "taken"); 
}