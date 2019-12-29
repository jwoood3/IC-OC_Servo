/*
 * File:   outca003_Lab4main.c
 * Author: User
 *
 * Created on March 11, 2019, 9:29 PM
 */


#include "xc.h"
#include <p24Fxxxx.h>
#include <xc.h>
#include "woodx552_lab4_asmlib.h"
#include <stdlib.h>

// PIC24FJ64GA002 Configuration Bit Settings
// CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1          // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF        // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF          // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF           // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF        // JTAG Port Enable (JTAG port is disabled)


// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator disabled. 
					// Primary Oscillator refers to an external osc connected to the OSC1 and OSC2 pins)
#pragma config I2C1SEL = PRI       // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF       // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON       // OSC2/CLKO/RC15 functions as port I/O (RC15)
#pragma config FCKSM = CSECME      // Clock Switching and Monitor (Clock switching is enabled, 
                                       // Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL      // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))

void initServo(void)
{
    CLKDIVbits.RCDIV = 0;  //Set RCDIV=1:1 (default 2:1) 32MHz or FCY/2=16M
    AD1PCFG = 0xffff;            //sets all pins to digital I/O
    TRISA = 0b0000000000011111;  //set port A to inputs, 
    TRISB = 0b0000000000000000;  //and port B to outputs
    
    RPOR3bits.RP6R = 18; 
  
    IFS0bits.T1IF = 0;
    
    T3CON = 0x0020; // turn off, 1:64 pre, Tcy clock source
    PR3 = 4999;  // combined with 1:64 ==> 200,000, which is 20ms
    TMR3 = 0;
    _T3IF = 0;
    T3CONbits.TON = 1;
    
     _OCTSEL = 1; //choosing TMR3
    OC1R = 375; //1.5ms
    OC1RS = 375; 
    OC1CONbits.OCM2 = 1; 
    OC1CONbits.OCM1 = 1;
    OC1CONbits.OCM0 = 0;
    
    __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
    RPOR3bits.RP6R = 18;  // Use Pin RP6 for Output Compare 1 = "18" (Table 10-3)
    __builtin_write_OSCCONL(OSCCON | 0x40); // lock   PPS

}

void setServo(int Val) 
{
    OC1RS = Val;
}

void delay()
{
    long int curCount = 0;  
    int i; 
    for (i = 0; i < 10; i++)
    {
        curCount = 0; 
        while(curCount < 80000)
        {
            curCount++;
        }
    }
}

void initPushButton(void) 
{
    CLKDIVbits.RCDIV = 0;  //Set RCDIV=1:1 (default 2:1) 32MHz or FCY/2=16M
    AD1PCFG = 0xffff;            //sets all pins to digital I/O
    TRISB = 0b0000000100000000;  //and port B to outputs
    CNPU2bits.CN22PUE = 1; 
    
    IFS0bits.T2IF = 0;
    
    T2CON = 0x0030; // turn off, 1:256 pre, Tcy clock source
    PR2 = 62499;  // combined with 1:256 ==> 1s
    TMR2 = 0;
    _T2IF = 0;
    T2CONbits.TON = 1;
    
   IC1CON = 0; // Turn off and reset internal state of IC1
   IC1CONbits.ICTMR = 1; // Use Timer 2 for capture source
   IC1CONbits.ICM = 0b011; // Turn on and capture every rising edge
   _T2IF = 0; 
    
   IEC0bits.T2IE = 1;
   _IC1IE = 1;
   _IC1IF = 0;
   
   __builtin_write_OSCCONL(OSCCON & 0xbf); // unlock PPS
   RPINR7bits.IC1R = 8;  // Use Pin RP9 = "9", for Input Capture 1 (Table 10-2)
   __builtin_write_OSCCONL(OSCCON | 0x40); // lock   PPS

}

static volatile int overflow = 0; //keeps track of overflows in TMR2

void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    overflow++;
}

volatile long unsigned int curPeriod = 0;  
volatile long unsigned int timeDifference; 
volatile long unsigned int eventTwoTime; 
volatile long unsigned int eventOneTime;
volatile long unsigned int buffer[2] = {16000,16000};
volatile long unsigned int index = 0;

void __attribute__((__interrupt__,__auto_psv__)) _IC1Interrupt(void)
{
   static unsigned long int prevEdge = 0;
   unsigned long int curEdge;
   unsigned long int calcPeriod;
   
   _IC1IF = 0;
   curEdge = IC1BUF + (unsigned long)overflow*(PR2);
   calcPeriod = curEdge - prevEdge;
   
   if (calcPeriod < 125)
   {
       //Debouncing
   }
   
   else if (calcPeriod >= 125)
   {
        buffer[index] = curEdge;
        index++;
        index%=2;
        prevEdge = curEdge;
        
        if(buffer[(index+1)%2] - buffer[index] <= 15625) //0.25 seconds or shorter counts as a double click
        {
           buffer[(index+1)%2] = 160000;
           buffer[index] = 0;
           setServo(250);
           overflow = 0;
           TMR2 = 0;
        }
   }    
   
}

int main(void) {
    
    initServo();
    initPushButton();
    setServo(350);
    while(1)
    {
        //reset the position if nothing has been done for two seconds
        if (overflow > 1) {
            setServo(350);
            overflow = 0;
        }
    }
    return 0;
}
