// These constants won't change.  They're used to give names
// to the pins used:

// Sensors' pin
const int MicInPin = A0;  // Analog input pin that the microphone is attached to
const int PhotodlnInPin = A1;  // Analog input pin that the photodarlington is attached to
const int ThermtrInPin = A2;  // Analog input pin that the thermistor is attached to
const int DistanceInPin = A3; // Analog input pin that the distance sensor is attached to
const int WindowInPin = 2;
//Actuators' pin
const int SpeakerOutPin = 8; // Speaker connected from digital pin 8 to ground
const int LedOutPin = 12; // LED connected from digital pin 12 to ground
const int MotorOutPin = 11; // Motor connected from digital pin 13 to ground

const int sensorNum = 5;
int sensorValue[sensorNum] = {0};        // initialize value read from the pot
int outputValue[sensorNum] = {0};        // initialize value output to the PWM (analog out)
int print_mask[sensorNum] = {0};       // 0 if the data is not going to show, 1 otherwise

const int MAX_DISTANCE = 1000;
int noiseLevel = 0;
int ledState = LOW;

boolean humanState = false;
int smoothedDistance = 0;

enum tempUnit{Kelvin,Celcius,Fahrenheit};
void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  pinMode(LedOutPin, OUTPUT);
  pinMode(SpeakerOutPin, OUTPUT);
  pinMode(MotorOutPin, OUTPUT);
  digitalWrite(MotorOutPin, HIGH);
  pinMode(WindowInPin, INPUT);
  //digitalWrite(MotorOutPin, LOW);
  //flashLed(LedOutPin, 3, 500);
  //playDisappointedSound();
  establishContact();  // send a byte to establish contact until receiver responds 
}

void loop() {
  // for php
  serialCallResponse();
}

void getSensorData() {
  // read the analog in value:
  sensorValue[0] = analogRead(MicInPin);
  sensorValue[1] = analogRead(PhotodlnInPin);  
  sensorValue[2] = analogRead(ThermtrInPin);  
  sensorValue[3] = analogRead(DistanceInPin);
  sensorValue[4] = getWindowState();
  
  // Sensor calibration
  outputValue[0] = map(sensorValue[0],  500, 700, 0, 255);  
  outputValue[1] = map(sensorValue[1],  0, 1023, 0, 255);  
  outputValue[2] = thermistorCalibration(sensorValue[2], Celcius);  
  outputValue[3] = distanceCalibration(sensorValue[3]);
  outputValue[4] = sensorValue[4];
}

int getWindowState(){
  int reading = digitalRead(WindowInPin);
  if(reading == HIGH) return LOW;
  return HIGH;
}

void giveCandies(){
  digitalWrite(MotorOutPin, LOW);
  digitalWrite(MotorOutPin, HIGH);   // candy coming
  delay(100);
  digitalWrite(MotorOutPin, LOW);
  delay(100);
}

void serialCallResponse(){
  if(Serial.available() > 0) {
    int inByte = Serial.read();
    int i;
    if (inByte == 'A') {
        giveCandies();
    }else if (inByte == 'B') {
        getSensorData();
        for(i = 0; i < sensorNum -1; i++){
              Serial.print(outputValue[i]);
              Serial.print(",");
        }
        Serial.println(outputValue[i]);
    }else if (inByte == 'C'){
        playDisappointedSound();
    }
  }
}

// This is the calibration function for Sharp Distance Sensor 2Y0A02
// Note: this distance sensor can measure from 20 cm to 150 cm
int distanceCalibration(int input){
  // Valid raw data range is from 80 ~ 490
  if(input >= 80 && input <= 490)
    return 9462/(input - 16.92);
  return MAX_DISTANCE;
}

// This is the calibration function for thermistor P/N:NTCLE413E2103H400
// parameter RawADC is the analogReading from the thermistor
// parameter Unit is the temperature unit of the return value
double thermistorCalibration(int RawADC, int Unit) {
 long double temp;
 long double A,B,C,D;

// this is the coefficient for the thermistor P/N:NTCLE413E2103H400
  A = 0.0012;
  B = 2.2614e-004;
  C = 7.0822e-007;
  D = 6.7885e-008;
  double R = 1000;
  double RT = (1024*R/RawADC) - R;
  
// Steinhart–Hart equation
// {1 \over T} = A + B \ln(R) + C (\ln(R))^3 \, 
// check wiki for more info http://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation
   temp = log(RT);
   long double divisor = (A + (B + (C + D * temp)* temp )* temp);
   temp = 1/ divisor;
  if(Unit == Kelvin)
    return temp;
  else if(Unit == Celcius)
    return temp = temp - 273.15;            // Convert Kelvin to Celcius
  else if(Unit == Fahrenheit)
    return temp = (temp * 9.0)/ 5.0 + 32.0; // Convert Celcius to Fahrenheit
  
}

void ledControl(int input){
   if(input!= ledState){
      digitalWrite(LedOutPin, input);
      ledState = input;
   }
}

void flashLed(int pin, int times, int wait) {
    for (int i = 0; i < times; i++) {
      digitalWrite(pin, HIGH);
      delay(wait);
      digitalWrite(pin, LOW);
      if (i + 1 < times) {
        delay(wait);
      }
    }
}

void playDisappointedSound() {
  for (int i = 0; i <= 60; i++) {
    tone(SpeakerOutPin, 880 * pow(2, -i / 60.0));
    delay(5);
    noTone(SpeakerOutPin);
  }
}

void establishContact() {
  while (Serial.available() <= 0) {
    int i;
    for(i = 0; i < sensorNum -1; ++i){ Serial.print("0,");}
    Serial.println("0");
    delay(500);
  }
}
