/*
WIP

Eventually should be able to receive a list of arguments over tcp to make a series of patterns as well as arbitrary values for the motor in real time.

*/

#include <Arduino.h>


  // Motor pin. (must be PWM capable)
  int mot = 2;

  // Number of steps to increase or decrease i by per loop. (influences the effect of speed setting)
  uint8_t step = 5;
  
  // Pseudo loop variable for patterns.
  uint8_t i;

  // Timekeeping for the loops.
  unsigned int oldMillis = 0;
  unsigned int currentMillis;
  unsigned int sTime;
  unsigned short int interval;
  
  // Getting around the problem of not using loops with flags.
  bool doOnce =false;
  bool execFinished;


// Breathing pattern going between sStrength and eStrength for duration waiting 100/speed ms between each step.
void breathing(uint8_t sStrength, uint8_t eStrength, unsigned int duration, unsigned short int speed) {

  if (!doOnce) {
    doOnce = true;
    interval = 100/speed;
    i = sStrength+step;
    sTime = currentMillis;
  }

  if (currentMillis - sTime <= duration){
    if (currentMillis - oldMillis >= interval){

        oldMillis = currentMillis;
        i += step;
        if(i >= eStrength || i <= sStrength) {step = -step;}
        analogWrite(mot, i);

    }
  }

  else {
      digitalWrite(mot, LOW);
      execFinished = true;
  }
}


// Function to create a square pattern going between sStrength and eStrength for duration ms alternating at 1/speed hz with T_sStrength/T_eStrength = 1/(ratio-1).
void pulsing(uint8_t sStrength, uint8_t eStrength, unsigned int duration, unsigned short int speed, uint8_t ratio) {

  if (!doOnce) {
    doOnce = true;
    interval = 1000/speed;
    sTime = currentMillis;
  }

  if (currentMillis - sTime <= duration){
    if (currentMillis - oldMillis >= interval - interval/ratio){
      if (currentMillis - oldMillis >= interval){
          oldMillis = currentMillis;
          analogWrite(mot, eStrength);
      }
        
      else {
          analogWrite(mot, sStrength);
      }
    }
  }
  else {
    digitalWrite(mot, LOW);
    execFinished = true;
  }


}

// Function to handle the queuing of patterns and order of execution.
void pattern() {



}



void setup() {
  pinMode(mot, OUTPUT);
}



void loop() {
  currentMillis = millis();

}