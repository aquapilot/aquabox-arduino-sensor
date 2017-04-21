/*
 * This file is part of the Aquapilot package.
 * <p>
 * (c) Sébastien Vermeille <sebastien.vermeille@gmail.com>
 * <p>
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */


#include "RF24.h"
#include "RF24Network.h"
#include "RF24Mesh.h"
#include <SPI.h>
#include <ArduinoJson.h>
#include <DallasTemperature.h>
#include <TimerOne.h>

#define LED_PIN 4

// How many bits to use for temperature values: 9, 10, 11 or 12
#define SENSOR_RESOLUTION 9

// Index of sensors connected to data pin, default: 0
#define SENSOR_INDEX 0

/**** Configure the nrf24l01 CE and CS pins ****/
RF24 radio(7, 8);
RF24Network network(radio);
RF24Mesh mesh(radio, network);

/**
   User Configuration: nodeID - A unique identifier for each radio. Allows addressing
   to change dynamically with physical changes to the mesh.
   In this example, configuration takes place below, prior to uploading the sketch to the device
   A unique value from 1-255 must be configured for each node.
   This will be stored in EEPROM on AVR devices, so remains persistent between further uploads, loss of power, etc.
 **/
#define nodeID 1

#define MESH_NOMASTER  // The nomaster option reduces program space and memory usage. Can be used on any node except for the master (nodeID 0) 

uint32_t displayTimer = 0;
uint32_t measureTimer = 0;
struct payload_t {
  unsigned long ms;
  unsigned long counter;
};

#define MAX_MESSAGE_LENGTH 50

char sensorValue[MAX_MESSAGE_LENGTH] = {};

#define ONE_WIRE_BUS 6  // Temperature sensor pin

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress sensorDeviceAddress;

  // Memory pool for JSON object tree.
  //
  // Inside the brackets, 200 is the size of the pool in bytes.
  // If the JSON object is more complex, you need to increase that value.
  StaticJsonBuffer<MAX_MESSAGE_LENGTH> jsonBuffer;

  // StaticJsonBuffer allocates memory on the stack, it can be
  // replaced by DynamicJsonBuffer which allocates in the heap.
  // It's simpler but less efficient.
  //
  // DynamicJsonBuffer  jsonBuffer;

  // Create the root of the object tree.
  //
  // It's a reference to the JsonObject, the actual bytes are inside the
  // JsonBuffer with all the other nodes of the object tree.
  // Memory is freed when jsonBuffer goes out of scope.
  JsonObject& dto = jsonBuffer.createObject();

 String temperature;
 float temp;

 boolean hasMeasured = false;

void requestMeasure(){

    sensors.setWaitForConversion(false);  // makes it async
    sensors.requestTemperatures(); // Tell the DS18B20 to get make a measurement
    // Takes approx 750ms

  
    
}


void sendMeasure(){

  dto["UUID"] = nodeID;
  dto["type"] = "temp";
  dto["measure"] = temperature;
  dto.printTo(sensorValue, sizeof(sensorValue));
    
    if (!mesh.write(&sensorValue, 'D', sizeof(sensorValue))) {

      // If a write fails, check connectivity to the mesh network
          
     if ( ! mesh.checkConnection() ) {
          //refresh the network address
          Serial.println(F("Renewing Address"));
          mesh.renewAddress();
        } else {
          Serial.println(F("Send fail, Test OK"));
        }
        
      } else {
        Serial.print(F("Send OK: ")); 
        Serial.println(sensorValue);
      }
    

}

void initSensor(){
    // Start reading sensors
  sensors.begin();
  sensors.getAddress(sensorDeviceAddress, 0);
  sensors.setResolution(sensorDeviceAddress, SENSOR_RESOLUTION);
}

void initMesh(){
  
  // Set the nodeID manually
  mesh.setNodeID(nodeID);
  
  // Connect to the mesh
  mesh.begin();
}

int freeRam() {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup() {

  Serial.begin(115200);
  delay(3000);

  pinMode(LED_PIN, OUTPUT);

Timer1.initialize(1000000);
Timer1.attachInterrupt(timerIsr);

  initMesh();
  initSensor();


}

int timerCycle = 0;

void timerIsr(){
  if(timerCycle > 16){
    timerCycle = 0;
  }
  timerCycle++;

  digitalWrite(LED_PIN, digitalRead(LED_PIN) ^ 1);

  if(timerCycle % 8 == 0) {
    Serial.println(F("SEND MEASURE"));

    temp = sensors.getTempCByIndex(SENSOR_INDEX); // Get temperature
    temp+=273.15; // Convert it to SI unit -> Kelvin
    temperature = String(temp, 2);
    
    sendMeasure();
  }

  else if(timerCycle % 4 == 0) {
      Serial.println(freeRam());
    Serial.println(F("REQUEST MEASURE"));
    requestMeasure();
  }

  
  
}

void loop(){
  mesh.update();

  while (network.available()) {

    RF24NetworkHeader header;
    payload_t payload;
    network.read(header, &payload, sizeof(payload));
    Serial.print("Received packet #");
     Serial.print(payload.counter);
    Serial.print(" at ");
    Serial.println(payload.ms);
    
  }
  
}




