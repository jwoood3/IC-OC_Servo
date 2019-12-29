# IC-OC_Servo

This code is to be used with the PIC24FJ64GA004 microcontroller and [Sparkfun Micro Servo](https://www.sparkfun.com/products/9065). 
This code uses IC (input capture) and OC (output compare) to detect button presses and use those to control a servo motor. Specifically, this will detect if a rapid double press of a button occurs, and turn a servo motor by a small angle should the double press be detected. A video example can be seen [here](https://imgur.com/a/gsCLYIR).
