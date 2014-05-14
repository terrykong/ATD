//
//  ATD.h
//  
//
//  Created by Terry Kong on 2/7/14.
//
//

#ifndef _ATD_h
#define _ATD_h

class ATD {
public:
    // Public constructor to create an ATD object
    ATD(unsigned char ADres,\
        int nCells,\
        int nGaurdCells,\
        int cutCellIndex,\
        float Pfa);
    /***************************/
    /* Public Instance Methods */
    /***************************/
    // Returns a boolean corresponding to whether new data point is found
    bool addToWindow(int input);
    // Returns an array of booleans corresponding to whether a new data point is found
    void addToWindowN(int* input, bool* outputFound, float* outputThresh, int inputLength);
    // Resets the ATD (just window);
    void reset();
    // Resets the ATD w/ parameters
    void reset(unsigned char ADres,\
               int nCells,\
               int nGaurdCells,\
               int cutCellIndex,\
               float Pfa);
    // Prints string with window contents into std::cout; 10 per line
    virtual void printWindow()=0;
    // Returns threshold for current CUT cell
    float getThreshold();
    // Returns CUT cell content
    int getCut();
    // Returns true if the ATD is ready to use. This happens when the first value
    //      inserted into the window reaches the CUT cell
    bool isReady();
    // Returns true if the CUT cell contains a target
    bool isFound();
protected:
    /******************************/
    /* Private Instance Variables */
    /******************************/
    // ADres holds what the resolution of the A/D converter is, i.e., 8-bit
    unsigned char _ADres;
    // nCells is the number of cells in the window
    int _nCells;
    // window[] is the window and should have 'nCells' cells
    int* _window;
    // nGaurdCells is the number of gaurd cells on either side of the CUT cell
    int _nGaurdCells;
    // cutCellIndex is the index of the cut cell
    int _cutCellIndex;
    // Pfa is the (user-defined) probability of false alarm
    float _Pfa;
    // Threshold Factor
    float _threshFac;
    // ready is 0 if the ATD is loaded up
    int _ready;
    // found is true if the data in the CUT cell is bigger
    bool _found;
    // threshold is the threshold value for the current CUT cell
    float _threshold;
    // Running sum from the front of window to gaurd cell
    int _frontSum;
    // Running sum from the front of gaurd cell to end
    int _backSum;
    // Number of effective cells
    int _nEffCells;
    // Index of cutCellIndex - nGaurdCells - 1;
    int _frontEndIndex;
    // Index of cutCellIndex + nGaurdCells + 1;
    int _backStartIndex;
    // Current max between front and back end of window
    int _currentMax;
    // Max in front
    int _frontMax;
    // Max in front
    int _backMax;
    /**************************/
    /* Private Static Methods */
    /**************************/
    // Returns threshold for Rayleigh Distributed Noise
    float thresholdRayleigh(int runningSum, int nEffCells, int frontMax, int backMax, float threshFac);
    // Returns max element in an array from start to end
    int max(int* window,int start,int end);
private:
    // Maximum size for the window
    static const int MAX_WINDOW_SIZE = 300;
};

#endif
