#include "tm4c123gh6pm.h" // Include the header file for the TM4C123GH6PM board
#include <stdint.h>       // Include the standard integer types header file

void PortF_Init(void);  // Port F initialization function declaration
void Delay(void);      // Debounce delay function declaration

int main(void)
{
    PortF_Init();             // Initialize Port F
    int lastButtonState = 1; // Variable to store the last state of the button
    while (1)               // Infinite loop
     {
        int buttonState = (GPIO_PORTF_DATA_R & 0x10) >> 4; // Read the current state of the button SW1
        if (buttonState == 0 && lastButtonState == 1) // If the button is pressed and was not pressed in the last iteration
        {
            Delay();                                     // Debounce the button press
            if ((GPIO_PORTF_DATA_R & 0x02) == 0x02)     // If red LED is on
            {
                GPIO_PORTF_DATA_R = 0x04;              // Turn on blue LED
            }
            else if ((GPIO_PORTF_DATA_R & 0x04) == 0x04) // If blue LED is on
            {
                GPIO_PORTF_DATA_R = 0x08;               // Turn on green LED
            } else
            {
                GPIO_PORTF_DATA_R = 0x02;              // Turn on red LED
            }
        }
        lastButtonState = buttonState; // Update the last state of the button
    }
}

void PortF_Init(void)
{
    SYSCTL_RCGC2_R |= 0x00000020;    // Enable clock for Port F
    GPIO_PORTF_LOCK_R = 0x4C4F434B; // Unlock Port F
    GPIO_PORTF_CR_R = 0x1F;        // Allow changes to PF4-0
    GPIO_PORTF_DIR_R = 0x0E;      // Set PF3-1 as output, PF4 and PF0 as input
    GPIO_PORTF_PUR_R = 0x11;     // Enable pull-up resistors for PF4 and PF0
    GPIO_PORTF_DEN_R = 0x1F;    // Enable digital function for PF4-0
}

void Delay(void)         // Delay to implement debounce
{
    int time = 1000000; // Variable for delay
    while (time)
    {
        time--;        // Decrement time until it reaches zero
    }
}
