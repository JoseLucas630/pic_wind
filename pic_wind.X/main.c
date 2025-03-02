/* 
 * File:   main.c
 * Author: JoseLucas630
 *
 * Created on 15 de Fevereiro de 2025, 10:23
 */

//==============configs=================//

// CONFIG1H
#pragma config OSC = HS          // Oscillator Selection bits (HS oscillator)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable bit (Fail-Safe Clock Monitor enabled)
#pragma config IESO = ON        // Internal External Oscillator Switchover bit (Internal External Switchover mode enabled)

// CONFIG2L
#pragma config PWRTEN = ON      // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = OFF      // Brown-out Reset Enable bits (Brown-out Reset disabled)
// BORV = No Setting

// CONFIG2H
#pragma config WDTEN = OFF       // Watchdog Timer Enable bit (WDT enabled)
#pragma config WDPS = 32768     // Watchdog Timer Postscale Select bits (1:32768)
#pragma config WINEN = OFF      // Watchdog Timer Window Enable bit (WDT window disabled)

// CONFIG3L
#pragma config PWMPIN = OFF     // PWM output pins Reset state control (PWM outputs disabled upon Reset (default))
#pragma config LPOL = HIGH      // Low-Side Transistors Polarity (PWM0, 2, 4 and 6 are active-high)
#pragma config HPOL = HIGH      // High-Side Transistors Polarity (PWM1, 3, 5 and 7 are active-high)
#pragma config T1OSCMX = ON     // Timer1 Oscillator MUX (Low-power Timer1 operation when microcontroller is in Sleep mode)

// CONFIG3H
#pragma config MCLRE = ON       // MCLR Pin Enable bit (Enabled)

// CONFIG4L
#pragma config STVREN = ON      // Stack Full/Underflow Reset Enable bit (Stack full/underflow will cause Reset)
#pragma config LVP = ON         // Low-Voltage ICSP Enable bit (Low-voltage ICSP enabled)

// CONFIG5L
#pragma config CP0 = OFF        // Code Protection bit (Block 0 (000200-000FFFh) not code-protected)
#pragma config CP1 = OFF        // Code Protection bit (Block 1 (001000-001FFF) not code-protected)
#pragma config CP2 = OFF        // Code Protection bit (Block 2 (002000-002FFFh) not code-protected)
#pragma config CP3 = OFF        // Code Protection bit (Block 3 (003000-003FFFh) not code-protected)

// CONFIG5H
#pragma config CPB = OFF        // Boot Block Code Protection bit (Boot Block (000000-0001FFh) not code-protected)
#pragma config CPD = OFF        // Data EEPROM Code Protection bit (Data EEPROM not code-protected)

// CONFIG6L
#pragma config WRT0 = OFF       // Write Protection bit (Block 0 (000200-000FFFh) not write-protected)
#pragma config WRT1 = OFF       // Write Protection bit (Block 1 (001000-001FFF) not write-protected)
#pragma config WRT2 = OFF       // Write Protection bit (Block 2 (002000-002FFFh) not write-protected)
#pragma config WRT3 = OFF       // Write Protection bit (Block 3 (003000-003FFFh) not write-protected)

// CONFIG6H
#pragma config WRTC = OFF       // Configuration Register Write Protection bit (Configuration registers (300000-3000FFh) not write-protected)
#pragma config WRTB = OFF       // Boot Block Write Protection bit (Boot Block (000000-0001FFh) not write-protected)
#pragma config WRTD = OFF       // Data EEPROM Write Protection bit (Data EEPROM not write-protected)

// CONFIG7L
#pragma config EBTR0 = OFF      // Table Read Protection bit (Block 0 (000200-000FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR1 = OFF      // Table Read Protection bit (Block 1 (001000-001FFF) not protected from table reads executed in other blocks)
#pragma config EBTR2 = OFF      // Table Read Protection bit (Block 2 (002000-002FFFh) not protected from table reads executed in other blocks)
#pragma config EBTR3 = OFF      // Table Read Protection bit (Block 3 (003000-003FFFh) not protected from table reads executed in other blocks)

// CONFIG7H
#pragma config EBTRB = OFF      // Boot Block Table Read Protection bit (Boot Block (000000-0001FFh) not protected from table reads executed in other blocks)

//======================================//

////////////////////////////////////////////////////////////////////////////////

#include <xc.h>
#include <stdlib.h>

#define _XTAL_FREQ 8000000   //8.000.000 hertz = 8 megahertz
#define DHT11_NUM 3

unsigned char check = 0, t_byte1 = 0, t_byte2 = 0, rh_byte1 = 0, rh_byte2 = 0, sum = 0;
unsigned char RH, TEMP;

////////////////////////////////////////////////////////////////////////////////

