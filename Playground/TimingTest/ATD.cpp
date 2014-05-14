//
//  ATD.cpp (Source file for ATD.h)
//
//  Created by Terry Kong on 2/8/14.
//
//  This is the source file that creates the ATD object to use.

#include "ATD.h"
#include "math.h"

ATD::ATD(unsigned char ADres,\
         int nCells,\
         int nGaurdCells,\
         int cutCellIndex,\
         float Pfa) {
    _ADres = ADres;
    
    if (nCells > MAX_WINDOW_SIZE) {
        nCells = MAX_WINDOW_SIZE;
    }
    
    _nCells = nCells;
    //_window = new int[nCells];
    _window = new int[MAX_WINDOW_SIZE];
    // Need to initialize for Gallileo
    for (int i = 0; i < _nCells; i++) {
        _window[i] = 0;
    }
    _nGaurdCells = nGaurdCells;
    _cutCellIndex = cutCellIndex;
    _Pfa = Pfa;
    _threshFac = -(float)log((double)Pfa);
    _ready = cutCellIndex+1;
    _found = false;
    _threshold = 0;
    _frontSum = 0;
    _backSum = 0;
    _nEffCells = (nCells - 1 - (2*nGaurdCells) - 1); // -1 CUT, -1 max value
    _frontEndIndex = cutCellIndex - nGaurdCells - 1;
    _backStartIndex = cutCellIndex + nGaurdCells + 1;
    _currentMax = 0;
    _frontMax = 0;
    _backMax = 0;
}

bool ATD::addToWindow(int input) {
    // Check if new max has popped out back
    if (_backMax < _window[_backStartIndex - 1]) {
        _backMax = _window[_backStartIndex - 1];
    } else if (_backMax == _window[_nCells-1]) {
        _backMax = max(_window,_backStartIndex-1,_nCells-2);
    }
    // Update backsum
    _backSum -= _window[_nCells-1];
    _backSum += _window[_backStartIndex-1];
    // Shift the window
    for (int i = _nCells - 1; i > 0; i--) {
        _window[i] = _window[i-1];
    }
    // Insert new data
    _window[0] = input;
    // Update frontsum
    _frontSum += _window[0];
    _frontSum -= _window[_frontEndIndex+1];
    // Check if a new max has entered the front
    if (_frontMax < input) {
        _frontMax = input;
    } else if (_frontMax == _window[_frontEndIndex+1]) {
        _frontMax = max(_window,0,_frontEndIndex);
    }
    // return if value in CUT is a target
    _threshold = thresholdRayleigh(_frontSum+_backSum,_nEffCells,_frontMax,_backMax,_threshFac);
    // count down until ready
    if(_ready > 0) {
        _ready--;
    }
    _found = (float)_window[_cutCellIndex] > _threshold;
    return _found;
}

struct MaxStruct {
    int *value;
    int *valueIndex;
    int last;
};

