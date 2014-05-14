#include <ATD.h>
#include <ATDArduino.h>
#include "math.h"

// Handshaking bytes
#define PROCEED_BYTE 255
#define HANDSHAKE_NUM 5

// ATD
unsigned char ADres = 0x8;
int nCells = 40;
int nGaurdCells = 1;
int cutCellIndex = floor((double) nCells / 2.0);
float Pfa = 1e-2;
ATDArduino atd(ADres,nCells,nGaurdCells,cutCellIndex,Pfa);
#define ATD_ENABLE_PIN 47
bool atdEnable;

//DECODER DEFINITIONS
#define CLR(x,y) (x&=(~(1<<y)))
#define SET(x,y) (x|=(1<<y))
#define LEFT 1
#define CENTER 2
#define RIGHT 3
#define ENABLE 0
#define DISABLE 1
//SERVO DEFINITIONS
#include <Servo.h> 
#define LEFT 132
#define CENTER 97
#define RIGHT 62
#define SERVO_PIN 53
Servo myservo;  // create servo object to control a servo 
                // a maximum of eight servo objects can be created 

byte servoPosition[4] = {LEFT,CENTER,RIGHT,CENTER};  
byte servoPositionDecoderIndex[4] = {1,2,3,2};

// Constants
#define PING_OVERHEAD_US 358
#define US_TO_CM 57 //divide by this factor
#define ADC_TIME 19.32 // 17.12 on UNO
#define ADC_TIME2 38.64 // This is the ADC_TIME * 2
#define ADC_INPHASE_BIAS 450 // 450*5/1023 = 2.2V
#define ADC_QUAD_BIAS 370 // 300*5/1023 = 1.46V

//Settable
#define MAX_DIST_CM 122 //122cm = 4ft
#define triggerPin 50
#define echoPin 52
#define inphasePin A0 // Analog pin
#define quadraturePin A1 //Analog pin
//location of LEDS in CM
#define NUMOFLED 16
#define LEDROWLITTIME 75 //Miliseconds
float LEDLocations[] = {7.62,15.24,22.86,30.48,38.10,45.72,53.34,60.96,68.58,76.20,83.82,91.44,99.06,106.68,114.30,121.92};
float LEDBoundaries[] = {11.43,19.05,26.67,34.29,41.91,49.53,57.15,64.77,72.39,80.01,87.63,95.25,102.87,110.49,118.11};


//Non-Settable
#define MAX_DIST_US  MAX_DIST_CM*US_TO_CM
const int MAX_DIST_ARRAYCELLS = (int)(MAX_DIST_US/ADC_TIME);
int in[MAX_DIST_ARRAYCELLS];
int quad[MAX_DIST_ARRAYCELLS];
int data[MAX_DIST_ARRAYCELLS];
float thresholds[MAX_DIST_ARRAYCELLS];
bool outputFound[MAX_DIST_ARRAYCELLS];

uint8_t triggerBit;
uint8_t echoBit;
volatile uint8_t* triggerOutput;
volatile uint8_t* triggerMode;
volatile uint8_t* echoInput;       

//Buzzers
#define LEFT_BUZZER_PIN 44
#define CENTER_BUZZER_PIN 46
#define RIGHT_BUZZER_PIN 48 
#define BUZZER_MILLISECONDS 50 // How long a buzzer should be on
bool useBuzzer = false;

//Constant Threshold Setting
#define CONSTANT_THRESH_PIN A7
#define MAX_CONSTANT_THRESH 399
int constantThreshold;

/* Data structure to convert between byte array and float: f = [ b[3] b[2] b[1] b[0] ] = f */
/* To send to MATLAB send low byte first */
union {
  float f;
  byte b[4];
} B_F;

/* Data structure to convert between byte array and int: i = [ b[1] b[0] ] */
/* To send to MATLAB send low byte first */
union {
  byte b[2];
  int i;
} B_I;

