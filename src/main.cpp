/*
WIP

Eventually should be able to receive a list of arguments over tcp to make a series of patterns as well as arbitrary values for the motor in real time.

Potential inspiration
https://github.com/jcfain/TCodeESP32

*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <vector>

  // Wifi credentials
  const char* ssid = "espVibe";
  const char* password = "bruhmoment";

  // Motor pin. (must be PWM capable)
  int mot = 2;
  int btn = D7;

  // Number of steps to increase or decrease i by per loop. (influences the effect of speed setting)
  uint8_t step = 3;
  
  // Pseudo loop variable for patterns.
  uint8_t i;

  // Timekeeping for the loops.
  unsigned int oldMillis = 0;
  unsigned int currentMillis;
  unsigned int sTime;

  unsigned short int interval;
  
  
  // Getting around the problem of not using loops with flags.
  bool doOnce =false;

  // Wifi and TCP variables

  WiFiServer server(5045);
  WiFiClient client = server.available();
  IPAddress local_IP(192,168,4,1);

/*
Vibration stuff
*/



// Breathing pattern going between sStrength and eStrength for duration waiting 100/speed ms between each step.
void breathing(uint8_t sStrength, uint8_t eStrength, unsigned int duration, unsigned short int speed) {

  if (!doOnce) {
    doOnce = true;
    interval = 1000/constrain(speed, 1, 65535);
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
      doOnce = false;
  }
}

// Run at strength for time

void singleShot(uint8_t strength, unsigned int duration) {

  if (!doOnce) {
    doOnce = true;
    sTime = currentMillis;
    analogWrite(mot, strength);
  }
  if (currentMillis - sTime >= duration || duration > 0){ // If duration = 0 real time mode just continue until next input
    doOnce = false;
    digitalWrite(mot, LOW);
  }


  
}

// Function to create a square pattern going between sStrength and eStrength for duration ms alternating at 1/speed hz with T_sStrength/T_eStrength = 1/(ratio-1).
void pulsing(uint8_t sStrength, uint8_t eStrength, unsigned int duration, unsigned short int speed, uint8_t ratio = 2) {

  if (!doOnce) {
    doOnce = true;
    interval = 1000/constrain(speed, 1, 65535);
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
    doOnce = false;
  }


}

// Handle deserialization for the tcp interface.
struct PatternParams {
    uint8_t mode;
    uint8_t sStrength;
    uint8_t eStrength;
    uint32_t duration;
    uint16_t speed;
    uint8_t ratio;
  };

  char serialize(PatternParams* params) {
    return params->mode | params->sStrength << 8 | params->eStrength << 16 | params->duration << 24 | params->speed << 56 | params->ratio << 72;
  }

  PatternParams deserialize(char data) {
    PatternParams params{};
    params.mode = data & 0xFF; // First byte
    params.sStrength = data & 0xFF; // Next byte
    params.eStrength = data >> 8 & 0xFF; // Next byte
    params.duration = data >> 16 & 0xFFFFFFFF; // Next 4 bytes
    params.speed = data >> 48 & 0xFFFF; // Next 2 bytes
    params.ratio = data >> 64 & 0xFF; // Last byte
    return params;
}


void TCPServer () {
  if (client) {
    if (client.connected()) {
      Serial.println("Connected to client");   
    }
    if (client.available() > 0) {
      char data = client.read();
      PatternParams params;
      params = deserialize(data);
      server.write("It veurks wirelessly!\n");
      Serial.write(data);
      Serial.write("\n");
      Serial.write(params.mode);
    }
  }
}

// Function to handle the queuing of patterns and order of execution.
void pattern() {

}

void setup() {
  Serial.begin(115200);
  Serial.println("It veurks!");
  pinMode(mot, OUTPUT);
  pinMode(btn, INPUT);
  WiFi.softAP(ssid,password);
  server.begin();
}

void loop() {
  currentMillis = millis();
  TCPServer();
  

}