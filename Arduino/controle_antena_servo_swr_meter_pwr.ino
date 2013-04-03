
/*
 * Copyright (C) 2012 The Antenna Tuner Controlled by Android and Arduino 
 *               via Bluetooth Project. By PU2CLR - Ricardo Lima Caratti.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Antenna Tuner Controlled by Android and Arduino via Bluetooth
 * Author: Ricardo Lima Caratti - PU2CLR.
 * 
 */


#include <Servo.h> 

#define LED_WORKING  12
#define LED_LIMIT_REACHED 13

#define RELAY_PIN 8       // Relay to connect SRW Meter to the Arduino 
#define SERVO_PIN 9       // Digital input to Servo
#define SWR_APIN  3       // AnalogInput

#define MIN_PULSE    510
#define MAX_PULSE    2400


#define INC_EXTRA_FINE_ANGLE  1
#define INC_FINE_ANGLE        5
#define INC_FAST_ANGLE       20

#define OFFSET_ADJUSTMENT    100


int inputLength;       // Indica o numero de caracteres lidos  

Servo  capacitor;

int    capacitor_position = 0;
int    increment = 0;

void setup() { 
  Serial.begin(9600);
  
  capacitor.attach(SERVO_PIN,MIN_PULSE,MAX_PULSE);   // attaches the servo 
  capacitor_position = MIN_PULSE + (MAX_PULSE - MIN_PULSE) / 2;
  capacitor.writeMicroseconds( capacitor_position );
  pinMode(LED_WORKING, OUTPUT); 
  pinMode(LED_LIMIT_REACHED, OUTPUT);
  
  pinMode(RELAY_PIN, OUTPUT);   
  digitalWrite(RELAY_PIN, LOW);
} 


void blinkLed(int led, int times) 
{
  int i;
  for ( i = 0; i < times; i++ ) {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
  }    
}


void  centralizesCapacitor()
{
    capacitor_position = MIN_PULSE + (MAX_PULSE - MIN_PULSE) / 2;
    capacitor.writeMicroseconds(capacitor_position);
}


// Tell to the user that the tune process has finished
void tellFinish() {
  int i;
  for (i = 0; i < 5; i++ ) {
      digitalWrite(LED_WORKING, HIGH);
      delay(200);
      digitalWrite(LED_LIMIT_REACHED, HIGH);
      delay(500);
      digitalWrite(LED_WORKING, LOW);
      delay(200);
      digitalWrite(LED_LIMIT_REACHED, LOW);
      delay(500);
  }    
}



/*
**
** Check if cancel command was sent
** return
** 1 = true
** 0 = false
*/
int cancel() {

    char c;
  
    if (Serial.available())  {
      c = Serial.read();
      if ( c == 'C' || c == 'c' ) return 1;   
    }
    
   return 0; 
}


int readAnalogInput(int aPin) {
  int firstValue;
  int secondValue;
  
  firstValue = analogRead(aPin);
  delay(8);
  secondValue = analogRead(aPin);
  
  return (firstValue > secondValue)? firstValue:secondValue;
}


/* AutoTune
** Scan lowest SWR
** Move the servo of the capacitor of the antenna tuner, the Arduino logs the information obtained from the SWR meter as well the capacitor position.
** Once known position of the capacitor which results in lowest SWR, the tuning is performed.
** minPosition   - Initial position of the capacitor
** maxPosition   - Final position of the capacitor
** offsetAdjust  - Not used for while
*/
void AutoTune(int minPosition, int maxPosition, int offsetAdjust) {
  
     int i; 
     
     // triggers the relay. Connect the Arduino to the SWR meter.
     digitalWrite(RELAY_PIN, HIGH);
     delay(250);
     
     // Start scaning. Positions the capacitor according to the range to be searched.
     increment = 0;
     capacitor_position = minPosition;
     capacitor.writeMicroseconds(capacitor_position);
     
     int maxValue = 0;
     int positionMaxValue;

     int swrValue;
     int firstReading;
     
     // Register the lowest SWR
     for ( i = minPosition; i <= maxPosition; i++ ) {
       
       swrValue = readAnalogInput(SWR_APIN);
       
       if ( swrValue > maxValue ) {
           maxValue = swrValue;
           positionMaxValue = i;
       }
       capacitor.writeMicroseconds(i);
       // Check cancel command
       if ( cancel() ) {
          centralizesCapacitor();
          return;
       } 
       delay(10);
     }
 
     firstReading = maxValue;    // Record the maxmum value read (first scan)
     
     // capacitor.writeMicroseconds(minPosition);
     // delay(1000);
     capacitor.writeMicroseconds(minPosition);     
     delay(150);
     
     // Register the lowest SWR
     for (i = minPosition; i <= maxPosition; i++) {
       
       swrValue = readAnalogInput(SWR_APIN);
       
       if ( swrValue >= firstReading || i == positionMaxValue ) 
          break;
  
       capacitor.writeMicroseconds(i);
       // Check cancel command
       if ( cancel() ) {
          centralizesCapacitor();
          return;
       } 
       delay(10);
     }
     //  The SWR meter turns off. 
     digitalWrite(RELAY_PIN, LOW);
     // Now RXTX is connected direct to the antenna   
     // Tell to the user that this process has just finished
     tellFinish();
     // Save the capacitor position;
     capacitor_position = i;
}

void loop() {
  // read string from buffer
  if (Serial.available()) {
    switch ( Serial.read() ) {
      case '+':
      case '=':
           increment = INC_EXTRA_FINE_ANGLE; 
           break;
      case '-':
           increment = -INC_EXTRA_FINE_ANGLE;
           break;     
      case 'l':
           increment = -INC_FINE_ANGLE;
           break;
      case 'r':
           increment = INC_FINE_ANGLE;
           break;
      case 'L':
           increment = -INC_FAST_ANGLE;   
           break;
      case 'R':
           increment = INC_FAST_ANGLE;
           break;  
      case 'M':
           increment = 0;
           capacitor_position = MIN_PULSE;
           capacitor.writeMicroseconds(capacitor_position);
           blinkLed(LED_LIMIT_REACHED, 3);  
           break;  
      case 'm':
           increment = 0;
           capacitor_position = MAX_PULSE;
           capacitor.write(capacitor_position);
           blinkLed(LED_LIMIT_REACHED, 3);
           break;  
      case 'C':  
      case 'c': 
           centralizesCapacitor();
           break;
      case 'S':
           AutoTune(MIN_PULSE + (MAX_PULSE - MIN_PULSE) / 2, MAX_PULSE, 0);
           increment = 0;
           break;           
      case 's': 
           AutoTune(MIN_PULSE, MIN_PULSE + (MAX_PULSE - MIN_PULSE) / 2, 0);
           increment = 0;
           break;           
       default:
          increment = 0;
          break;
    }

    // Rotates the capacitor as the increment
    if ( increment ) {
       capacitor_position += increment;
      if ( capacitor_position < MIN_PULSE) {
          blinkLed(LED_LIMIT_REACHED, 3);
          capacitor_position = MIN_PULSE;
       }
       else if ( capacitor_position > MAX_PULSE ) {
          blinkLed(LED_LIMIT_REACHED,3);
          capacitor_position = MAX_PULSE;       
       }
       else {   
            digitalWrite(LED_WORKING, HIGH);
             capacitor.writeMicroseconds(capacitor_position);
            delay(10);
            digitalWrite(LED_WORKING, LOW);
       }  

    }

  } 
}

