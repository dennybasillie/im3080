//Mode : 0  = RESET ; 1 = PIANO ; 2 = BEAT ; 3 = X'MAS ; 6 = DEFAULT

int prevMode = -1;
int currMode = 2;

int currSeq = -1;
boolean isInitial [8];

//************************ LEDS *****************************************//
#include "FastLED.h"
#define NUM_STRIPS 2
#define NUM_LEDS_PER_STRIP 120
CRGB leds[NUM_STRIPS][NUM_LEDS_PER_STRIP];

int val = 0;
int cutoffIndex = 0;
int prevCutoffIndex = 0;
int threshold = 10;
int brightness = 100;
int colorWeight = -1;


//************************ MOTOR **************************************//
#include <ShiftPWM.h>

//dataPin is by default 11 and clockPin by default 13
const int ShiftPWM_latchPin = 8; //Latch pin for the Motor Shift Register

const bool ShiftPWM_invertOutputs = false;

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

// Here you set the number of brightness levels, the update frequency and the number of shift registers.
// These values affect the load of ShiftPWM.

unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 75 ;
int numRegisters = 2;


//**************************** SWITCH **********************************//
//pins for Shift In Register
int latchPin = 10;
int dataPin = 9;
int clockPin = 7;

//Define variables to hold the data for shift register. Starting with a non-zero numbers can help troubleshoot
byte switchVar1 = 72;  //01001000

//define an array that corresponds to values for each of the shift register's pins
int noBtn[] = {0, 1, 2, 3, 4, 5, 6, 7};


//**************************** VOICE RECOGNITION **********************************//
#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"

/**
  Connection
  Arduino    VoiceRecognitionModule
   2   ------->     TX
   3   ------->     RX
*/
VR myVR(2, 3);   // 2:RX 3:TX, you can choose your favourite pins.

uint8_t records[7]; // save record
uint8_t buf[64];

#define resetRecord  (0)
#define pianoRecord  (1)
#define beatRecord   (2)
#define xmasRecord   (3)
#define testRecord (6)


