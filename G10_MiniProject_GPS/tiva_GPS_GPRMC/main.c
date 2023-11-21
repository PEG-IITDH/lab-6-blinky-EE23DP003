/* Mini Project: Send the values of Latitute, Latitude and Time by parsing $GPRMC string received from Neo-6M GPS module
 * Using PE4 (Rx), PE5 (Tx): UART Module 5
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"

#define Red 0X02    // Red LED: PF1
#define Blue 0X04   // Blue LED: PF2
#define Green 0X08  // Green LED: PF3
#define Off 0x00    // LED's off

void PortF_Config(void);   // PORTF GPIO configuration (LED's)
void PortE_Config(void);   // PORTE GPIO configuration (UART5 for GPS module)
void PortA_Config(void);   // PORTA GPIO configuration (UART0 Send Data)
void UART_Config(void);    // UART5 (GPS) and UART0 (Tx Data) Configuration
void UART0_SendString(char *str);  // UART0 transmit string
void UART_Handler(void);   // UART5 Receive Interrupt (GPS Module)
void Data_Parse(void);     // GPS Data Parse Function
void Data_Send(void);      // UART5 Send Data Function

char gps_str[100];       // GPS string data
char str[1];             // UART0 send string char argument
volatile int state;      // state 0 = wait for $; state 1 = check for GPGLL; state 2 = Read till '\r';
volatile int pos=0;      // index for gps_str

char latitudeResult[10], longitudeResult[10], parseValue[12][20], *token, date[9], *time, currentTime[9];
double latitude = 0.0, longitude = 0.0, seconds = 0.0, result = 0.0, minutes = 0.0;
const char sep[1] = ",";                       // Data Separator
int index = 0, degrees, i = 0, j = 0;

void main(void)
{
    SYSCTL_RCGCGPIO_R |= (1<<5);      // Enable and provide a clock to GPIO Port F (LED's)
    SYSCTL_RCGCGPIO_R |= (1<<4);      // Enable and provide a clock to GPIO Port E (UART 5)
    SYSCTL_RCGCUART_R |= (1<<5);      // Enable and provide a clock to UART module 5 in Run mode
    SYSCTL_RCGCUART_R |= 0x01;        // Enable UART0 (Data Transmit to view in Terminal)
    SYSCTL_RCGCGPIO_R |= 0x01;        // Enable and provide a clock to GPIO Port A (UART0)


    PortF_Config();      // GPIO PORTF Configuration
    PortE_Config();     // GPIO PORTE Configuration
    PortA_Config();     // GPIO PORTA Configuration
    UART_Config();      // UART5 and UART0 Configuration

    while(1)
        {
            Data_Parse();   // Parse (Seperate Data) from GPS string
            Data_Send();    // Transmit parsed GPS data to UART0
        }
}

void Data_Parse(void)
{
    /*Parse the GPS $GPRMC Data
     * 0 $GPRMC
     * 1 UTC Time
     * 2 Status
     * 3 Latitude
     * 4 N/S
     * 5 Longitude
     * 6 E/W
     * 7 Speed Over Ground
     * 8 Course Over Ground
     * 9 Date
     * 10 Magnetic Variation
     * 11 E/W indicator
     * 12 Mode
     * 13 Checksum
     * 14 End of message
     */

    index = 0;                          // index for parseValue initialised to 0
    token = strtok(gps_str, sep);       // Seperate 'gps_str' string into tokens with delimiter as ','
    while (token != NULL)               // While token is not empty
    {
        //parseValue is a matrix where the data is stored token wise in rows
        strcpy(parseValue[index], token);   // Copy data from token to parseValue[index]
        token = strtok(NULL, sep);          // Increment the index to read next parseValue[index]
        index++;                            //Go to next row
    }
}

