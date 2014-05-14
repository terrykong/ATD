//
//  ATDArduino.h
//  
//
//  Created by Terry Kong on 2/12/14.
//
//

#ifndef ____ATDArduino__
#define ____ATDArduino__

#include "ATD.h"

class ATDArduino: public ATD {
public:
    // Invoke super()
    ATDArduino(unsigned char ADres,\
               int nCells,\
               int nGaurdCells,\
               int cutCellIndex,\
               float Pfa);
    // Implement the supermethod printWindow()
    
    void printWindow();
};

#endif /* defined(____ATDArduino__) */
