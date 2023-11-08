/* Lab7 Part1:  PWM Waveform 100KHz with variable duty cycle on pin PF2 (Blue LED)
Initial duty cycle=50%, SW1 decreases duty cycle by 5%, SW2 increases duty cycle by 5% */

#include <stdint.h>
#include "tm4c123gh6pm.h"

#define PWM_FREQ 100000                      // 100 KHz PWM Frequency
#define PWM_PERIOD (16000000 / PWM_FREQ)    // PWM Period = 160
#define PWM_START_DUTY 50                  // Start duty cycle

volatile uint32_t Duty = PWM_START_DUTY;  // Initialize duty cycle to 50

void GPIO_Init(void);   // GPIO Init function declaration
void PWM_Init(void);   // PWM Init function declaration


void main(void)
{

    GPIO_Init();  //Initialize GPIO for pin PF0, PF4
    PWM_Init();   //Initialize PWM for pin PF0, PF4


    while (1)
    {
        //wait indefinitely
    }
}


void GPIO_Init(void)
{
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;            // Enable GPIO Port F (for SW1 and SW2)
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0); // Wait for GPIO PortF peripheral to be ready

    // Configure SW1 (PF4) and SW2 (PF0) as inputs
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY; // Unlock GPIO PortF
    GPIO_PORTF_CR_R = 0x01;           // Uncommit and Allow changes to PF0 (SW2)
    GPIO_PORTF_DIR_R &= ~(0x11);     // Direction: bits 0 and 4 set as inputs
    GPIO_PORTF_DEN_R |= 0x11;       // Digital Enable: bits 0 and 4
    GPIO_PORTF_PUR_R |= 0x11;      // Pull-up resistors enabled on PF0 AND PF4

    // Configure GPIO interrupt for Port F (Switches)
    GPIO_PORTF_IS_R &= ~0x11;     // Interrupt Sense: Edge-sensitive
    GPIO_PORTF_IBE_R &= ~0x11;   // Interrupt Both Edges: Not both edges
    GPIO_PORTF_IEV_R &= ~0x11;  // Interrupt Event: Falling edge event
    GPIO_PORTF_ICR_R |= 0x11;  // Interrupt Clear: Clear the interrupt flags for PF0 , PF4
    GPIO_PORTF_IM_R |= 0x11;  // Interrupt Mask: Unmask (Enable) interrupt on PF0 , PF4
    NVIC_EN0_R |= 1 << 30;   // Interrupt Enable: Enable interrupt for GPIO PF0 (bit 30)
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


void GPIOF_Handler(void)               // GPIO PortF Interrupt handler
{
    if ( GPIO_PORTF_RIS_R & 0x10)     // Raw Interrupt Status: Check for PF4 (SW1) interrupt
    {
        if (Duty < 95)               // Check if current duty cycle is less than max value 95
        {
            Duty += 5;              // Increase duty by 5, this increases CMPA value and decreases duty cycle
        }
        GPIO_PORTF_ICR_R = 0x10;   // Clear GPIO interrupt for PF4 (SW1)
    }
    if ( GPIO_PORTF_RIS_R & 0x01)  // Raw Interrupt Status: Check for PF0 (SW2) interrupt
    {
        if (Duty > 5)      // Check if current duty cycle is greater than min value 5%
        {
            Duty -= 5;    // Decrease duty by 5, this decreases CMPA value and increases duty cycle
        }
        GPIO_PORTF_ICR_R = 0x01; // Clear GPIO interrupt for PF0 (SW2)
    }

    PWM1_3_CMPA_R = (PWM_PERIOD * Duty) / 100; // Update PWM1 Generator 3 Compare A value
}