void Data_Send(void)
{
    // If parseValue[2] = A, the data is valid. If V, the data is not valid.
    if (strcmp(parseValue[2], "A") == 0) // strcmp returns 0 if the two strings match
    {
        //Send Latitude
        UART0_SendString("Latitude = ");  // Send string "Latitude"
        UART0_SendString(parseValue[3]);  // Latitude value is parseValue[3] in ddmm.mmmm format
        UART0_SendString(" ");
        UART0_SendString(parseValue[4]); // Send Latitude N/S indicator
        UART0_SendString(" ");
        UART0_SendString(";");
        UART0_SendString(" ");


        //Send Longitude
        UART0_SendString("Longitude = "); // Send string "Longitude"
        UART0_SendString(parseValue[5]); // Longitude value is parseValue[3] in dddmm.mmmm format
        UART0_SendString(" ");
        UART0_SendString(parseValue[6]); // Send Longitude E/W indicator
        UART0_SendString(" ");
        UART0_SendString(";");
        UART0_SendString(" ");


        //Send Time
        UART0_SendString("Time = ");     // Send string "Time"
        UART0_SendString(parseValue[1]); // Time value is parseValue[5] in hhmmss.sss UTC format
        UART0_SendString(" ");
        UART0_SendString("UTC");
        UART0_SendString(" ");
        UART0_SendString(";");
        UART0_SendString(" ");

        //Send Date
        UART0_SendString("Date = ");     // Send string "Date"
        UART0_SendString(parseValue[9]); // Date value is parseValue[9] in ddmmyy format
        UART0_SendString(" ");
        UART0_SendString(";");
        UART0_SendString(" ");

        //Send Speed Over Ground
        UART0_SendString("Speed = ");     // Send string "Speed "
        UART0_SendString(parseValue[7]); // Speed value is parseValue[7]
        UART0_SendString(" ");
        UART0_SendString("Knots");
        UART0_SendString(";");
        UART0_SendString(" ");

        UART0_SendString("\n \r "); // New Line, Carriage Return
    }
}

void PortF_Config(void)                 // PORTF (LED) Configuration
{
    GPIO_PORTF_LOCK_R = 0x4C4F434B;     // Unlock PortF register
    GPIO_PORTF_CR_R = 0x1F;             // Enable Commit function

    GPIO_PORTF_PUR_R = 0x11;            // Pull-up for user switches
    GPIO_PORTF_DEN_R = 0x1F;            // Enable all pins on port F
    GPIO_PORTF_DIR_R = 0x0E;            // Define PortF LEDs as output and switches as input
}

void UART_Config(void)                  // UART5 and UART0 Configuration
{
    /*
    *BRDI = integer part of the BRD; BRDF = fractional part
    *BRD = BRDI + BRDF = UARTSysClk / (ClkDiv * Baud Rate)
    *UARTSysClk = 16MHz, ClkDiv = 16, Baud Rate = 9600
    *BRD = 104.167; BRDI = 104; BRDF = 167;
    *UARTFBRD[DIVFRAC] = integer(BRDF * 64 + 0.5) = 11
    */

    //PortE (UART5) configuration
    UART5_CTL_R &= (0<<0);                       //Disable UART module 5
    UART5_IBRD_R = 104;
    UART5_FBRD_R = 11;
    UART5_CC_R = 0x00;                          // System Clock
    UART5_LCRH_R |= 0x60;                       // 8 bit word length, FIFO disable, Parity disable
    UART5_CTL_R |= ((1<<0)|(1<<8)|(1<<9));     // Enable UART module 5

    //UART5 interrupt configuration
    UART5_IM_R &= ((0<<4)|(0<<5)|(0<<8));       // Mask Tx, Rx and Parity interrupts
    UART5_ICR_R &= ((0<<4)|(0<<5)|(0<<8));      // Clear Tx, Rx and Parity interrupts
    UART5_IM_R |= (1<<4);                       // Enable Rx interrupt
    NVIC_EN1_R |= (1<<29);                      // Interrupts enabled for UART5
    NVIC_PRI15_R &= 0xFFFF5FFF;                 // Interrupt Priority 2 to UART5

    //PortA (UART0) configuration
    UART0_CTL_R &= ~UART_CTL_UARTEN; // Disable UART0 during configuration
    UART0_IBRD_R = 104;             // Integer part of the baud rate
    UART0_FBRD_R = 11;             // Fractional part of the baud rate
    UART0_LCRH_R = (UART_LCRH_WLEN_8 | UART_LCRH_FEN); // 8-bit word length, enable FIFO
    UART0_CTL_R |= (UART_CTL_UARTEN | UART_CTL_RXE | UART_CTL_TXE); // Enable UART0, RX, and TX

    state=0;                     // initialise state as 0
}

