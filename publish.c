// Publish Functions for MQTT
// Angie
// on the other end, a timer conts down and dispenses the same amount of pills 
// at the correct time by setting a week long timer 

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

#include <stdio.h>
#include <string.h>
#include "publish.h"
#include "mqtt.h"
#include "uart0.h"

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

uint32_t secSinceBoot = 0; // seconds since boot

//array of scheduler structs
struct scheduler pillScheduler[10];
int pillSchedCount = 0; // number of pills in the scheduler

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Function to initialize the time 
void bootTime()
{
    // Ask for current time
    putsUart0("What is the current time? (ex. SUNDAY 10:30 AM)\n");
    char time[30];
    getsUart0(time);

    // Parse input
    char *day = strtok(time, " ");
    char *hour = strtok(NULL, ":");
    char *minute = strtok(NULL, " ");
    char *ampm = strtok(NULL, " ");

    if (!day || !hour || !minute || !ampm) return; // incorrectly formatted input 

    int dayOfWeek = 0;
    if      (strcmp(day, "SUNDAY") == 0) dayOfWeek = 0;
    else if (strcmp(day, "MONDAY") == 0) dayOfWeek = 1;
    else if (strcmp(day, "TUESDAY") == 0) dayOfWeek = 2;
    else if (strcmp(day, "WEDNESDAY") == 0) dayOfWeek = 3;
    else if (strcmp(day, "THURSDAY") == 0) dayOfWeek = 4;
    else if (strcmp(day, "FRIDAY") == 0) dayOfWeek = 5;
    else if (strcmp(day, "SATURDAY") == 0) dayOfWeek = 6;
    else return; // invalid day

    int hourOfDay = atoi(hour);
    int minuteOfHour = atoi(minute);

    // if its in PM then add 12 to the hour
    if (strcmp(ampm, "PM") == 0 && hourOfDay != 12)
        hourOfDay += 12;
    else if (strcmp(ampm, "AM") == 0 && hourOfDay == 12)
        hourOfDay = 0;

    // Calculate seconds from start of week (sunday 12:00 am)
    secSinceBoot = (dayOfWeek * 86400) + (hourOfDay * 3600) + (minuteOfHour * 60);
    //                       24*60*60                60*60 
}

// Function to return the current date and time in a readable format inside putty
void returnDate()
{
    int day = (secSinceBoot / 86400) % 7;
    int hour = (secSinceBoot % 86400) / 3600;
    int minute = (secSinceBoot % 3600) / 60;
    int second = secSinceBoot % 60;

    // Print day
    if (day == 0) putsUart0("SUNDAY ");
    else if (day == 1) putsUart0("MONDAY ");
    else if (day == 2) putsUart0("TUESDAY ");
    else if (day == 3) putsUart0("WEDNESDAY ");
    else if (day == 4) putsUart0("THURSDAY ");
    else if (day == 5) putsUart0("FRIDAY ");
    else if (day == 6) putsUart0("SATURDAY ");

    // Convert time to readable form without sprintf
    char buffer[20];
    int x = 0;

    // Hour
    if (hour >= 10) buffer[x++] = '0' + (hour / 10);
    buffer[x++] = '0' + (hour % 10);
    buffer[x++] = ':';

    // Minute
    if (minute >= 10) buffer[x++] = '0' + (minute / 10);
    else buffer[x++] = '0';
    buffer[x++] = '0' + (minute % 10);
    buffer[x++] = ':';

    // Second
    if (second >= 10) buffer[x++] = '0' + (second / 10);
    else buffer[x++] = '0';
    buffer[x++] = '0' + (second % 10);

    buffer[x] = '\0'; // Null terminate

    putsUart0(buffer);
}