void setup() 
{ 
  Serial.begin(115200);
  myservo.attach(SERVO_PIN);
  decoderSetup();
  // This is for the ping
  pingSetup(triggerPin,echoPin);
  ADCSetup();
  pinMode(ATD_ENABLE_PIN,INPUT_PULLUP);
  buzzerSetup();
  handshake();
  B_I.i = MAX_DIST_ARRAYCELLS;
  Serial.write(B_I.b[0]);
  Serial.write(B_I.b[1]);
  B_I.i = MAX_DIST_CM;
  Serial.write(B_I.b[0]);
  Serial.write(B_I.b[1]);
} 
 
void loop() 
{ 
  for(int i = 0; i < sizeof servoPosition; i++) {
  //for(int i = 1; i <= 1; i++) {
    myservo.write(servoPosition[i]);
    delay(300); // To get into position
    sendPing();
    //Start reading in from the analog pin
    for(int i = 0; i < MAX_DIST_ARRAYCELLS; i++) {
      in[i] = analogRead(inphasePin);
      quad[i] = analogRead(quadraturePin);
    }
    for(int j = 0; j < MAX_DIST_ARRAYCELLS; j++) {
      // Use data[] as the rayleigh array
      data[j] = my_sqrt2(abs(in[j]-ADC_INPHASE_BIAS), abs(quad[j]-ADC_QUAD_BIAS));
    }
    
    atdEnable = digitalRead(ATD_ENABLE_PIN);
    atd.addToWindowN(data, outputFound, thresholds, MAX_DIST_ARRAYCELLS);
    
    constantThreshold = analogRead(CONSTANT_THRESH_PIN);
    constantThreshold = map(constantThreshold,0,1023,0,MAX_CONSTANT_THRESH);
    if (atdEnable) {
      fillArrayConstantThreshold(servoPositionDecoderIndex[i],data,constantThreshold);
    } else {
      fillArrayAdaptiveThreshold(servoPositionDecoderIndex[i],outputFound,cutCellIndex);
    }
    //lightArray(servoPositionDecoderIndex[i]);
    //lightAllArray();
    lightAllArrayBuzzOne(servoPositionDecoderIndex[i]);
    //printArray(servoPositionDecoderIndex[i]);
    
    B_I.i = constantThreshold;
    Serial.write(B_I.b[0]);
    Serial.write(B_I.b[1]);
    sendint16Array(data,MAX_DIST_ARRAYCELLS);
    sendFloatArray(thresholds,MAX_DIST_ARRAYCELLS);
    sendBooleanArray(outputFound,MAX_DIST_ARRAYCELLS);
    
    handshake();
    delay(50); // To let the LED light up and finish up
  }
} 

byte DEC1MASK;
byte DEC2MASK;
byte DEC3MASK;
byte DECLEFTMASK;
byte DECCENTERMASK;
byte DECRIGHTMASK;
byte DEC1ENABLEBIT;
byte DEC2ENABLEBIT;
byte DEC3ENABLEBIT;
byte DECLEFTENABLEBIT;
byte DECCENTERENABLEBIT;
byte DECRIGHTENABLEBIT;
byte DECALLENABLEMASK;
volatile unsigned char* DEC1;
volatile unsigned char* DEC2;
volatile unsigned char* DEC3;
volatile unsigned char* DECLEFT;
volatile unsigned char* DECCENTER;
volatile unsigned char* DECRIGHT;
volatile unsigned char* DECENABLE;
void decoderSetup() {
  DDRA = DDRA | B11111111; // FOR DEC 1 and DEC 2
  DDRC = DDRC | B10101010; // FOR DEC 3
  DDRC = DDRC | B01010100; // FOR THE ENABLE OF THE 3 DECODERS
  DEC1 = &PORTA;
  DEC2 = &PORTA;
  DEC3 = &PORTC;
  DECLEFT = &PORTA;
  DECCENTER = &PORTA;
  DECRIGHT = &PORTC;
  DEC1MASK = B01010101;
  DEC2MASK = B10101010;
  DEC3MASK = B10101010;
  DECLEFTMASK = B01010101;
  DECCENTERMASK = B10101010;
  DECRIGHTMASK = B10101010;
  //
  DECENABLE = &PORTC;
  // For Enable
  DEC1ENABLEBIT = 6;
  DEC2ENABLEBIT = 4;
  DEC3ENABLEBIT = 2;
  DECLEFTENABLEBIT = 6;
  DECCENTERENABLEBIT = 4;
  DECRIGHTENABLEBIT = 2;
  DECALLENABLEMASK = B01010100;
  //Enable All decoders
  enableDecoder(1);
  enableDecoder(2);
  enableDecoder(3);
  //RUN TO SEE THAT ALL LEDS are lit
  for(int i = 1; i <= 3; i++) {
    enableDecoder(i);
   for(int j = 0; j < 16; j++) {
     writeToDec(i,j);
     delay(25);
   }
   disableDecoder(i);
 }
}

