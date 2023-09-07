#include <stdint.h>
#include "tm4c123gh6pm.h"
int main(void)
{
    SYSCTL_RCGCGPIO_R |= 0x02;   // Enable clock to GPIOB
    GPIO_PORTB_DIR_R |= 0x02;   //  Set PB1 as output
    GPIO_PORTB_DEN_R |= 0x02;  //   Enable digital function for PB1

    int i;
    while(1)
    {
        GPIO_PORTB_DATA_R = 0x02;          // Set port PB1
            for(i = 0; i <800; i++){}     //  Wait for 0.5 millisecond
        GPIO_PORTB_DATA_R = 0x00;        //   Clear port PB1
            for(i = 0; i <800; i++){}   //    Wait for 0.5 millisecond
    }

}
