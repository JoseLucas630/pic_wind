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
    TRISBbits.TRISB3 = 0;
    
    LATBbits.LATB3 = 0;        //PORTBbits.RB3 = 0;
    __delay_ms(18);
    LATBbits.LATB3 = 1;       //PORTBbits.RB3 = 1;
    __delay_us(40);
    TRISBbits.TRISB3 = 1;
}

////////////////////////////////////////////////////////////////////////////////

void check_DHT11(void)
{
    check = 0;
    
    __delay_us(40);
    if(PORTBbits.RB3 == 0) 
    { 
        __delay_us(80);
        if(PORTBbits.RB3 == 1)
        {
            check = 1;
            __delay_us(80);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

char read_dht11(void)
{
    char data = 0, for_count;
 
    for(for_count = 0; for_count < 8; for_count++)
    {
        while(!PORTBbits.RB3); 
       __delay_us(30);
       if(PORTBbits.RB3 == 0)
       {
           data&= ~(1<<(7 - for_count)); //Clear bit (7-b)
       }
       else
       {
           data|= (1 << (7 - for_count)); //Set bit (7-b)
           while(PORTBbits.RB3);
       } //Wait until PORTD.F0 goes LOW
    }
    return data;
 }

////////////////////////////////////////////////////////////////////////////////

void main(void)
{
    ADCON0 = 0x00; //Turns the ADC off
    
    TRISBbits.TRISB4 = 0;
    TRISBbits.TRISB0 = 0;
    
    int temp1;
    
    while(1)
    {
        start_DHT11();
        check_DHT11();
        if(check == 1)
        {
            rh_byte1 = read_dht11();
            rh_byte2 = read_dht11();
            t_byte1 = read_dht11();
            t_byte2 = read_dht11();
            sum = read_dht11();
            
            if(sum == ((rh_byte1 + rh_byte2 + t_byte1 + t_byte2)))
             {
                TEMP = t_byte1;//temp = (int*)malloc(t_byte1 * sizeof(int));
                RH = rh_byte1;//rh = (int*)malloc(rh_byte1 * sizeof(int));
             }
            
            PORTBbits.RB0 = 0;
            temp1 = (int)TEMP;
        }
        
        if (temp1 >= 28)
        {
            PORTBbits.RB4 = 1; // Liga o relé
        }
        else
        {
            PORTBbits.RB4 = 0; // Desliga o relé
        }

        __delay_ms(1000);
    }
}