void writeToDec(int decoder, int number) {
  if (decoder == 1) {
    chooseDecoder(DEC1,DEC1MASK,number);
  } else if (decoder == 2) {
    chooseDecoder(DEC2,DEC2MASK,number);
  } else if (decoder == 3) {
    chooseDecoder(DEC3,DEC3MASK,number);
  }
}

void chooseDecoder(volatile unsigned char* decoder, byte decoderMask, int number) {
  int numberIndex = 3;
  for(int i = 7; i >= 0; i--) {
    // Do I need to write?
    if(bitRead(decoderMask,i)) {
      // What should I write?
      if(bitRead(number,numberIndex)) {
        SET(*decoder,i);
      } else {
        CLR(*decoder,i);
      }
      --numberIndex;
    }
  }
}

void enableDecoder(int number) {
  if (number == 1) {
    CLR(*DECENABLE,DEC1ENABLEBIT);
  } else if (number == 2) {
    CLR(*DECENABLE,DEC2ENABLEBIT);
  } else if (number == 3) {
    CLR(*DECENABLE,DEC3ENABLEBIT);
  }
}

void disableDecoder(int number) {
  if (number == 1) {
    SET(*DECENABLE,DEC1ENABLEBIT);
  } else if (number == 2) {
    SET(*DECENABLE,DEC2ENABLEBIT);
  } else if (number == 3) {
    SET(*DECENABLE,DEC3ENABLEBIT);
  }
}

void pingSetup(uint8_t trig_pin, uint8_t echo_pin) {
  triggerBit = digitalPinToBitMask(trig_pin);
  echoBit = digitalPinToBitMask(echo_pin);
  triggerOutput = portOutputRegister(digitalPinToPort(trig_pin));
  echoInput = portInputRegister(digitalPinToPort(echo_pin));
  triggerMode = (uint8_t *) portModeRegister(digitalPinToPort(trig_pin));
}

void sendPing() {
  // Send the trigger data
  *triggerMode |= triggerBit;    // Set trigger pin to output.
  *triggerOutput &= ~triggerBit; // Set the trigger pin low, should already be low, but this will make sure it is.
  delayMicroseconds(4);            // Wait for pin to go low, testing shows it needs 4uS to work every time.
  *triggerOutput |= triggerBit;  // Set trigger pin high, this tells the sensor to send out a ping.
  delayMicroseconds(10);           // Wait long enough for the sensor to realize the trigger pin is high. Sensor specs say to wait 10uS.
  *triggerOutput &= ~triggerBit; // Set trigger pin back to low.
  delayMicroseconds(PING_OVERHEAD_US); // Overhead until the rising edge of the echo returns.
}

void ADCSetup() {
  bitClear(ADCSRA,ADPS0);
  bitClear(ADCSRA,ADPS1);
  bitSet(ADCSRA,ADPS2);
}

/* This function calculates the integer sqrt */
uint8_t my_sqrt(uint16_t input) {
    uint16_t res = 0;
    uint16_t one = 1u << 14;
    while (one > input) one /= 4;
    while (one != 0) {
        if (input >= res + one) {
            res += one;
            input -= res;
            res += one;
        }
        res /= 2;
        one /= 4;
    }
    return res;
}

