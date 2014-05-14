//
//  ATDArduino.cpp
//  
//
//  Created by Terry Kong on 2/12/14.
//
//

#include <iostream>
#include "ATDArduino.h"
#include "ATD.h"
//#include "Arduino.h"

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

void ATDArduino::printWindow() {
    int howManyPerLine = 10;
    char digits = '5'; //includes space
    // Create format string
    char* format = new char[4*howManyPerLine];
    for (int i = 0; i < howManyPerLine; i++) {
        format[4*i] = '%';
        format[4*i+1] = digits;
        format[4*i+2] = 'u';
        format[4*i+3] = ' ';
    }
    // Print lines of 10 numbers
    for (int i = 0; i < (int) _nCells/howManyPerLine; i++) {
        printf(format,_window[howManyPerLine*i],_window[howManyPerLine*i+1],_window[howManyPerLine*i+2],_window[howManyPerLine*i+3],_window[howManyPerLine*i+4],_window[howManyPerLine*i+5],_window[howManyPerLine*i+6],_window[howManyPerLine*i+7],_window[howManyPerLine*i+8],_window[howManyPerLine*i+9]);
        printf("\n");
    }
    delete[] format;
    // Print the remaining numbers
    char* format2 = new char[4];
    format2[0] = '%';
    format2[1] = digits;
    format2[2] = 'u';
    format2[3] = ' ';
    for (int i = 0; i < _nCells%howManyPerLine; i++) {
        printf(format2,_window[((int)_nCells/howManyPerLine)+i]);
    }
    //printf("\n");
    delete[] format2;
}
