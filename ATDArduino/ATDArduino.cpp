//
//  ATDArduino.cpp
//  
//
//  Created by Terry Kong on 2/12/14.
//
//

#include "ATDArduino.h"
#include "ATD.h"
#include "Arduino.h"

/* Constructor for ATD. Calls super constructor. */
ATDArduino::ATDArduino(unsigned char ADres,\
             int nCells,\
             int nGaurdCells,\
             int cutCellIndex,\
             float Pfa):
ATD(ADres,\
    nCells,\
    nGaurdCells,\
    cutCellIndex,\
    Pfa){}

/* This prints the ATD window assuming the Serial communication has been established. */
void ATDArduino::printWindow() {
    int howManyPerLine = 10;
    
    // Print lines of 10 numbers
    for (int i = 0; i < (int) _nCells/howManyPerLine; i++) {
        Serial.print(_window[howManyPerLine*i]); Serial.print(" ");
        Serial.print(_window[howManyPerLine*i+1]); Serial.print(" ");
        Serial.print(_window[howManyPerLine*i+2]); Serial.print(" ");
        Serial.print(_window[howManyPerLine*i+3]); Serial.print(" ");
        Serial.print(_window[howManyPerLine*i+4]); Serial.print(" ");
        Serial.print(_window[howManyPerLine*i+5]); Serial.print(" ");
        Serial.print(_window[howManyPerLine*i+6]); Serial.print(" ");
        Serial.print(_window[howManyPerLine*i+7]); Serial.print(" ");
        Serial.print(_window[howManyPerLine*i+8]); Serial.print(" ");
        Serial.print(_window[howManyPerLine*i+9]); Serial.println("");
    }
    // Print the last line
    for (int i = 0; i < _nCells%howManyPerLine; i++) {
        Serial.print(_window[((int)_nCells/howManyPerLine)+i]); Serial.print(" ");
    }
    
    Serial.println("");
}
