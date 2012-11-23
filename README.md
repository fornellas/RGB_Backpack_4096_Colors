What it is
==========

This is an alternative firmware for Sparkfun's serial interface RGB LED Matrix (http://www.sparkfun.com/products/760). It was based on Sparkfun's firmware v4 (http://www.sparkfun.com/datasheets/Components/RGB_Backpack_v4.zip) to address some limitations.

Please also check out the Arduino library to easily interface with the new firmware: https://github.com/fornellas/SFRGBLEDMatrix.

Motivation
==========

The v4 firmware had some issues:

* 255 colors only (3bit red/green, 2bit blue, minus the control char '%').
* Partial frame transfers require a reset or power off / on to synchronize the screen buffer again.
* The control character '%' can not be used as a valid color, essentially eliminating this color.
* Daisy chain operation requires pre configuration of each board individually.
* Limit of 8 boards with daisy chain configuration.

Although there was a firmware v5 released (http://www.sparkfun.com/tutorial/ArduinoRGBMatrix/RGB_Backpack_v5.zip), not everything was fixed.

Features
========

This new firmware gives new life to the boad:

* 4096 colors (4bit red, 4bit green and 4bit blue), higher contrast.
* Synchronization is done when SS goes from low to high, so there is no need to reboot anymore.
* No more control characters.
* Zero configuration daisy chain operation.
* Unlimited boards with daisy chain configuration (as long as SS output port from master can handle all input slave ports from all boards).

Compile and Install
===================

The code was tested at Ubuntu 11.10, with gcc-avr 1:4.5.3-2 and avr-libc 1:1.7.1-2. To compile, just rum "$ make".

Sparkfun has a nice tutorial on how to use your Arduino to program the AVR chip: http://www.sparkfun.com/tutorials/200. The issue I found, is that newer Arduino IDE versions do not come with ArduinoISP sketch. I own a Duemilanove, and was able to program using ancient Arduino 0018 IDE, its avrdude and fixed ArduinoISP-dev04b.zip sketch from http://code.google.com/p/mega-isp/issues/detail?id=14.

If you own an Uno, check out http://arduino.cc/en/Tutorial/ArduinoISP to see if there is a newer version of ArduinoISP sketch. Currently (Jan 2012), Uno can not be used as ISP. Worst case scenario, you will need to buy an AVR programmer, such as http://www.sparkfun.com/products/9825.

Interfacing
===========

SPI configuration
-----------------

The controlling device must be configured as follows:

* Master.
* MSB first.
* Mode 0 (CPOL=0, CPHA=0).
* 4MHz maximum.

Frame data packing
------------------

Each color has 4bits (MSB) and you need 3 bytes to store data for 2 pixels:

 |      pixel0     |      pixel1     |
 | RRRR GGGG | BBBB RRRR | GGGG BBBB |
 |   byte0   |   byte1   |   byte2   |

One frame will contain enough data to fill up all boards. A single board, will have a 96 bytes frame; two boards daisy chain, 192 bytes and so on.

The order data must be sent, is as follows. For the example configuration below:

            +--> 0x0
           /
           +---+---+---+---+ --> 31x0
 Wires ==> | 0 | 1 | 2 | 3 |
           +---+---+---+---+ --> 31x7

You must send data for each board in reverse order:

* Board 3
* Board 2
* Board 1
* Board 0

Pixels for each board, must be sent in this order:

* 0x7 to 7x7.
* 0x6 to 7x6.
* 0x5 to 7x5.
* 0x4 to 7x4.
* 0x3 to 7x3.
* 0x2 to 7x2.
* 0x1 to 7x1.
* 0x0 to 7x0.

Frame transfer protocol
-----------------------

The firmware will accept frame data via SPI. To send a single frame, the procedure below must be followed:

* Assert SS.
* Transfer frame data:
** Send one byte.
** Delay 64us.
** Repeat last 2 steps for the remaining bytes.
* Deassert SS.
* Delay 297us.

The delay after each byte is necessary to allow time for the SPI Interrupt code to run. If you do not delay, some bytes will not be properly trasfered to the boards.

The delay after SS HIGH is needed to allow some time for the receive / displays buffers to be swapped. Data can not be received at this point.

Sample interface code
---------------------

Here is a minimal code for Arduino 1.0. You should see a full white frame blinking at ~1Hz.

  #define BOARDS 1
  
  void setup() {
    // SPI Init
    SPCR=(1<<SPE)|(1<<MSTR);
    SPSR = SPSR & B11111110;
    // Pins set up
    pinMode(11, OUTPUT); // MOSI
    pinMode(10, OUTPUT); // SS
    digitalWrite(10, HIGH);
    delay(1000);
  }
  
  byte c=0;
  
  void loop() {
    // swap colors
    switch(c){
      case 0:
      c=255;
      break;
    case 255:
      c=0;
      break;
    }
    
    // Initiate transfer
    digitalWrite(10, LOW);
    // Send all bytes
    for(word b=0;b<96*BOARDS;b++){
      SPDR=c;
      while(!SPSR&(1<<SPIF));
      // Give some time for the interrupt code at the boards
      delayMicroseconds(64);
    }
    // Finish transfer, image will be displayed only at this point
    digitalWrite(10, HIGH);
    // Give some time for the receive / image buffers to be swapped
    delayMicroseconds(297);
  
    // Wait 1s
    delay(1000);
  }
