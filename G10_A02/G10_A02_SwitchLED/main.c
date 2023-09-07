#include "tm4c123gh6pm.h"   // Include the header file for the TM4C123GH6PM board
#include <stdint.h>        // Include the standard integer types header file

#define LED_RED (1U << 1) // macro for red LED, which is connected to pin 1 of port F
#define SW1 (1U<<4)      // macro for switch 1, which is connected to pin 4 of port F

int main(void)
{
    int state;                         // variable to store the state of the switch
    SYSCTL_RCGCGPIO_R |= (1U << 5);   // Enable clock for GPIO port F
    GPIO_PORTF_LOCK_R = 0x4C4F434B;  // Unlock GPIO port F
    GPIO_PORTF_CR_R = 0x01;         // Allow changes to PF0
    GPIO_PORTF_DIR_R |= LED_RED;   // Set the direction of the red LED pin as output
    GPIO_PORTF_DIR_R &= ~SW1;     // Set the direction of switch 1 pin as input
    GPIO_PORTF_DEN_R = 0x1F;     // Enable digital function for pins PF0-PF4
    GPIO_PORTF_PUR_R = 0x11;    // Enable pull-up resistors for switch 1 and switch 2
    while (1)                  // Infinite loop
    {
        state = GPIO_PORTF_DATA_R & 0x10;       // Read the state of switch 1
        if (state == 0x00)                     // If switch 1 is pressed (active low)
        {
            GPIO_PORTF_DATA_R |= LED_RED;     // Turn on the red LED
        }
 else
 {
            GPIO_PORTF_DATA_R &= ~LED_RED;   // Turn off the red LED
        }
    }
}