// Function to initialize the SysTick timer and increment the seconds since boot
void initSysTick()
{
    NVIC_ST_CTRL_R = 0;               // disable SysTick
    NVIC_ST_RELOAD_R = 40000000 - 1;  // 40MHz clock, 1 second reload
    NVIC_ST_CURRENT_R = 0;            // clear current value register
    NVIC_ST_CTRL_R = 0x07;            // enable with core clock and interrupts
}
void sysTickISR()
{
    // increment the seconds since boot every second
    secSinceBoot++;
    // start over at 604800 seconds (1 week)
    if (secSinceBoot >= 604800) secSinceBoot = 0;
}

// Format the data for publishing
void formatPublishData(char *strData)
{
    // strData is in the format "pillName, DAY HH:MM AM/PM, pillAmount"
    char *pillName = strtok(strData, ", ");
    char *pillTimeRaw = strtok(NULL, ", ");
    char *pillAmount = strtok(NULL, ", ");

    if (!pillTimeRaw || !pillAmount) return;
    pillSchedCount ++;

    char *day = strtok(pillTimeRaw, " ");
    char *hour = strtok(NULL, ":");
    char *minute = strtok(NULL, " ");
    char *ampm = strtok(NULL, " ");

    if (!day || !hour || !minute || !ampm) return;

    int dayOfWeek = 0;
    if      (strcmp(day, "SUNDAY") == 0) dayOfWeek = 0;
    else if (strcmp(day, "MONDAY") == 0) dayOfWeek = 1;
    else if (strcmp(day, "TUESDAY") == 0) dayOfWeek = 2;
    else if (strcmp(day, "WEDNESDAY") == 0) dayOfWeek = 3;
    else if (strcmp(day, "THURSDAY") == 0) dayOfWeek = 4;
    else if (strcmp(day, "FRIDAY") == 0) dayOfWeek = 5;
    else if (strcmp(day, "SATURDAY") == 0) dayOfWeek = 6;
    else return;

    int hourOfDay = atoi(hour);
    int minuteOfHour = atoi(minute);

    if (strcmp(ampm, "PM") == 0 && hourOfDay != 12) hourOfDay += 12;
    else if (strcmp(ampm, "AM") == 0 && hourOfDay == 12) hourOfDay = 0;

    int targetSeconds = (dayOfWeek * 86400) + (hourOfDay * 3600) + (minuteOfHour * 60);
    int timeDiff = targetSeconds - secSinceBoot;
    if (timeDiff < 0) timeDiff += 604800; // wrap around

    // Store pill data in the scheduler
    strcpy(pillScheduler[pillSchedCount].pillName, pillName);
    pillScheduler[pillSchedCount].pillCount = atoi(pillAmount);
    pillScheduler[pillSchedCount].time = targetSeconds;

    int pillCount = atoi(pillAmount);

    // Start overwriting strData
    int x = 0;

    // Write single-digit pillCount
    strData[x++] = '0' + pillCount;

    // Add space separator
    strData[x++] = ' ';

    // Write timeDiff
    if (timeDiff == 0)
    {
        strData[x++] = '0';
    }
    else
    {
        char temp[10];
        int n = 0;
        while (timeDiff > 0)
        {
            temp[n++] = '0' + (timeDiff % 10);
            timeDiff /= 10;
        }
        for (int i = n - 1; i >= 0; i--)
        {
            strData[x++] = temp[i];
        }
    }

    strData[x] = '\0'; // null terminate
}

// called every minute to see if any pills need to be dispensed
// if so, dispense them and publish the data
void publishSchedule()
{
    // for loop checking if the current time is equal to the time in the scheduler
    int i = 0;
    for (i = 0; i < pillSchedCount; i++)
    {
        int schedTime = pillScheduler[i].time;
        if (secSinceBoot < (schedTime + 60) && secSinceBoot > (schedTime - 60)) // time within 1 minute
        {
            char data[4];
            data[0] = pillScheduler[pillSchedCount].pillCount + '0';
            data[1] = ' ';
            data[2] = "0";
            data[3] = '\0';
            // publish the data
            publishMQTT("PillSetupData", data);
        }
    }
}