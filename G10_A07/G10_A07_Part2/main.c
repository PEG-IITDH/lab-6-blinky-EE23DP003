/* Lab 7 Part 2:  PWM Waveform 100KHz with variable duty cycle on pin PF2 (Blue LED)
Initial duty cycle=50%, SW1: short press increases duty cycle by 5%, long press decreases duty by 5% */

#include <stdint.h>
#include "tm4c123gh6pm.h"

#define PWM_FREQ 100000                     // 100 KHz PWM Frequency
#define PWM_PERIOD (16000000 / PWM_FREQ)    // PWM Period = 160
#define PWM_START_DUTY 50                  // Start duty cycle

//Definitions for SysTick CSR (Control and Status Register)
#define ENABLE (1 << 0) // CSR bit 0 to enable the SysTick timer
#define INT_EN (1<<1) // CSR bit 1 to generate interrupt to NVIC when Systick reaches 0
#define CLK_SRC (1<<2) // CSR bit 2 to take system clock
#define COUNT_FLAG (1<<16)  // CSR bit 16,  1 -> SysTick counted to zero

volatile uint32_t Duty = PWM_START_DUTY;  // Initialize current duty cycle to 50

void GPIO_Init(void);   // GPIO Init function declaration
void PWM_Init(void);   // PWM Init function declaration

void main(void)
{

    GPIO_Init();  //Initialise GPIO for pin PF4
    PWM_Init();   //Initialise PWM for pin PF4

    while (1)
    {
        //wait indefinitely
    }
}


void GPIO_Init(void)
{
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;            // Enable GPIO Port F (for SW1)
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0); // Wait for GPIO PortF peripheral to be ready

    // Configure SW1 (PF4) as input
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; // Unlock GPIO PortF
    GPIO_PORTF_CR_R = 0x01;           // Uncommit and Allow changes to PF0 (SW2 not used)
    GPIO_PORTF_DIR_R &= ~(0x10);     // Direction: PF4 as input
    GPIO_PORTF_DEN_R |= 0x10;       // Digital Enable: bit 4
    GPIO_PORTF_PUR_R |= 0x10;      // Pull-up resistor enabled on PF4

    // Configure GPIO interrupt for Port F (Switch) PF0
    GPIO_PORTF_IS_R &= ~0x10;     // Interrupt Sense: Edge-sensitive
    GPIO_PORTF_IBE_R &= ~0x10;   // Interrupt Both Edges: Not both edges
    GPIO_PORTF_IEV_R &= ~0x10;  // Interrupt Event: Falling edge event
    GPIO_PORTF_ICR_R |= 0x10;  // Interrupt Clear: Clear the interrupt flags for PF4
    GPIO_PORTF_IM_R |= 0x10;  // Interrupt Mask: Unmask (Enable) interrupt on PF4
    NVIC_EN0_R |= 1 << 30;   // Interrupt Enable: Enable interrupt for GPIO Port F (bit 30)
}


void PWM_Init(void)
{
    SYSCTL_RCGCPWM_R |= SYSCTL_RCGCPWM_R1;            // Enable the PWM1 module
    while ((SYSCTL_PRPWM_R & SYSCTL_PRPWM_R1) == 0); // Wait for PWM1 module to be ready


    //Configure PF2 (M1PWM6) as PWM Output
    GPIO_PORTF_AFSEL_R |= 0x04;          // Enable alternate function on PF2
    GPIO_PORTF_PCTL_R &= ~0x00000F00;   // Port Control: Clear PF2
    GPIO_PORTF_PCTL_R |= 0x00000500;   // Port Control: Configure PF2 as M1PWM6
    GPIO_PORTF_DIR_R |= 0x04;         // Direction: Make PF2 as output
    GPIO_PORTF_DEN_R |= 0x04;        // Digital enable PF2

    //Configure Module 1 PWM Generator 3 which controls Module 1 PWM6 (M1PWM6) on pin PF2
    PWM1_3_CTL_R = 0;             // Disable PWM while configuring
    PWM1_3_GENA_R = 0x0000008C;  /* Down Count: M1PWM6 output is high at the start of the period
                                   and low when the counter matches comparator A */
    PWM1_3_LOAD_R = PWM_PERIOD - 1;             // Set PWM period
    PWM1_3_CMPA_R = (PWM_PERIOD * Duty) / 100; // Set Compare A value, consider initial duty
    PWM1_3_CTL_R |= 0x00000001;               // Enable PWM1 Generator 3
    PWM1_ENABLE_R |= 0x00000040;             // Enable Module1 PWM6 (M1PWM6) output
}


void SysTick_Init(void)
{
NVIC_ST_CURRENT_R = 0x00; // SysTick current value register cleared
NVIC_ST_RELOAD_R = 16000000; // Reload value when counter reaches 0 (1sec delay)
NVIC_ST_CTRL_R |= (ENABLE | INT_EN | CLK_SRC); // Enable Systick, Interrupt and use System clock
}

void GPIOF_Handler(void)
{
    GPIO_PORTF_IM_R &= ~0x10;     // Disable interrupt from PF4
    SysTick_Init();

}

void SysTick_Handler(void)
{
    NVIC_ST_CTRL_R = 0;  // Disable the SysTick timer
        if ( (GPIO_PORTF_DATA_R & (1<<4)) == 0 )   // Long Press: Check if switch SW1 still pressed after one sec
        {
            if (Duty < 95)      // Check if duty cycle is less than max value 95
                {
                    Duty += 5; // Increase duty by 5, this increases CMPA value and decreases duty cycle
                }

        }
        else                       // Short Press of SW1
        {
            if (Duty > 5)         // Check if duty cycle is greater than min value 5%
                {
                    Duty -= 5;    // Decrease duty by 5, this decreases CMPA value and increases duty cycle
                }
        }


    PWM1_3_CMPA_R = (PWM_PERIOD * Duty) / 100; // Update PWM1 Generator 3 Compare A value
    GPIO_PORTF_ICR_R |= 0x10; // Clear GPIO interrupt for PF4 (SW1)
    GPIO_PORTF_IM_R |= 0x10;  // Unmask (Enable) interrupts from PF4 (SW1)
}
