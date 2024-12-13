// CONFIG
#pragma config OSC = IntRC      // Oscillator selection bits (internal RC oscillator)
#pragma config WDT = OFF        // Watchdog timer enable bit (WDT disabled)
#pragma config CP = OFF         // Code protection bit (Code protection off)
#pragma config MCLRE = OFF      // MCLR enable bit (MCLR tied to VDD, (Internally))

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>

#define PWR_OK (1U << 0)
#define PS_ON (1U << 1)
#define LED (1U << 2)
#define EXT_ONOFF (1U << 3)
#define INT_ONOFF (1U << 4)
#define PULSE_OR_LATCH (1U << 5)
#define PWR_OK_MAX_FAILCOUNT 25

void
main(void)
{
    //CMCON = 7;
    //ANSEL = 0;
    
    OPTION = 0xC0;
    
    GPIO = LED;
    TRIS = ~(PS_ON | LED) & 0xFF;
    
    bool psuOn = false;
    bool isPulseNotLatch = GPIO & PULSE_OR_LATCH;
    bool wasPulseNotLatch = GPIO & PULSE_OR_LATCH;
    bool intOnOffWasPressed = false;
    bool intOnOffIsPressed = false;
    bool extOnOffWasPressed = false;
    bool extOnOffIsPressed = false;
    bool pwrOkFailed = false;
    uint8_t pwrOkFailCount = 0;
    
    while (1)
    {
        isPulseNotLatch = GPIO & PULSE_OR_LATCH;
        intOnOffIsPressed = !(GPIO & INT_ONOFF);
        extOnOffIsPressed = !(GPIO & EXT_ONOFF);
        
        if (isPulseNotLatch != wasPulseNotLatch)
        {
            while (1)
            {
                //Safety lockup, do not change jumper on the fly...
                GPIO &= ~PS_ON;
                GPIO ^= LED;
                __delay_ms(100);
            }
        }
        
        if (psuOn & !(GPIO & PWR_OK))
        {
            if (++pwrOkFailCount >= PWR_OK_MAX_FAILCOUNT)
            {
                pwrOkFailed = true;
                psuOn = false;
            }
        }
        
        if (psuOn)
        {           
            GPIO |= PS_ON;
        }
        else
        {
            GPIO &= ~PS_ON;
        }
        
        if (pwrOkFailed)
        {
            GPIO ^= LED;
            if ((intOnOffIsPressed != intOnOffWasPressed) 
            ||  (extOnOffIsPressed != extOnOffWasPressed))
            {
                pwrOkFailCount = 0;
                pwrOkFailed = false;
            }
        }
        else
        {
            GPIO |= LED;
            
            if (intOnOffIsPressed && !intOnOffWasPressed)
            {
                psuOn = !psuOn;
            }

            if (isPulseNotLatch)
            {
                if (extOnOffIsPressed && !extOnOffWasPressed)
                {
                    psuOn = !psuOn;
                }
            }
            else
            {
                if (extOnOffIsPressed && !extOnOffWasPressed)
                {
                    psuOn = true;
                }

                if (extOnOffWasPressed && !extOnOffIsPressed)
                {
                    psuOn = false;
                }
            }
        }
            
        wasPulseNotLatch = isPulseNotLatch;
        intOnOffWasPressed = intOnOffIsPressed;
        extOnOffWasPressed = extOnOffIsPressed;
                
        __delay_ms(100);
    }
}
