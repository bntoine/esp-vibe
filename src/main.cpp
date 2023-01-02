/**

Most of this code was taken from https://github.com/MonomoriumP/Buttplug.io--Lelo
I have tested this with xtoys.app, it should work with buttplug.io but I haven't tested
For now it emulates a lovense edge and gives you two channels with 20 steps.
You should be able to use https://stpihkal.docs.buttplug.io/docs/stpihkal/protocols/lovense/ to emulate a different toy from lovense.

**/


#include <Arduino.h>
//#include <analogWrite.h> // The ESP32 analogWrite library by ERROPiX is needed if you are using the arduino IDE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>


//          ================= Motor pins ================
uint8_t mtr1 = 13;
uint8_t mtr2 = 14;


BLEServer* pServer = NULL;
BLECharacteristic* pTxCharacteristic = NULL;
BLECharacteristic* pRxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;

// Only vibration1 and vibration2 are used here but I assume vibration is useful in vibrators with a single motor.

int vibration;
int vibration1;
int vibration2;

#define SERVICE_UUID           "50300001-0023-4bd4-bbd5-a6920e4c5653"
#define CHARACTERISTIC_RX_UUID "50300002-0023-4bd4-bbd5-a6920e4c5653"
#define CHARACTERISTIC_TX_UUID "50300003-0023-4bd4-bbd5-a6920e4c5653"



void SetMTR(void) {
  int power1 = map(vibration1, 0 , 20 , 0, 255);
  power1 = constrain(power1, 0, 255);
  analogWrite(mtr1, power1);

  int power2 = map(vibration2, 0 , 20 , 0, 255);
  power2 = constrain(power2, 0, 255);
  analogWrite(mtr2, power2);
}



class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



class MySerialCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      static uint8_t messageBuf[64];
      assert(pCharacteristic == pRxCharacteristic);
      std::string rxValue = pRxCharacteristic->getValue();

      
      if (rxValue == "DeviceType;") {
        memmove(messageBuf, "P:37:FFFFFFFFFFFF;", 18);
        // CONFIGURATION:               ^ Use a BLE address of the Lovense device you're cloning. (Doesn't seem to matter for xtoys. Might need to add radomization to avoid conflicts but I need to check with a second board I don't have yet.)
        pTxCharacteristic->setValue(messageBuf, 18);
        pTxCharacteristic->notify();
      } else if (rxValue == "Battery;") {
        memmove(messageBuf, "69;", 3);
        pTxCharacteristic->setValue(messageBuf, 3);
        pTxCharacteristic->notify();
      } else if (rxValue == "PowerOff;") {
        memmove(messageBuf, "OK;", 3);
        pTxCharacteristic->setValue(messageBuf, 3);
        pTxCharacteristic->notify();
      } else if (rxValue == "RotateChange;") {
        memmove(messageBuf, "OK;", 3);
        pTxCharacteristic->setValue(messageBuf, 3);
        pTxCharacteristic->notify();
      } else if (rxValue.rfind("Status:", 0) == 0) {
        memmove(messageBuf, "2;", 2);
        pTxCharacteristic->setValue(messageBuf, 3);
        pTxCharacteristic->notify();
      } else if (rxValue.rfind("Vibrate:", 0) == 0) {
        vibration = std::atoi(rxValue.substr(8).c_str());
        memmove(messageBuf, "OK;", 3);
        pTxCharacteristic->setValue(messageBuf, 3);
        pTxCharacteristic->notify();
        SetMTR();
      } else if (rxValue.rfind("Vibrate1:", 0) == 0) {
        vibration1 = std::atoi(rxValue.substr(9).c_str());
        memmove(messageBuf, "OK;", 3);
        pTxCharacteristic->setValue(messageBuf, 3);
        pTxCharacteristic->notify();
        SetMTR();
      } else if (rxValue.rfind("Vibrate2:", 0) == 0) {
        vibration2 = std::atoi(rxValue.substr(9).c_str());
        memmove(messageBuf, "OK;", 3);
        pTxCharacteristic->setValue(messageBuf, 3);
        pTxCharacteristic->notify();
        SetMTR();
      } else {
        memmove(messageBuf, "ERR;", 4);
        pTxCharacteristic->setValue(messageBuf, 4);
        pTxCharacteristic->notify();
      }
    }
};

void setup() {
  
  
  pinMode(mtr1, OUTPUT);
  pinMode(mtr2, OUTPUT);
  delay(100);
  digitalWrite(mtr1, LOW);
  delay(100);
  analogWrite(mtr1, 127);
  delay(100);
  digitalWrite(mtr1, LOW);
  delay(100);
 
  
  // Create the BLE Device
  BLEDevice::init("LVS-EDGE"); // CONFIGURATION: The name doesn't actually matter, The app identifies it by the reported id.

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristics
  pTxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_TX_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pTxCharacteristic->addDescriptor(new BLE2902());

  pRxCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_RX_UUID,
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );
  pRxCharacteristic->setCallbacks(new MySerialCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
}

void loop() {
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(100); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}