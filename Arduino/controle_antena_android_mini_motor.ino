
/*
 *
 * Mini Stepper Motor Controlled by Android and Arduino via Bluetooth
 * Author: Ricardo Lima Caratti - PU2CLR.
 * 
 */


#include <Stepper.h> 

#define REV 250
#define FAST_SPEED    120
#define MEDIUM_SPEED   90
#define SLOW_SPEED     50

Stepper  capacitor(REV, 8,9,10,11);

void setup() { 
  Serial.begin(9600);
  capacitor.setSpeed(MEDIUM_SPEED);
  doSleep();
} 

void doSleep() {
  
  digitalWrite(8,LOW);
  digitalWrite(9,LOW);
  digitalWrite(10,LOW);
  digitalWrite(11,LOW);

}

/*
** Looking for the best condition
** Obs: To be implemented
*/
void scan() {
  
  int i;
  char c;
  
  capacitor.setSpeed(FAST_SPEED);
  capacitor.step(-100);
  capacitor.step(200);
  capacitor.step(-250);
  capacitor.setSpeed(SLOW_SPEED);
  
  for(i = 0; i < 250; i++ ) {
    capacitor.step(1);
    // TO DO
    if (Serial.available())  {
      c = Serial.read();
      if ( c == 'C' || c == 'c' ) {        // Cancel if something is sent.
          capacitor.setSpeed(MEDIUM_SPEED); 
          return;   
      }
    }
    delay(50);
  }
  
  capacitor.setSpeed(MEDIUM_SPEED);
  capacitor.step(-100);  

  doSleep();

}

void loop() {
  
  
  // read string from buffer (Bluetooth)
  if (Serial.available()) {
    
    switch ( Serial.read() ) {
      case '+':
      case '=':
           capacitor.step(1);
           break;
      case '-':
           capacitor.step(-1);
           break;     
      case 'l':
           capacitor.step(-20);
           break;
      case 'r':
           capacitor.step(20);
           break;
      case 'L':
           capacitor.step(-100);
           break;
      case 'R':
           capacitor.step(100);;   
           break;  
       case 'S':
       case 's':
           scan();
           break; 
       default:
          doSleep();
          break;
    }

  } 
}