/* This function calculates sqrt(x^2+y^2) with some error */
unsigned int MAXINT = -1;
unsigned int my_sqrt2(uint16_t x, uint16_t y) {
  if (MAXINT/x > x && MAXINT/y > y && MAXINT - x*x > y*y) {
    return my_sqrt(x*x+y*y);
  } else {
    // deal with all digits besides the ones digit
    return (100*my_sqrt((x/10)*(x/10) + (y/10)*(y/10)) + \
              my_sqrt((x%10)*10*(x%10)*10 + (y%10)*10*(y%10)*10))/10;
  }
}

bool leftArray[NUMOFLED] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool centerArray[NUMOFLED] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool rightArray[NUMOFLED] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
/* This function will light a certain row of leds for LEDROWLITTIME milliseconds */
void lightArray(int decoder) {
  bool* dec;
  if (decoder == 1) {
    dec = leftArray;
  } else if (decoder == 2) {
    dec = centerArray;
  } else if (decoder == 3) {
    dec = rightArray;
  }
  //Check if we need to turn anything on
  int offNum = NUMOFLED;
  for (int i = 0; i < NUMOFLED; i++) {
    if (!dec[i]) {
      --offNum;
    }
  }
  if (!offNum) {
    return;
  }
  enableDecoder(decoder);
  unsigned int duration = 0;
  unsigned int start = millis();
  while (duration < LEDROWLITTIME) {
    for(int i = 0; i < NUMOFLED; i++) {
      if (dec[i]) {
        writeToDec(decoder,i);
      }
    }
    duration = millis() - start;
  }
  disableDecoder(decoder);
}

/* This function will light a certain all rows of LEDs that need lighting */
void lightAllArray() {
  for(int i = 1; i<= 3; i++ ) {
    lightArray(i);
  }
}

/* This function sets up the buzzers. It also enables them for the function lightAllArrayBuzzOne(int) */
void buzzerSetup() {
  useBuzzer = true;
  pinMode(LEFT_BUZZER_PIN,OUTPUT);
  pinMode(CENTER_BUZZER_PIN,OUTPUT);
  pinMode(RIGHT_BUZZER_PIN,OUTPUT);
}

/* This function will light a certain all rows of LEDs that need lighting but buzz only one */
void lightAllArrayBuzzOne(int decoder) {
  bool* dec;
  int buzzerPin;
  if (decoder == 1) {
    dec = leftArray;
    buzzerPin = LEFT_BUZZER_PIN;
  } else if (decoder == 2) {
    dec = centerArray;
    buzzerPin = CENTER_BUZZER_PIN;
  } else if (decoder == 3) {
    dec = rightArray;
    buzzerPin = RIGHT_BUZZER_PIN;
  }
  for(int i = 1; i <= 3; i++ ) {
    lightArray(i);
    if (i == decoder) {
      //Check if we need to sound a buzzer
      for (int j = 0; j < NUMOFLED; j++) {
        if (dec[j]) {
          if (useBuzzer) {
            digitalWrite(buzzerPin,HIGH);
            delay(BUZZER_MILLISECONDS);
            digitalWrite(buzzerPin,LOW);
          }
          break;
        }
      }
    }
  }
}

/* This function determines which elements of the led array should be lit */
void fillArrayConstantThreshold(int decoder,int* data,int constantThreshold) {
  bool* dec;
  if (decoder == 1) {
    dec = leftArray;
  } else if (decoder == 2) {
    dec = centerArray;
  } else if (decoder == 3) {
    dec = rightArray;
  }
  // Need to empty the array first
  for(int i = 0; i < NUMOFLED; i++) {
    dec[i] = false;
  }
  for(int i = 0; i < MAX_DIST_ARRAYCELLS; i++) {
    if (data[i] > constantThreshold) {
      dec[returnLedIndex(i)] = true;
    }
  }
}

