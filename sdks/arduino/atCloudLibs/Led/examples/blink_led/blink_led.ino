/*
 * 
 * This examples shows how to blink a LED
 * using the Led class from the Led library
 * 
 */

#include <atCloudIO_Lib.h>

// Use the built-in LED
#define LED_PIN 13

// Create a Led object
// This will set the pin to OUTPUT
//Led led(LED_PIN);

void setup() { }

void loop() {
    // Power on the LED
  //  led.on();
    delay(1000);
    // Power off the LED
    //led.off();
    delay(1000);
}