/**
  @brief   Print signature, if the character is invisible,
           print hexible value instead.
  @param   buf     --> command length
           len     --> number of parameters
*/
void printSignature(uint8_t *buf, int len)
{
  int i;
  for (i = 0; i < len; i++) {
    if (buf[i] > 0x19 && buf[i] < 0x7F) {
      Serial.write(buf[i]);
    }
    else {
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}

/**
  @brief   Print signature, if the character is invisible,
           print hexible value instead.
  @param   buf  -->  VR module return value when voice is recognized.
             buf[0]  -->  Group mode(FF: None Group, 0x8n: User, 0x0n:System
             buf[1]  -->  number of record which is recognized.
             buf[2]  -->  Recognizer index(position) value of the recognized record.
             buf[3]  -->  Signature length
             buf[4]~buf[n] --> Signature
*/
void printVR(uint8_t *buf)
{
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");

  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if (buf[0] == 0xFF) {
    Serial.print("NONE");
  }
  else if (buf[0] & 0x80) {
    Serial.print("UG ");
    Serial.print(buf[0] & (~0x80), DEC);
  }
  else {
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");

  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if (buf[3] > 0) {
    printSignature(buf + 4, buf[3]);
  }
  else {
    Serial.print("NONE");
  }
  Serial.println("\r\n");
}


/****************************** MAIN FUNCTIONALITY ******************************************/

void setup() {
  Serial.begin(115200);
  myVR.begin(9600);

  FastLED.addLeds<NEOPIXEL, 5>(leds[0], NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, 6>(leds[1], NUM_LEDS_PER_STRIP);

  //define pin modes
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);

  // Sets the number of 8-bit shift out registers that are used (for motors)
  ShiftPWM.SetAmountOfRegisters(numRegisters);
  ShiftPWM.Start(pwmFrequency, maxBrightness);
  ShiftPWM.SetAll(0); //Set all motors to idle initially

  for (int index = 0; index < 8; index++) {
    isInitial[index] = true;
  }
}

void loop() {
  int ret;
  ret = myVR.recognize(buf, 50);
  if (ret > 0) {
    if (buf[1] != currMode) {
      switch (buf[1]) {
        case resetRecord:
          prevMode = currMode;
          currMode = 0;
          break;
        case pianoRecord:
          prevMode = currMode;
          currMode = 1;
          break;
        case beatRecord:
          prevMode = currMode;
          currMode = 2;
          break;
        case xmasRecord:
          prevMode = currMode;
          currMode = 3;
          break;
        case testRecord:
          prevMode = currMode;
          currMode = 6;
          break;
        default:
          Serial.println("Record function undefined");
          break;
      }
      /** voice recognized */
      //printVR(buf);
    }
  }

  if (currMode == 0) {
    reset();
  } else if (currMode == 1) {
    if (prevMode != currMode) {
      prevMode = currMode;
      Serial.write(2);
    }
    pianoMode();
  } else if (currMode == 2) {
    if (prevMode != currMode) {
      prevMode = currMode;
      Serial.write(1);
    }
    beatMode();
  } else if (currMode == 3) {
    xmasMode();
  } else if (currMode == 6) {
    testMode(1);
  }

}

/******************************* END OF MAIN LOOP ***************************/

////// ----------------------------------------shiftIn function
///// just needs the location of the data pin and the clock pin
///// it returns a byte with each bit in the byte corresponding
///// to a pin on the shift register. leftBit 7 = Pin 7 / Bit 0= Pin 0

byte shiftIn(int myDataPin, int myClockPin) {
  int i;
  int temp = 0;
  int pinState;
  byte myDataIn = 0;

  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, INPUT);

  //we will be holding the clock pin high 8 times (0,..,7) at the
  //end of each time through the for loop

  //at the begining of each loop when we set the clock low, it will
  //be doing the necessary low to high drop to cause the shift
  //register's DataPin to change state based on the value
  //of the next bit in its serial information flow.
  //The register transmits the information about the pins from pin 7 to pin 0
  //so that is why our function counts down
  for (i = 7; i >= 0; i--)
  {
    digitalWrite(myClockPin, 0);
    delayMicroseconds(0.2);
    temp = digitalRead(myDataPin);
    if (temp) {
      pinState = 1;
      //set the bit to 0 no matter what
      myDataIn = myDataIn | (1 << i);
    }
    else {
      //turn it off -- only necessary for debuging
      //print statement since myDataIn starts as 0
      pinState = 0;
    }

    //Debuging print statements
    //Serial.print(pinState);
    //Serial.print("     ");
    //Serial.println (dataIn, BIN);

    digitalWrite(myClockPin, 1);

  }
  //debuging print statements whitespace
  //Serial.println();
  //Serial.println(myDataIn, BIN);
  return myDataIn;
}

void testMode(int n) {

  digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(6, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

}


//**************************************************************//
//MODE (reset, piano, beat, christmas)

void reset() {
    getSwitchValue();
    int sum = 0;
    for (int n = 0; n <= 7; n++)
    {
      isInitial[n] = switchVar1 & (1 << n); //extract bit at position n
      if (isInitial[n]) {
        ShiftPWM.SetOne(n * 2, 0);
        Serial.println(sum);
        sum++;
      }
    }
    if (sum == 8) {
      currSeq = -1;
    }
}

void pianoMode() {

  digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(6, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

}

void beatMode() {

  /*************** LED ********************//*
    if (Serial.available()) {
    val = Serial.read();
    prevCutoffIndex = cutoffIndex;
    cutoffIndex = (int)(NUM_LEDS_PER_STRIP - NUM_LEDS_PER_STRIP * threshold / (val + 0.0)); //obtained from the formula (total_leds-i)/total_leds * val < threshold
    if (cutoffIndex > (NUM_LEDS_PER_STRIP - 1)) cutoffIndex = (NUM_LEDS_PER_STRIP - 1); //prevent index out of range
    if (cutoffIndex < 0) cutoffIndex = 0;
    }

    if (cutoffIndex > prevCutoffIndex) { //grow right
    for (int i = prevCutoffIndex + 1; i <= cutoffIndex; i++) {
      leds[0][i] = CRGB(brightness - i / (NUM_LEDS_PER_STRIP - 1.0) * brightness, i / (NUM_LEDS_PER_STRIP - 1.0) * brightness, 0);
    }

    } else if (cutoffIndex < prevCutoffIndex) { //shrink left
    for (int i = prevCutoffIndex; i > cutoffIndex; i--) {
      leds[0][i] = CRGB::Black;
    }
    }
    FastLED.show();*/

  /*************** DISK ********************/
  if (currSeq == -1) {
    sequence1();
    currSeq = 1;
  } else {
    getSwitchValue();
    int sum = 0;
    for (int n = 0; n <= 7; n++)
    {
      isInitial[n] = switchVar1 & (1 << n); //extract bit at position n
      if (isInitial[n]) {
        ShiftPWM.SetOne(n * 2, 0);
        Serial.println(sum);
        sum++;
      }
    }
    if (sum == 8) {
      if (currSeq == 1) {
        sequence2();
        currSeq = 2;
        //delay(1000);
      } else if (currSeq == 2) {
        sequence1();
        currSeq = 1;
        //delay(1000);
      }
    }
  }
}

void xmasMode() {

  digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(6, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);

}

//************************** DISK CONTROL ************************************//
void getSwitchValue () {
  //e.g. 11111110 indicates all switch activated except first switch
  digitalWrite(latchPin, 1); //latchPin = 1 to collect parallel data
  delayMicroseconds(20); //wait for data
  digitalWrite(latchPin, 0); //latchPin = 0 to transmit data serially

  switchVar1 = shiftIn(dataPin, clockPin);  //while the shift register is in serial mode collect each shift register into a byte
}

void sequence1()
{
  Serial.println("In Sequence 1");
  for (int index = 1; index < 16; index += 2) {
    ShiftPWM.SetOne(index, 0);
  }

  for (int index = 0; index < 16; index += 2) {
    ShiftPWM.SetOne(index, 255);
  }
}

void sequence2()
{
  Serial.println("In Sequence 2");
  for (int index = 1; index < 16; index += 2) {
    if ((index % 4) == 1) {
      ShiftPWM.SetOne(index, 0);
    } else {
      ShiftPWM.SetOne(index, 255);
    }
  }

  for (int index = 0; index < 16; index += 2) {
    ShiftPWM.SetOne(index, 255);
  }
}

void sequence3()
{
  //same speed, with delay


  /*int index = 0;
    for (; index < 16; index += 2) {
    ShiftPWM.SetOne(index, 255);
    delay(300);
    }*/
  int i = 0;
  while (i < 3) {
    digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);              // wait for a second
    digitalWrite(6, LOW);    // turn the LED off by making the voltage LOW
    delay(500);              // wait for a second
    i++;
  }
  delay(1000);

}

void sequence4()
{
  //mid fastest

  /* int index = 0;
    for (; index < 8; index += 2) {
     ShiftPWM.SetOne(index,255*((index/2)+1)/4 );
     ShiftPWM.SetOne(8-index,255*((index/2)+1)/4 );
    }*/
  int i = 0;
  while (i < 4) {
    digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(500);              // wait for a second
    digitalWrite(6, LOW);    // turn the LED off by making the voltage LOW
    delay(500);              // wait for a second
    i++;
  }
  delay(1000);

}

void moveOne(int i)
{

  int index = i;
  int speed;
  int delayTime = 20; // milliseconds between each speed step

  // accelerate the motor

  for (speed = 55; speed <= 255; speed++)
  {
    ShiftPWM.SetOne(index, speed);  // set the new speed
    delay(delayTime);             // delay between speed steps
  }

  // decelerate the motor

  for (speed = 255; speed >= 55; speed--)
  {
    ShiftPWM.SetOne(index, speed);  // set the new speed
    delay(delayTime);             // delay between speed steps
  }
}