void ATD::addToWindowN(int* input, bool* outputFound, float* outputThresh, int length) {
    int *window = new int[_nCells+length];
    // Copy the current window in
    for (int i = 0; i < _nCells; i++) {
        window[_nCells-i-1] = _window[i];
    }
    // Copy the new window in
    for (int i = 0; i < length; i++) {
        window[_nCells+i] = input[i];
    }
    // Set new indices
    int frontEndIndexNew = _nCells-1;
    int frontStartIndexNew = _nCells-1-_frontEndIndex;
    int backEndIndexNew = _nCells-1-_backStartIndex;
    int backStartIndexNew = 0;
    int cutCellNew = _nCells-1-_cutCellIndex;
    // Declaring Descending Maxima structs
    MaxStruct frontStruct = {
        //(add 1 extra b/c we add before we delete, so it's possible to seg fault)
        new int[frontEndIndexNew - frontStartIndexNew + 1 + 1], // value
        new int[frontEndIndexNew - frontStartIndexNew + 1 + 1], // valueIndex
        0 // last
    };
    MaxStruct backStruct = {
        new int[backEndIndexNew - backStartIndexNew + 1 + 1], // value
        new int[backEndIndexNew - backStartIndexNew + 1 + 1], // valueIndex
        0 // last
    };
    // Initialize Maxima structs
    frontStruct.value[0] = window[frontStartIndexNew];
    frontStruct.valueIndex[0] = frontStartIndexNew;
    frontStruct.last = 0;
    backStruct.value[0] = window[backStartIndexNew];
    backStruct.valueIndex[0] = backStartIndexNew;
    backStruct.last = 0;
    for (int i = frontStartIndexNew+1; i <= frontEndIndexNew; i++) {
        if (frontStruct.value[frontStruct.last] > window[i]) {
            frontStruct.value[frontStruct.last+1] = window[i];
            frontStruct.valueIndex[frontStruct.last+1] = i;
            ++frontStruct.last;
        } else {
            while (frontStruct.last >= 0 && frontStruct.value[frontStruct.last] <= window[i]) {
                --frontStruct.last;
            }
            ++frontStruct.last;
            frontStruct.value[frontStruct.last] = window[i];
            frontStruct.valueIndex[frontStruct.last] = i;
        }
    }
    for (int i = backStartIndexNew+1; i <= backEndIndexNew; i++) {
        if (backStruct.value[backStruct.last] > window[i]) {
            backStruct.value[backStruct.last+1] = window[i];
            backStruct.valueIndex[backStruct.last+1] = i;
            ++backStruct.last;
        } else {
            while (backStruct.last >= 0 && backStruct.value[backStruct.last] <= window[i]) {
                --backStruct.last;
            }
            ++backStruct.last;
            backStruct.value[backStruct.last] = window[i];
            backStruct.valueIndex[backStruct.last] = i;
        }
    }
    // Loop through all the data
    for (int i = _nCells; i < _nCells + length; i++) {
        ++frontEndIndexNew;
        ++frontStartIndexNew;
        ++backEndIndexNew;
        ++backStartIndexNew;
        ++cutCellNew;
        _frontSum -= window[frontStartIndexNew-1];
        _frontSum += window[frontEndIndexNew];
        _backSum -= window[backStartIndexNew-1];
        _backSum += window[backEndIndexNew];
        // Update front Maximum Struct
        if (frontStruct.value[frontStruct.last] > window[frontEndIndexNew]) {
            frontStruct.value[frontStruct.last+1] = window[frontEndIndexNew];
            frontStruct.valueIndex[frontStruct.last+1] = frontEndIndexNew;
            ++frontStruct.last;
        } else {
            while (frontStruct.last >= 0 && frontStruct.value[frontStruct.last] <= window[frontEndIndexNew]) {
                --frontStruct.last;
            }
            ++frontStruct.last;
            frontStruct.value[frontStruct.last] = window[frontEndIndexNew];
            frontStruct.valueIndex[frontStruct.last] = frontEndIndexNew;
        }
        if (frontStruct.valueIndex[0] <= (frontStartIndexNew - 1)) {
            // Remove the first element if it exits the window
            for (int j = 1; j <= frontStruct.last; j++) {
                frontStruct.value[j-1] = frontStruct.value[j];
                frontStruct.valueIndex[j-1] = frontStruct.valueIndex[j];
            }
            --frontStruct.last;
        }
        // Update back Maximum Struct
        if (backStruct.value[backStruct.last] > window[backEndIndexNew]) {
            backStruct.value[backStruct.last+1] = window[backEndIndexNew];
            backStruct.valueIndex[backStruct.last+1] = backEndIndexNew;
            ++backStruct.last;
        } else {
            while (backStruct.last >= 0 && backStruct.value[backStruct.last] <= window[backEndIndexNew]) {
                --backStruct.last;
            }
            ++backStruct.last;
            backStruct.value[backStruct.last] = window[backEndIndexNew];
            backStruct.valueIndex[backStruct.last] = backEndIndexNew;
        }
        if (backStruct.valueIndex[0] <= (backStartIndexNew - 1)) {
            // Remove the first element if it exits the window
            for (int j = 1; j <= backStruct.last; j++) {
                backStruct.value[j-1] = backStruct.value[j];
                backStruct.valueIndex[j-1] = backStruct.valueIndex[j];
            }
            --backStruct.last;
        }
        // Calculate the threshold
        outputThresh[i-_nCells] = thresholdRayleigh(_frontSum+_backSum,_nEffCells,frontStruct.value[0],backStruct.value[0],_threshFac);
        outputFound[i-_nCells] = window[cutCellNew] > outputThresh[i-_nCells];
    }
    // Now move the latest data into the instance variables
    for (int k = 0; k < _nCells; k++) {
        _window[k] = window[_nCells + length - 1 - k];
    }
    _threshold = outputThresh[length-1];
    _found = outputFound[length-1];
    if (_ready - length < 0) {
        _ready = 0;
    } else {
        _ready -= length;
    }
    delete[] window;
    delete[] frontStruct.value;
    delete[] frontStruct.valueIndex;
    delete[] backStruct.value;
    delete[] backStruct.valueIndex;
}

void ATD::reset() {
    for (int i = 0; i < _nCells; i++) {
        _window[i] = 0;
    }
    _ready = _cutCellIndex + 1;
    _frontSum = 0;
    _backSum = 0;
    _frontMax = 0;
    _backMax = 0;
}

void ATD::reset(unsigned char ADres,\
                int nCells,\
                int nGaurdCells,\
                int cutCellIndex,\
                float Pfa) {
    _ADres = ADres;
    if (nCells > MAX_WINDOW_SIZE) {
        nCells = MAX_WINDOW_SIZE;
    }
    _nCells = nCells;
    // Need to initialize for Gallileo
    for (int i = 0; i < _nCells; i++) {
        _window[i] = 0;
    }
    _nGaurdCells = nGaurdCells;
    _cutCellIndex = cutCellIndex;
    _Pfa = Pfa;
    _threshFac = -(float)log((double)Pfa);
    _ready = cutCellIndex+1;
    _found = false;
    _threshold = 0;
    _frontSum = 0;
    _backSum = 0;
    _nEffCells = (nCells - 1 - (2*nGaurdCells) - 1); // -1 CUT, -1 max value
    _frontEndIndex = cutCellIndex - nGaurdCells - 1;
    _backStartIndex = cutCellIndex + nGaurdCells + 1;
    _currentMax = 0;
    _frontMax = 0;
    _backMax = 0;
}

float ATD::getThreshold() {
    return _threshold;
}

int ATD::getCut() {
    return _window[_cutCellIndex];
}

bool ATD::isReady() {
    return !(_ready);
}

bool ATD::isFound() {
    return _found;
}

float ATD::thresholdRayleigh(int runningSum, int nEffCells, int frontMax, int backMax, float threshFac) {
    float threshold = runningSum;
    if (frontMax > backMax) {
        threshold -= frontMax;
    } else {
        threshold -= backMax;
    }
    threshold /= nEffCells;
    threshold *= threshFac;
    return threshold;
}

int ATD::max(int* window,int start,int end) {
    int currentMax = window[start];
    for (int i = start + 1; i <= end; i++) {
        if (currentMax < window[i]) {
            currentMax = window[i];
        }
    }
    return currentMax;
}