/* This function determines which elements of the led array should be lit */
void fillArrayAdaptiveThreshold(int decoder, bool* found, int cutCellIndex) {
  bool* dec;
  if (decoder == 1) {
    dec = leftArray;
  } else if (decoder == 2) {
    dec = centerArray;
  } else if (decoder == 3) {
    dec = rightArray;
  }
  // Need to empty the array first
  for(int i = 0; i < NUMOFLED; i++) {
    dec[i] = false;
  }
  for(int i = cutCellIndex; i < MAX_DIST_ARRAYCELLS; i++) {
    if (found[i]) {
      dec[returnLedIndex(i-cutCellIndex)] = true;
    }
  }
}

/* This (HELPER) function returns which LED should be lit */
int returnLedIndex(int locationInArray) {
  float distInCM = locationInArray*ADC_TIME2/US_TO_CM; //ADC_TIME2 b/c time lost from reading quad and inphase = ADC_TIME+ADC_TIME
  if (distInCM < LEDBoundaries[0]) {
  	return 0;
  } else if (distInCM > LEDBoundaries[0] && distInCM < LEDBoundaries[1]) {
  	return 1;
  } else if (distInCM > LEDBoundaries[1] && distInCM < LEDBoundaries[2]) {
  	return 2;
  } else if (distInCM > LEDBoundaries[2] && distInCM < LEDBoundaries[3]) {
  	return 3;
  } else if (distInCM > LEDBoundaries[3] && distInCM < LEDBoundaries[4]) {
  	return 4;
  } else if (distInCM > LEDBoundaries[4] && distInCM < LEDBoundaries[5]) {
  	return 5;
  } else if (distInCM > LEDBoundaries[5] && distInCM < LEDBoundaries[6]) {
  	return 6;
  } else if (distInCM > LEDBoundaries[6] && distInCM < LEDBoundaries[7]) {
  	return 7;
  } else if (distInCM > LEDBoundaries[7] && distInCM < LEDBoundaries[8]) {
  	return 8;
  } else if (distInCM > LEDBoundaries[8] && distInCM < LEDBoundaries[9]) {
  	return 9;
  } else if (distInCM > LEDBoundaries[9] && distInCM < LEDBoundaries[10]) {
  	return 10;
  } else if (distInCM > LEDBoundaries[10] && distInCM < LEDBoundaries[11]) {
  	return 11;
  } else if (distInCM > LEDBoundaries[11] && distInCM < LEDBoundaries[12]) {
  	return 12;
  } else if (distInCM > LEDBoundaries[12] && distInCM < LEDBoundaries[13]) {
  	return 13;
  } else if (distInCM > LEDBoundaries[13] && distInCM < LEDBoundaries[14]) {
  	return 14;
  } else if (distInCM > LEDBoundaries[14]) {
  	return 15;
  }
}

void printArray(int decoder) {
  bool* dec;
  if (decoder == 1) {
    dec = leftArray;
  } else if (decoder == 2) {
    dec = centerArray;
  } else if (decoder == 3) {
    dec = rightArray;
  }
  for(int i = 0; i < 16; i++) {
    Serial.print(dec[i]);
  }
  Serial.println("");
}

void sendFloatArray(float* array, int number) {
  for (int i = 0; i < number; i++) {
    B_F.f = array[i];
    Serial.write(B_F.b[0]);
    Serial.write(B_F.b[1]);
    Serial.write(B_F.b[2]);
    Serial.write(B_F.b[3]);
  }
}

void sendint16Array(int* array, int number) {
  for (int i = 0; i < number; i++) {
    B_I.i = array[i];
    Serial.write(B_I.b[0]);
    Serial.write(B_I.b[1]);
  }
}

void sendBooleanArray(bool* array, int number) {
  for (int i = 0; i < number; i++) {
    Serial.write((byte)array[i]);
  }
}

/* This function will handshake */
void handshake() {
  byte inByte;
  while (true) {
    byte proceedNum = 5;
    byte stopNum = 5;
    for (int i = 0; i < HANDSHAKE_NUM; i++) {
      inByte = readByte();
      if (inByte == PROCEED_BYTE) {
        --proceedNum;
      }
    }
    if (!proceedNum) {
      break;
    }
  }
}

/* Function returns a byte from Serial Comm */
byte readByte() {
    byte result;
    while(Serial.available() < 1) {}
    result = Serial.read();
    return result;
}
