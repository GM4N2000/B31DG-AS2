// Pin numbers
#define t1Pin        0 
#define t2Pin        2   
#define t3Pin        3   
#define t4AnalogPin  1   
#define t4LEDPin    19 

// Timings
#define t2P       10  
#define t2PWait   1   
#define t2PWait2   7   
#define t3P       2   
#define t4P       5   
#define t5P       25  

// Unique parameters
#define baudRate       9600  
#define tickLength     4     
#define numFrames      50    

#define t2Timeout   3100  
#define t2MinFreq   333   
#define t2MaxFreq   1000  

#define t3Timeout   2100  
#define t3MinFreq   500   
#define t3MaxFreq   1000  

#define numParams      4     
#define t4Thresh    2048  

#define t5Min       0     
#define t5Max       99    

#define periodToFreq(T) (1 / (T / 1000000))

typedef unsigned char uint8;
typedef unsigned int uint16;
typedef unsigned long uint32;

#include <B31DGMonitor.h>
#include <Ticker.h>

// Globals
B31DGCyclicExecutiveMonitor monitor;

uint8 ticCount;
Ticker ticker;

double freqT2;
double freqT3;

uint16 anIn[numParams];
uint8 currInd;

void setup() {
  Serial.begin(baudRate);

  pinMode(t1Pin, OUTPUT); 
  pinMode(t2Pin, INPUT);
  pinMode(t3Pin, INPUT); 
  pinMode(t4AnalogPin, INPUT); 
  pinMode(t4LEDPin, OUTPUT); 

  currInd = 0;
  for (uint8 i = 0; i < numParams; i++) {
    anIn[i] = 0;
  }

  ticCount = 0;
  monitor.startMonitoring();
  ticker.attach_ms(tickLength, cycle);
  cycle();
}

void cycle() {
  task1();
  if (((ticCount - t2PWait) % t2P) == 0) task2();
  if (((ticCount - t2PWait2) % t2P) == 0) task2();
  if ((ticCount % t3P) == 0)  task3();
  if ((ticCount % t4P) == 0)  task4();
  if ((ticCount % t5P) == 0)  task5();
  
  ticCount++;
  if (ticCount > numFrames) ticCount = 1;
}

void task1() {
  monitor.jobStarted(1);
  
  digitalWrite(t1Pin, HIGH);
  delayMicroseconds(200);
  digitalWrite(t1Pin, LOW);
  delayMicroseconds(50);
  digitalWrite(t1Pin, HIGH);
  delayMicroseconds(30);
  digitalWrite(t1Pin, LOW);

  monitor.jobEnded(1);
}

void task2() {
  monitor.jobStarted(2);
  
  double period = (double) pulseIn(t2Pin, HIGH) * 2;
  freqT2 = periodToFreq(period);

  monitor.jobEnded(2);
}

void task3() {
  monitor.jobStarted(3);
  
  double period = (double) pulseIn(t3Pin, HIGH) * 2;
  freqT3 = periodToFreq(period);

  monitor.jobEnded(3);
}

void task4() {
  monitor.jobStarted(4);
  
  anIn[currInd] = analogRead(t4AnalogPin);
  currInd = (currInd + 1) % numParams;
  
  double filteredIn = 0;
  for (uint8 i = 0; i < numParams; i++) {
    filteredIn += anIn[i];
  }
  filteredIn /= numParams;

  digitalWrite(t4LEDPin, (filteredIn > t4Thresh));

  monitor.jobEnded(4);
}

void task5() {
  monitor.jobStarted(5);
  
  int normFreqT2 = map(freqT2, t2MinFreq, t2MaxFreq, t5Min, t5Max);
  normFreqT2 = constrain(normFreqT2, t5Min, t5Max);
  int normFreqT3 = map(freqT3, t3MinFreq, t3MaxFreq, t5Min, t5Max);
  normFreqT3 = constrain(normFreqT3, t5Min, t5Max);
  Serial.print(normFreqT2);
  Serial.print("---");
  Serial.println(normFreqT3);

  monitor.jobEnded(5);
}

void loop() {}