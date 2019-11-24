#include <Arduino.h>

// motor sheild
#define motorREnable 11
#define motorLEnable 10

#define motorRPin1 12
#define motorRPin2 9
#define motorLPin1 8
#define motorLPin2 7

// sensor array
#define c1 A0
#define c2 A1 
#define c3 A2
#define c4 A3
#define c5 A4
#define clp A5
#define near A6

// int irSensorDigital[5] = {0,0,0,0,0};

// binary representation of the sensor reading 
//from left to right when facing the same direction as the robot
int irSensors = B00000; 

int count = 0; // number of sensors detecting the line

// A score to determine deviation from the line [-180 ; +180]. 
// Negative means the robot is left of the line.
int error = 0;  

int errorLast = 0;  //  store the last value of error

// A correction value, based on the error from target. 
// It is used to change the relative motor speed with PWM.
int correction = 0; 

int lap = 0; // keep track of the laps

/* Set up maximum speed and speed for turning (to be used with PWM) */
int maxSpeed = 60; // used for PWM to control motor speed [0 - 255]

/* variables to keep track of current speed of motors */
int motorLSpeed = 0;
int motorRSpeed = 0;


/* functions-declared for c++ style code */

void Scan();
void Drive();
void UpdateError();
void UpdateCorrection();

void setup() {
  Serial.begin(9600);
  
  /* Set up ir array as input */
  pinMode(c1,INPUT);
  pinMode(c2,INPUT);
  pinMode(c3,INPUT);
  pinMode(c4,INPUT);
  pinMode(c5,INPUT);
  pinMode(clp,INPUT);
  pinMode(near,INPUT);

   /* Set up motor controll pins as output */
  pinMode(motorLPin1,OUTPUT);        
  pinMode(motorLPin2,OUTPUT);
  pinMode(motorLEnable,OUTPUT);
  
  pinMode(motorRPin1,OUTPUT);        
  pinMode(motorRPin2,OUTPUT);
  pinMode(motorREnable,OUTPUT);

}

void loop() {

     Scan();
     UpdateError();
     UpdateCorrection();
     Drive();

}

void Scan() {
  // Initialize the sensor counter and binary value
  count = 0;
  irSensors = B00000;

  int irSensorDigital[5] = {digitalRead(c1),digitalRead(c2),digitalRead(c3),digitalRead(c4),digitalRead(c5)};
    
  for (int i = 0; i < 5; i++) {
    count = count + irSensorDigital[i];
    int b = 4-i;
    irSensors = irSensors + (irSensorDigital[i]<<b);
    }
     // Debugging
     // Serial.println(irSensors);    

}


void UpdateError() {
  
  errorLast = error;  
  
  switch (irSensors) {
     
    case B000000:
       if (errorLast < 0) { error = -180;}
       else if (errorLast > 0) {error = 180;}
       break;
     
     case B10000: // leftmost sensor on the line
       error = -150;
       break;

     case B01000: 
       error = -30;
       break;

     case B00100:  
       error = 30;
       break;
       
     case B00010: 
       error = 90;
       break;           

     case B00001: // rightmost sensor on the line
       error = 150;
       break;
       
/* 2 Sensors on the line */         
     
     case B11000:
       error = -120;
       break;
      
     case B01100:
       error = -60;
       break;

     case B00110: 
       error = 0;
       break;

     case B00011: 
       error = 60;
       break;           

     case B01010:
       error = 0;
       break;

/* 3 Sensors on the line */    
       
     case B11100:
     case B01110:
       error = -150;
       break;
      
     case B00111:
       error = 150;
       break;
       
     case B10011:
       error = 150;
       break;      

 /* 4 Sensors on the line */       
     case B11110:
       error = -150;
       break;
       
     case B11101:
       error = -150;
       break;
      
     case B01111:
       error = 150;
       break;
       
     case B10111:
       error = 150;
       break;
       
     case B11011:
       error = 0;
       break;       

/* 5 Sensors on the line */      
     case B11111:
       error = 0;
       break;
      
     default:
      error = errorLast;
      Serial.print("Unhandled case: ");
      Serial.print(count);
      Serial.print("| ");
      Serial.println(irSensors);    
  }
}

void UpdateCorrection() {

  if (error >= 0 && error < 30) {
    correction = 0;
  }
  
  else if (error >=30 && error < 60) {
    correction = 15;
  }
  
  else if (error >=60 && error < 90) {
    correction = 40;
  }
  
  else if (error >=90 && error < 120) {
    correction = 55;
  }  
  
  else if (error >=120 && error < 150) {
    correction = 75;
  } 
  
  else if (error >=150 && error < 180) {
    correction = 255;
  }   

  else if (error >=180) {
    correction = 305;
  }

  if (error <= 0 && error > -30) {
    correction = 0;
  }
  
  else if (error <= -30 && error > -60) {
    correction = -15;
  }
  
  else if (error <= -60 && error > -90) {
    correction = -40;
  }
  
  else if (error <= -90 && error > -120) {
    correction = -55;
  }  
  
  else if (error <= -120 && error > -150) {
    correction = -75;
  } 
  
  else if (error <= -150 && error > -180) {
    correction = -255;
  }   

  else if (error <= -180) {
    correction = -305;
  }
  
  /* Adjust the correction value if maxSpeed is less than 255 */
  correction = (int) (correction * maxSpeed / 255 + 0.5);
  
  if (correction >= 0) {
    motorRSpeed = maxSpeed - correction;
    motorLSpeed = maxSpeed;
  }
  
  else if (correction < 0) {
    motorRSpeed = maxSpeed;
    motorLSpeed = maxSpeed + correction;
  }
}

void Drive() {
  if (motorRSpeed > 255) {motorRSpeed = 255;}
  else if (motorRSpeed < -255) {motorRSpeed = -255;}
  
  if (motorLSpeed > 255) {motorLSpeed = 255;}
  else if (motorLSpeed < -255) {motorLSpeed = -255;}
  
  if (motorRSpeed > 0) { // right motor forward (using PWM)
     analogWrite(motorREnable, motorRSpeed);
     digitalWrite(motorRPin1, HIGH);
     digitalWrite(motorRPin2, LOW);
  } 
  
  else if (motorRSpeed < 0) {// right motor reverse (using PWM)
     analogWrite(motorREnable, abs(motorRSpeed));
     digitalWrite(motorRPin1, LOW);
     digitalWrite(motorRPin2, HIGH);
  } 
  
  else if (motorRSpeed == 0) { // right motor fast stop
     digitalWrite(motorREnable, HIGH);
     digitalWrite(motorRPin1, LOW);
     digitalWrite(motorRPin2, LOW);
  }
  
  if (motorLSpeed > 0) { // left motor forward (using PWM)
     analogWrite(motorLEnable, motorLSpeed);
     digitalWrite(motorLPin1, HIGH);
     digitalWrite(motorLPin2, LOW);
  } 
  
  else if (motorLSpeed < 0) { // right motor reverse (using PWM)
     analogWrite(motorLEnable, abs(motorLSpeed));
     digitalWrite(motorLPin1, LOW);
     digitalWrite(motorLPin2, HIGH);
  } 
  
    else if (motorLSpeed == 0) { // left motor fast stop
     digitalWrite(motorLEnable, HIGH);
     digitalWrite(motorLPin1, LOW);
     digitalWrite(motorLPin2, LOW);
  }
}
