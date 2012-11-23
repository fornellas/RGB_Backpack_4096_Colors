#include <SFRGBLEDMatrix.h>
#include <SPI.h>

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
