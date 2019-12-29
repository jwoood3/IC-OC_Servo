# IC-OC_Servo

This code is to be used with the PIC24FJ64GA004 microcontroller and [Sparkfun Micro Servo](https://www.sparkfun.com/products/9065). 
This code uses IC (input capture) and OC (output compare) to detect button presses and use those to control a servo motor. Specifically, this will detect if a rapid double press of a button occurs, and turn a servo motor by a small angle should the double press be detected. A video example can be seen [here](https://imgur.com/a/gsCLYIR).

Some of the functions in the code and how they work are described as follows:

initServo(void)  
This function sets up Output Compare. A 1:64 prescaler is set for Timer 3, and PR3 is set to 4999 to create a 50ms timer length. TMR3 and _T3IF are set to 0 to make sure that they are starting at 0, and then the timer is turned on.
_OCTSEL is used to select Timer 3 for output compare. OC1R is set such that the pulse width will be1.5ms since this sets the servo to the middle, and OC1RS is set to the same value, since it will only be changed later. OCM is chosen to be 110 which is PWM mode. PPS is unlocked and is used to set RP6 to be the pin used for output compare, and then locked again.

setServo(int val)  
This function changes the position of the servo. It does this by changing the value of OC1RS, which causes the pulse width to change. Since the pulse width controls the position of the motor, changing the value of the shadow register will change the position of the motor. setServo should go between 300 and 450, or have a pulse width of between 1.2s and 1.8s.

initPushButton(void)  
This function sets up input capture so that the detection of a rising edge from a button press triggers an interrupt.
This function sets the clock divider, sets all the pins to digital I/O, sets PORTB to be all outputs except for RB8, which we use for an input from the button. A pullup resistor is enabled for this pin as well. We use Timer 2 with Input Capture, so we set the interrupt flag to 0 to make sure it doesn’t start high. A 1:256 prescaler is used, and along with a PR2 value of 62499, a 1s timer is created. TMR2 is set to 0, and the timer is turned on.
IC1CON is set to 0, timer 2 is set for the capture source, and ICM is chosen such that ever rising edge is captured. Since the button pulls the pin low, the button is released, the pin returns high, creating a rising edge when the button is released. The timer 2 interrupt is enabled, IC1 interrupt is enabled, and the IC1 interrupt flag is set to 0. PPS is unlocked and is used to RP9 to be the pin used for input capture, and then locked again.

void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void)  
This interrupt works with timer 2, so it triggers every second. Inside the interrupt, the interrupt flag is reset so it can trigger again, and our overflow is incremented so we can keep track of the number of overflows.

void __attribute__((__interrupt__,__auto_psv__)) _IC1Interrupt(void)  
This interrupt is triggered by our input capture, which occurs when the button is pressed and then released. The interrupt flag is reset, and the current edge is calculated by getting the value from IC1BUF (which is the time at which the interrupt occurred), and then adding the length of all of the overflows. The calculated period is determined by subtracting the previous edge from the current edge. For debouncing, we check that the calculated period is sufficiently long, and if it’s not, we throw it away. If the period is sufficiently long, we put the current edge value into our circular buffer, and set the previous edge to be the current edge. We check if the time between button clicks is sufficiently short by finding the difference between the edge values in the circular buffer, and if it is, we reset the buffer, change the position of the servo, reset the overflow, and reset the timer so that our program is ready to detect more double clicks.

main(void)  
In our main, we call initServo(), initPushButton(), and setServo(). We call setServo just so we know what the initial position is. In our infinite while, we reset the position of the servo if no button presses have been detected for 2 seconds by waiting for the overflow to be greater than 1. setServo() is called, and overflow is reset if the overflow is greater than 1.
