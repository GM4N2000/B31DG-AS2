/*
*   cyclicexecutive.ino
*   Assignment 2 for B31DG
*   Gregor Evans(H00299112)
*   Created: March 2023
*/

// Pin numbers
#define t1Pin        0  // Task 1 output pin 
#define t2Pin        2  // Task 2 input pin 
#define t3Pin        3  // Task 1 input pin  
#define t4AnalogPin  1  // Task 1 analogue input pin 
#define t4LEDPin    19  // Task 1 LED output pin 

// Timings
#define t2P       10  // Task 2 Period (Repeats every 10 frames)
#define t2PWait   1   // Task 2 wait from frame 0
#define t2PWait2  7   // Alternative wait for task 2 
#define t3P       2   // Task 3 Period (Repeats every 2 frames)
#define t4P       5   // Task 4 Period (Repeats every 5 frames)
#define t5P       25  // Task 5 Period (Repeats evert 25 frames)

// Unique parameters
#define baudRate       9600  // Baud Rate
#define tickLength     4     // Length of Each Frames 4ms
#define numFrames      50    // Number of frames before schedule refresh 

#define t2Timeout   3100  // Manual Timeout for worst case of pulseIn 31us
#define t2MinFreq   333   // Task 2 minimium frequency of waveform in Hz
#define t2MaxFreq   1000  // Task 2 maximum frequency of wavefrom in Hz

#define t3Timeout   2100  // Manual Timeout for worst case of pulseIn 21us
#define t3MinFreq   500   // Task 3 minimium frequency of waveform in Hz
#define t3MaxFreq   1000  // Task 3 minimium frequency of waveform in Hz

#define numParams      4  // Array length for storing task 4 measurement   
#define t4Thresh    2048  // Task 4 Threshhold for turning LED on/off

#define t5Min       0     // Task 5 Lower limit of range
#define t5Max       99    // Task 5 Upper limit of range

#define periodToFreq(T) (1 / (T / 1000000)) // Period to Frequency conversion

typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;

#include <B31DGMonitor.h>
#include <Ticker.h>

// Globals
B31DGCyclicExecutiveMonitor monitor;

uint8 ticCount; //Tick counter, is reset after 50 frame
Ticker ticker;  //Ticker object

double freqT2; //Calculated frequency for Task 2
double freqT3; //Calculated frequenct for Task 3

uint16 anIn[numParams]; //Array Holding the Analoge signal Measurements
uint8 currInd; //Current index to overwrite in anIn[]

void setup() {
  Serial.begin(baudRate);

  // Define inputs and outputs
  pinMode(t1Pin, OUTPUT); 
  pinMode(t2Pin, INPUT);
  pinMode(t3Pin, INPUT); 
  pinMode(t4AnalogPin, INPUT); 
  pinMode(t4LEDPin, OUTPUT); 

 // Begins analogue input array for task 4
  currInd = 0;
  for (uint8 i = 0; i < numParams; i++) {
    anIn[i] = 0;
  }

  // Begin Ticker and Monitoring
  ticCount = 0;
  monitor.startMonitoring();
  ticker.attach_ms(tickLength, cycle);
  cycle();
}

void cycle() {
  // Schedule - tasks run sequentially dependant on the tXp in frames.
  task1();
  // Task 2 wont start at frame 0 as offset used
  if (((ticCount - t2PWait) % t2P) == 0) task2();
  if (((ticCount - t2PWait2) % t2P) == 0) task2();
  if ((ticCount % t3P) == 0)  task3();
  if ((ticCount % t4P) == 0)  task4();
  if ((ticCount % t5P) == 0)  task5();
  
  //Increment ticker and reset if maximum frames are exceded
  ticCount++;
  if (ticCount > numFrames) ticCount = 1;
}

void task1() { // Period  4ms  Rate 250Hz

  monitor.jobStarted(1);
  
  // Create waveform
  digitalWrite(t1Pin, HIGH);
  delayMicroseconds(200);
  digitalWrite(t1Pin, LOW);
  delayMicroseconds(50);
  digitalWrite(t1Pin, HIGH);
  delayMicroseconds(30);
  digitalWrite(t1Pin, LOW);

  monitor.jobEnded(1);
}


void task2() { // Period 20ms Rate 50Hz
  monitor.jobStarted(2);
  
  double period = (double) pulseIn(t2Pin, HIGH) * 2;  // Measure period during high signal and multiply by 2 due to 50% duty cycle
  freqT2 = periodToFreq(period);  //Calculate the Frequency

  monitor.jobEnded(2);
}

void task3() { // Period 8ms  Rate 125Hz
  monitor.jobStarted(3);
  
  double period = (double) pulseIn(t3Pin, HIGH) * 2;  // Measure period during high signal and multiply by 2 due to 50% duty cycle
  freqT3 = periodToFreq(period);  //Calculate the Frequency

  monitor.jobEnded(3);
}

void task4() {  // Period 20ms  Rate 50Hz
  monitor.jobStarted(4);
  
  // Read analogue signal and increment through array for next reading
  // Analogue signal converted to 12 bit integer
  anIn[currInd] = analogRead(t4AnalogPin);
  currInd = (currInd + 1) % numParams;
  
  // find mean average of array
  double filteredIn = 0;
  for (uint8 i = 0; i < numParams; i++) {
    filteredIn += anIn[i];
  }
  filteredIn /= numParams;

  digitalWrite(t4LEDPin, (filteredIn > t4Thresh));  // Turn on LED if average is above the threshhold

  monitor.jobEnded(4);
}

void task5() {  // Period 100ms  Rate 10Hz
  monitor.jobStarted(5);
  
  // Map frequencies from 333Hz to 1000Hz into 0 - 99
  // Constrain to 0 - 99 as map() only creates gradient
  int normFreqT2 = map(freqT2, t2MinFreq, t2MaxFreq, t5Min, t5Max);
  normFreqT2 = constrain(normFreqT2, t5Min, t5Max);

  // Map frequencies from 500Hz to 1000Hz into 0 - 99
  // Constrain to 0 - 99 as map() only creates gradient
  int normFreqT3 = map(freqT3, t3MinFreq, t3MaxFreq, t5Min, t5Max);
  normFreqT3 = constrain(normFreqT3, t5Min, t5Max);

  // Print to monitor
  Serial.print(normFreqT2);
  Serial.print(",");
  Serial.println(normFreqT3);

  monitor.jobEnded(5);
}

void loop() {}