void PortE_Config(void)      // PORTE (UART 5) Configuration
{
    GPIO_PORTE_LOCK_R = 0x4C4F434B;     // Unlock PortE register
    GPIO_PORTE_CR_R = 0xFF;             // Enable Commit function
    GPIO_PORTE_DEN_R = 0xFF;            // Enable all pins on port E
    GPIO_PORTE_DIR_R |= (1<<5);         // Define PE5 as output
    GPIO_PORTE_AFSEL_R |= 0x30;         // Enable Alternate function for PE4 and PE5
    GPIO_PORTE_PCTL_R |= 0x00110000;    // Selecting UART function for PE4 and PE5
}

void PortA_Config(void)   // Port A configuration (UART0)
{
    GPIO_PORTA_LOCK_R = 0x4C4F434B;    // Unlock PortA register
    GPIO_PORTA_CR_R = 0xFF;           // Enable Commit function
    GPIO_PORTA_DEN_R = 0xFF;         // Enable all pins on port A
    GPIO_PORTA_DIR_R |= (1 << 1);   // Define PA1 as output
    GPIO_PORTA_AFSEL_R |= 0x03;    // Enable Alternate function for PA0 and PA1
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) | 0x00000011; // Selecting UART function for PA0 and PA1
}

void UART0_SendString(char *str)       // UART0 String Send Function
{
    while (*str)
    {
        // Send each character of the string
        while ((UART0_FR_R & 0x20) != 0); // Wait as long as Transmit FIFO (TXFF) is full
        UART0_DR_R = *str; // Send the character to UART0 Data Register
        str++;             // Go to next character in the string
    }
}


void UART_Handler(void)         // UART Receive interrupt handler
{
    UART5_IM_R &= (0<<4);       //Mask UART Rx interrupt

    // state 0 = wait for '$'; state 1 = check for GPGLL received; state 2 = Read till '\r'

    if(state == 0)              // state 0 = wait for '$'
    {
        GPIO_PORTF_DATA_R = Red;      // Turn on RED LED
        pos=0;                        // position index for gps_str
        gps_str[pos]=UART5_DR_R;      // Send UART5 Data Register
        if(gps_str[pos]=='$')
        {
            state=1;        // Change state to 1 if '$' receieved
            pos++;         // increment pos value
        }
        else
        {
            pos=0;
            //UART5_ICR_R |= (1<<4);                  //Clear UART Rx Interrupt
            //UART5_IM_R |= (1<<4);                   //UnMask UART Rx interrupt
        }
    }

    if(state == 1)                  // state 1 = check if GPRMC received
    {
        GPIO_PORTF_DATA_R = Blue;  // Turn on Blue LED (PF3) in state 1

        gps_str[pos]=UART5_DR_R;  // Assign UART5 Data Register value to gps_str[pos]
        pos++;                   // Increment pos value

        if(pos==7)             // Check if pos value is 7
        {
            if((gps_str[2] == 'G') && (gps_str[3] == 'P') && (gps_str[4] == 'R') && (gps_str[5] == 'M') && (gps_str[6] == 'C'))
            {
                state = 2;   // Change state to 2 if 'GPRMC' received
                pos--;      // Decrement position by 1
            }
            else
            {
                state = 0;  // Change 'state' to 0 if 'GPGLL' not received
                pos = 0;    // Change 'pos' to 0 if 'GPGLL' not received
            }
       }
        //UART5_ICR_R |= (1<<4);     // Clear UART Rx Interrupt
        //UART5_IM_R |= (1<<4);     // UnMask UART Rx interrupt

    }

    if(state == 2)                   // state 2 = Read till '\r'
    {
        GPIO_PORTF_DATA_R = Green;   // Turn on GREEN LED (PF2) in state 2
        gps_str[pos]=UART5_DR_R;     // Assign UART5 Data Register value to gps_str[pos]

        if((gps_str[pos]=='\r') || (gps_str[pos]=='\n')) // Check if gps_str[pos] = Carriage return (\r)
        {
            state=0;  // Change state to 0
            pos=0;    // Change pos to 0
        }
        else
        {
            pos++;  // increase 'pos' value if gps_str[pos] not equal to '\r'
        }
    }
    GPIO_PORTF_DATA_R = Off;                // Turn off all LED's
    UART5_ICR_R |= (1<<4);                  // Clear UART Rx Interrupt
    UART5_IM_R |= (1<<4);                   // UnMask (Enable) UART Rx interrupt
}
