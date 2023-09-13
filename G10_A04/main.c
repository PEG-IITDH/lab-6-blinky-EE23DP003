/* Lab-4: Systick timer interrupt and GPIO interrupt to generate a precisely 1 second long LED pulse each time a button is pressed */

#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>


//Definitions for SysTick CSR (Control and Status Register)
#define ENABLE (1 << 0)      // CSR bit 0 to enable the SysTick timer
#define INT_EN (1<<1)       // CSR bit 1 to generate interrupt to NVIC when Systick reaches 0
#define CLK_SRC (1<<2)     // CSR bit 2 to take system clock

//GPIO and Systick initialization and interrupt handler function declarations
void GPIO_PORTF_Init(void);       //GPIO Port F initialization
void Systick_Init(void);          //SysTick initialization
void GPIO_PORTF_Handler(void);   //GPIO Port F interrupt handler
void SystickHandler(void);       // GPIO Port F Systick handler


int main(void)
{

    GPIO_PORTF_Init();        //Initialize GPIO Port F

    //NVIC Configuration
    NVIC_EN0_R |= (1<<30);   //Interrupts enabled for PortF --> interrupt number 30
    NVIC_PRI7_R |= (0<<21) | (0<<22) | (0<<23) ; //Interrupt 30 set to priority highest (0) priority

    while(1);               // wait indefinitely

}


void GPIO_PORTF_Init(void)
{
    SYSCTL_RCGC2_R |= 0x00000020;   //enable clock to GPIOF
    GPIO_PORTF_LOCK_R = 0x4C4F434B; //Unlock GPIO Port F commit register
    GPIO_PORTF_CR_R = 0x1F;         //Make PORTF configurable

    GPIO_PORTF_DEN_R = 0x1F;        //Enable digital functions on PortF
    GPIO_PORTF_DIR_R = 0x0E;        //Set LEDs as outputs (1), switches as inputs (0)
    GPIO_PORTF_PUR_R = 0x11;        //Pull-up resistor for user switches enabled

    // Enabling Interrupts

     //GPIOIS -- Interrupt Sense  --> 0 for edge detect                           --> 0x00
     GPIO_PORTF_IS_R = 0x00;

     //GPIOIBE -- Both Edges      --> 0 for GPIOIEV controls interrupt generation --> 0x00
     GPIO_PORTF_IBE_R = 0x00;

     //GPIOIEV -- Interrupt Event --> 0 for falling edge triggers interrupt       --> 0x00
     GPIO_PORTF_IEV_R = 0x00;

     //GPIOIM -- Interrupt Mask   --> 0 for masked interrupt (disabled)           --> 0x00
     GPIO_PORTF_IM_R = 0x00;

     //GPIOICR -- Interrupt clear --> cleared by writing 1 to corresponding bit   --> 0x11
     GPIO_PORTF_ICR_R = 0x11;

     //GPIOIM -- Interrupt Mask   --> 1 for unmasked interrupt (enabled)          --> 0x11
     GPIO_PORTF_IM_R = 0x11;


     //GPIORIS -- Raw Int Status (Read Only)
     //GPIOMIS -- Masked Int Status (Read Only)
}


void Systick_Init(void)
{
    NVIC_ST_CURRENT_R = 0x00;                       // SysTick current value register cleared
    NVIC_ST_RELOAD_R = 16000000;                   // Reload value when counter reaches 0 (1sec delay)
    NVIC_ST_CTRL_R |= (ENABLE | INT_EN | CLK_SRC); //Enable Systick, Interrupt and use System clock
}


void GPIO_PORTF_Handler(void)        // called whenever GPIO interrupt occurs
{
        GPIO_PORTF_ICR_R = 0x11;    // Clear interrupts from PF0 and PF4
        GPIO_PORTF_IM_R = 0x00;     // Interrupts from PortF masked (disabled)
        Systick_Init();            // Call the Systick function
        GPIO_PORTF_DATA_R = 0x0E;  // Turn on Red (PF1), Blue (PF2) and Green (PF3) LEDs
}


void SystickHandler(void)        // called when Systick reaches 0 and interrupt gets triggered
{
    GPIO_PORTF_DATA_R &=~ 0x0E; //turn off Red, Blue and Green LEDs
    GPIO_PORTF_ICR_R = 0x11;   // Clear any pending interrupts from PF0 and PF4
    GPIO_PORTF_IM_R = 0x11;   // Interrupts from pin PF0 and PF4 unmasked (enabled)
}

