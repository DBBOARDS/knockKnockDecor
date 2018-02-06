/*-----------------------------------------------------------------------------------------------------------
dbBoardsKnockKnockLighting.ino

  Summary:
    This program uses Sparkfun's ADXL345 Library to listen for double taps and inactivity. This allow the 
    user to simply knock twice on or near the project to turn it on or off. And even to change modes into
    the party mode also included in this example.
  
  Utilizing:
    Sparkfun's ADXL345 Library https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library
    Adafruit NeoPixel Library: https://github.com/adafruit/Adafruit_NeoPixel
   
  Programmer:
    Duncan Brandt @ DB Boards, LLC
    Created: Feb 3, 2018
  
  Development Environment Specifics:
    Arduino 1.6.11
  
  Hardware Specifications:
    DB Boards SPI ADXL345, DB3000
    Arduino Drawing Board (UNO) DB1000
    WS2812 Strip (4 Connected in Series) DB7000

  Beerware License:
    This program is free, open source, and public domain. The program is distributed as is and is not
    guaranteed. However, if you like the code and find it useful I would happily take a beer should you 
    ever catch me at the local.
*///---------------------------------------------------------------------------------------------------------
#include <SparkFun_ADXL345.h>         // https://github.com/sparkfun/SparkFun_ADXL345_Arduino_Library
ADXL345 adxl = ADXL345(10);           // USE FOR SPI COMMUNICATION, ADXL345(chipSelectPin);
int x,y,z;                            // Variable used to store accelerometer data
int trigger = -1, lastTrigger = -1;   // Values used to navigate the "knock menu"
boolean ledState = false;             // Are the LEDs on or off right now
//-----------------------------------------------------------------------------------------------------------
#include <Adafruit_NeoPixel.h>        // https://github.com/adafruit/Adafruit_NeoPixel
#define PIN 6                         // Pin 6 for communication with LED strip
// 10  Number of pixels in strip, NEO_GRB  Pixels are wired for GRB bitstream, NEO_KHZ800  800 KHz bitstream
Adafruit_NeoPixel strip = Adafruit_NeoPixel(70, PIN, NEO_GRB + NEO_KHZ800);
int color = 0, timing = 0;            // These are used to create patterns in the display without intterupts
const int maxTiming = 30;             // The larger this number is, the slower the party mode will progress
boolean partyMode = false;            // Has the user selected party mode
//-----------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------
void setup() {                        // The setup program runs only once after power-up or reset
  strip.begin();                      // Activate the LED strand
  strip.setBrightness(182);           // For our battery application we will limit the brightness to 1Amp max
  strip.show();                       // Initialize all pixels to 'off'
  adxl.powerOn();                     // Power on the ADXL345
  adxl.setRangeSetting(16);           // (2,4,8, or 16) lower values are more sensitive
  adxl.setSpiBit(0);                  // Configure the device to be in 4 wire SPI mode when set to '0' or 3 wire SPI mode when set to 1
  adxl.setActivityXYZ(1, 0, 0);       // Set to activate movement detection in the axes "adxl.setActivityXYZ(X, Y, Z);" (1 == ON, 0 == OFF)
  adxl.setActivityThreshold(75);      // Inactivity thresholds (0-255)
  adxl.setTimeInactivity(10);         // How many seconds of no activity is inactive?
  adxl.setTapDetectionOnXYZ(0, 0, 1); // Detect taps in the directions turned ON "adxl.setTapDetectionOnX(X, Y, Z);" (1 == ON, 0 == OFF)
  // Set values for what is considered a TAP and what is a DOUBLE TAP (0-255)
  adxl.setTapThreshold(50);           // 62.5 mg per increment
  adxl.setTapDuration(15);            // 625 μs per increment
  adxl.setDoubleTapLatency(80);       // 1.25 ms per increment
  adxl.setDoubleTapWindow(200);       // 1.25 ms per increment
  adxl.InactivityINT(1);              // Turn on Interrupts for each mode 1 == ON
  adxl.ActivityINT(0);                // Turn on Interrupts for each mode 0 == OFF
  adxl.FreeFallINT(0);                // Turn on Interrupts for each mode 0 == OFF
  adxl.doubleTapINT(1);               // Turn on Interrupts for each mode 0 == OFF
  adxl.singleTapINT(0);               // Turn on Interrupts for each mode 0 == OFF
}
//-----------------------------------------------------------------------------------------------------------
void loop() {                         // Loop adjusts the current lighting if need be then checks for knocks
  int action = ADXL_ISR();            // Check the interrupts for inactivity or double taps
  if(action > 0){                     // If there is a double tap
    lastTrigger = trigger;            // Save the previous state of trigger
    trigger = action;                 // Change Trigger to the new action state
  }
  if(action == -1) trigger = -1;      // If the action is inactivity, record the trigger as such 
  if(trigger == 1){                   // If we are in an active mode
    if(lastTrigger == -1){            // If inactivity is triggered
      if(ledState){                   // If the lights are on
        ledState = false;             // Say they are off
        for(int i = 0; i < 70; i++){  // Step through every LED
          strip.setPixelColor(i, strip.Color(0,0,0)); // Turn each LED off
        }
        partyMode = false;            // In case we are in party mode turn it off
        strip.show();                 // Show the off LEDs
        trigger = -1;                 // Record the trigger as inactive
      }
      else{                           // If the LEDs are offf
        for(int i = 0; i < 70; i++){  // Step through every LED
          strip.setPixelColor(i, strip.Color(0,0,182)); // Turn them on blue only
        }
        ledState = true;              // Record that the LEDs are on
        trigger = 0;                  // Balance the trigger in case the user wishes to change modes
        strip.show();                 // Show blue light
      }
    }
    else{                             // If the double tap came quickly after a recent double tap
      partyMode = true;               // Turn party mode on
      trigger = -1;                   // Force the trigger to inactive
    }
  }
  if(partyMode) showPartyMode();      // Show the next step in the partyMode sequence
}
//-----------------------------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------------------------
void showPartyMode(){                 // Slide every LED on the strip through the spectrum together
  if(color < 182){                    // If the color counter is in the first stage
    for(int i = 0; i < 70; i++){      // Step through every LED
      strip.setPixelColor(i, strip.Color(181-color,color,0)); // red falls, green rises
    }
  }
  else if(color < 364){               // If we are in the second stage
    for(int i = 0; i < 70; i++){      // Step through every LED
      strip.setPixelColor(i, strip.Color(0,181-(color-182),color-182)); // green falls, blue rises
    } 
  }
  else if(color < 545){               // If we are in stage three
    for(int i = 0; i < 70; i++){      // Step through every LED
      strip.setPixelColor(i, strip.Color(color-364,0,181-(color-364))); // blue falls, red rises
    }
  }
  if(color < 544){                    // If we have not completed the final stage
    if(timing < maxTiming) timing++;  // Add one to the timing clock
    else{                             // Unless it is full, in which case
      strip.show();                   // Show the last step in the sequence on the LEDs
      delay(1);                       // Brief Pause for communication
      color++;                        // Advance a step in the color sequence
      timing = 0;                     // Restart the timing clock
    }
  }
  else{                               // If we have completed the final stage
    color = 0;                        // Reset the sequence
  }
}
//-----------------------------------------------------------------------------------------------------------
int ADXL_ISR() {                      // Look for Interrupts and Triggered Action
  int x,y,z;                          // Used to store ADXL345 axis data
  adxl.readAccel(&x, &y, &z);         // Read the accelerometer values and store them in variables declared above x,y,z
  byte interrupts = adxl.getInterruptSource(); // Call for interrupt data
  if(adxl.triggered(interrupts, ADXL345_INACTIVITY)){ // If inactivity is triggered
    return -1;                        // Notify the loop with -1
  }
  if(adxl.triggered(interrupts, ADXL345_DOUBLE_TAP)){ // If double tap is triggered
    return 1;                         // Notify the loop with 1
  }
  return 0;                           // If there is no triggered action say 0 (NULL)
}
//-----------------------------------------------------------------------------------------------------------
