//
//  
//
//  Created by Terry Kong on 2/12/14.
//
//

#include <iostream>
#include "ATDArduino.h"
#include "ATD.h"
#include <math.h>

#include <sys/time.h>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::high_resolution_clock::time_point Time;
using namespace std::chrono;

int main() {
    int trials = 10000;
    
    unsigned char ADres = 8;
    int nCells[] = {5,10,20,50,100,200,500};
    int nCellsSize = sizeof nCells/sizeof(int);
    int cutCellIndex[nCellsSize];
    for (int n = 0; n < nCellsSize; n++) {
        cutCellIndex[n] = floor(nCells[n]/2);
    }
    int nGaurdCells = 1;
    float Pfa = 10e-3;
    
    /* Set Up Data */
    int nData[] = {1,10,50,100,500,1000,10000,100000};
    int nDataSize = sizeof nData/sizeof(int);
    
    int *data = new int[nData[nDataSize-1]];
    float *thresh = new float[nData[nDataSize-1]];
    bool *found = new bool[nData[nDataSize-1]];
    
    for (int i = 0; i < nData[nDataSize-1]; i++) {
        data[i] = i;
    }
    
    // Testing
    Time begin;
    Time end;
    duration<double> time_span;
    
    ATDArduino atd(ADres,nCells[0],nGaurdCells,cutCellIndex[0],Pfa);
    for (int i = 0; i < nCellsSize; i++) {
        atd.reset(ADres,nCells[i],nGaurdCells,cutCellIndex[i],Pfa);
        //std::cout << nCells[i] << std::endl;
        for (int j = 0; j < nDataSize; j++) {
            std::cout << nData[j] << "  ";
            for (int k = 0; k < trials; k++) {
                begin = Clock::now();
                atd.addToWindowN(data, found, thresh, nData[j]);
                end = Clock::now();
                time_span = duration_cast<duration<double> >(end - begin);
                std::cout << time_span.count() << " ";
            }
            std::cout << std::endl;
        }
        //std::cout << std::endl;
    }
    
    //delete atd;
    //delete[]cutCellIndex;
    delete[]data;
    delete[]thresh;
    delete[]found;
}
