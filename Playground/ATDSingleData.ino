#include <ATD.h>
#include <ATDArduino.h>
#include "math.h"

unsigned char ADres = 0x8;
int nCells = 40;
int nGaurdCells = 1;
int cutCellIndex = floor((double) nCells / 2.0);
float Pfa = 3e-2;
ATDArduino atd(ADres,nCells,nGaurdCells,cutCellIndex,Pfa);
unsigned long start;
//int restart = 5000; // ms

void setup() {
  Serial.begin(115200);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  start = millis();
  digitalWrite(13,LOW);
  //requestParameters();
}

union {
  float f;
  byte b[4];
} B_F;

void loop() { 
  //int restart = 5000;
  
  if (Serial.available() > 0) {
     // read the incoming byte:
     byte incomingByte = Serial.read();
     boolean found = atd.addToWindow(incomingByte);
     if(found && atd.isReady()) {
       digitalWrite(13,HIGH);
       delay(50);
       digitalWrite(13,LOW);
       delay(50);
     } 
     //Write the threshold
     B_F.f = atd.getThreshold();
     Serial.write(B_F.b[0]);
     Serial.write(B_F.b[1]);
     Serial.write(B_F.b[2]);
     Serial.write(B_F.b[3]);
     //Write isFound()
     Serial.write((byte)atd.isFound());
     
     //Reset the counter if we have a data point
     start = millis();
  } else {
    
      unsigned long current = millis() - start;
      
      if ((int)current > 5000) {
          //requestParameters();
          start = millis();
      }
  }
}

void requestParameters() {
    while(!Serial.available()) {}
    unsigned char ADresNew = readByte();
    int nCellsNew = readInt();
    int cutCellIndexNew = readInt();
    int nGaurdCellsNew = readInt();
    float PfaNew = readFloat();
    atd.reset(ADresNew,nCellsNew,nGaurdCellsNew,cutCellIndexNew,PfaNew);
}

/* Function returns a byte from Serial Comm */
byte readByte() {
    byte result;
    while(Serial.available() < 1) {}
    result = Serial.read();
    return result;
}

/* Function returns an int16 from Serial Comm 
 *   Lower byte first, then upper byte (this no longer complies with the Processing 
 *      script I wrote)
 */
int readShort() {
    int result;
    while(Serial.available() < 2) {}
    result = Serial.read();
    result += ((int)Serial.read())<<8;
    return result;
}

/* Function returns an int32 from Serial Comm 
 *   Little Endian
 */
int readInt() {
    int result;
    while(Serial.available() < 4) {}
    result = Serial.read();
    result += ((int)Serial.read())<<8;
    result += ((int)Serial.read())<<16;
    result += ((int)Serial.read())<<24;
    return result;
}

/* Function returns a float from Serial Comm 
 *   Highest byte first, lowest byte last
 */
union byteFloat {
    byte b[4];
    float fval;
} bf;

float readFloat() {
    while(Serial.available() < 4) {}
    bf.b[0] = Serial.read();
    bf.b[1] = Serial.read();
    bf.b[2] = Serial.read();
    bf.b[3] = Serial.read();
    return bf.fval;
}