void start_DHT11(void)
{
    /*
     *  this is the starting point. According to dht11 datasheet, you first need to
     * send a low voltage for 18ms, and the pull up it for 40us.
     * TRISB is the data direction register for the IO pins. TRISB is the data
     * direction register of PORTB. by setting PIN B 3 as a 0, it's set as output.
     */
    TRISBbits.TRISB3 = 0;
    
    LATBbits.LATB3 = 0;        //now, write low for 18ms
    __delay_ms(18);
    LATBbits.LATB3 = 1;       //pull up
    __delay_us(40);           //for about 40us(microseconds)
    TRISBbits.TRISB3 = 1;     //set it as a input, because it will now receive signal
}

////////////////////////////////////////////////////////////////////////////////

void check_DHT11(void)
{
    /*
     * now the part that checks if the sensor has answered us.
     * "Once DHT detects the start signal, it will send out a low-voltage-level response signal, which
     * lasts 80us. Then the programme of DHT sets Data Single-bus voltage level from low to high and
     * keeps it for 80us for DHT?s preparation for sending data."
     * This is the datasheet. 
     * this code checks if the input is low, if it is, delay it for 80us.
     * after the delay, the pin will again the check the input, but this time to
     * find if it's high, it it is, the program delay again for 80us and change
     * "check" to 1.
     */
    check = 0;   //just to check :p
    
    __delay_us(40);
    if(PORTBbits.RB3 == 0) //detect if the port is receiving a low signal
    { 
        //if it is, delay and wait for the high voltage
        __delay_us(80);
        if(PORTBbits.RB3 == 1)  //check for high voltage
        {
            //high voltage checked. delay it.
            check = 1;
            __delay_us(80);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

char read_dht11(void)
{
    /*
     * Well, this code is not mine. i just copied and pasted it from a tutorial.
     * It measures the time of the signal.
     * according to datasheet:
     * 50us low and then 70us high: 1
     * 50us low and then 28-29us high: 0
     * by this, the code implements a function to measure the lenght of signal
     * and return data with it value
     */
    char data = 0, for_count;
 
    for(for_count = 0; for_count < 8; for_count++)
    {
        //this function here is the key. it waits for the signal to go high and start reading
        while(!PORTBbits.RB3);
       __delay_us(30);
       /*
        * it waits 30us for a parameter.
        * if 30us passed, it reads the port;
        * if the port is low, the data is write as 0
        * if the port is still high, data is write as 1
       */
       if(PORTBbits.RB3 == 0)
       {
           data&= ~(1<<(7 - for_count)); //Clear bit (7-b)
       }
       else
       {
           data|= (1 << (7 - for_count)); //Set bit (7-b)
           while(PORTBbits.RB3);
       } //Wait until PORTB3
    }
    return data;   //return the data in a char
 }

////////////////////////////////////////////////////////////////////////////////

void main(void)
{
    ADCON0 = 0x00; //Turns the ADC off because i won't be needed
    
    TRISBbits.TRISB4 = 0;  //my relay port set as output
    int temp1;            //variable for storing the temperature
    
    while(1) //infinite
    {
        start_DHT11();   //start dht11
        check_DHT11();   //check dht11
        if(check == 1)    //if check = 1, it means the process gone right
        {
            /*
             * "Data consists of decimal and integral parts. A complete data transmission is 40bit, and the
             * sensor sends higher data bit first.
             * Data format: 8bit integral RH data + 8bit decimal RH data + 8bit integral T data + 8bit decimal T
             * data + 8bit check sum. If the data transmission is right, the check-sum should be the last 8bit of
             * "8bit integral RH data + 8bit decimal RH data + 8bit integral T data + 8bit decimal T data"."
             * 
             * each read is associated as of one of the chars
             */
            rh_byte1 = read_dht11();
            rh_byte2 = read_dht11();
            t_byte1 = read_dht11();
            t_byte2 = read_dht11();
            sum = read_dht11();
            
            if(sum == ((rh_byte1 + rh_byte2 + t_byte1 + t_byte2)))
             {
                /*
                 * the sum needs to be literally the sum of all number
                 */
                TEMP = t_byte1; 
                RH = rh_byte1;
             }
            
            /*
             * now, just cast the char to a int and the temperature is read to be used!
             * this variable is probably useless. I could just used RH as a int and 
             * use it...
             */
            temp1 = (int)TEMP; 
        }
       
        /*
         * as the final thing, the program sees if the temperature is higher than 28
         * degrees celsius. if it is, turn on the fan!
         */
        if (temp1 >= 28)
        {
            PORTBbits.RB4 = 1; //relay
        }
        else
        {
            PORTBbits.RB4 = 0; //not today
        }

        __delay_ms(1000);   //wait a second for another read.
    }
}