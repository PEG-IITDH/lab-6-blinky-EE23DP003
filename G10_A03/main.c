#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"


//Definitions for SysTick CSR (Control and Status Register)
#define ENABLE (1 << 0)         // CSR bit 0 to enable the SysTick timer
#define CLK_SRC (1<<2)         // CSR bit 2 to take system clock
#define COUNT_FLAG  (1 << 16) // CSR bit 16 automatically set to 1 when SysTick reaches 0
#define CLOCK_MHZ 16         // Default System Clock

// GPIO PORTF and Systick function declarations
void GPIO_PORTF_Init (void);        //GPIO PORTF Initialization Function
void Systick_Delay (uint32_t us);  //Systick Delay function

/* Main Program */

int main(void)
{
    GPIO_PORTF_Init ();
    while(1)                          //implement continuously
    {
        GPIO_PORTF_DATA_R = 0X0E;    // Red, Blue and Green LED turned on
        Systick_Delay (200);        // Call Delay function
        GPIO_PORTF_DATA_R = 0X00;  // All pins on Port F turned off
        Systick_Delay (800);      // Call Delay function
    }
}

void GPIO_PORTF_Init (void)
{
    SYSCTL_RCGC2_R |= 0x00000020;      // enable clock to GPIOF
    GPIO_PORTF_LOCK_R = 0x4C4F434B;   // unlock GPIO Port F commit register
    GPIO_PORTF_CR_R = 0x1F;          // make PORTF configurable
    GPIO_PORTF_DEN_R = 0x1F;        // enable digital functions on PF 0,1,2,3,4
    GPIO_PORTF_DIR_R = 0x0E;       // set PF1,PF2,PF3 (LEDs) as output
}


/* Delay Function */
void Systick_Delay (uint32_t us)
{
    NVIC_ST_CURRENT_R = 0;                  // SysTick current value register cleared
    NVIC_ST_RELOAD_R = us*CLOCK_MHZ;       // SysTick reload value for 'us' microseconds
    NVIC_ST_CTRL_R |= ( ENABLE | CLK_SRC );  // SysTick Enable, use system clock

    while ((NVIC_ST_CTRL_R & COUNT_FLAG) == 0)  // wait until systick count flag is set
    {
           // do nothing
    }
    NVIC_ST_CTRL_R = 0;                // Disable SysTick when COUNT_FLAG is set
}
