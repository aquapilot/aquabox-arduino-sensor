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
#include "S_message.h"
#include "printf.h"

#define BRIGHTNESS_SENSOR_PIN 3

#define LED_PIN 4

RF24 radio(7, 8); // nRF24L01(+) CE and CS pins
RF24Network network(radio);
RF24Mesh mesh(radio, network);

// Address of our node
const uint16_t this_node = 1;

// Address of the other node
const uint16_t other_node = 0;

// How often to send 'hello world to the other unit
const unsigned long interval = 2000; //ms

// When did we last send?
unsigned long last_sent;

// How many have we sent already
unsigned long packets_sent;

uint16_t lost_packets = 0;

// Structure of our payload
struct payload_t
{
    unsigned long ms;
    unsigned long counter;
};

void setup(void)
{
    Serial.begin(57600);
    printf_begin();
    printf_P(PSTR("\n\rRF24Network/examples/sensornet/\n\r"));

    SPI.begin();
    radio.begin();
    network.begin(/*channel*/ 90, /*node address*/ this_node);
}

void loop(void)
{
    // Pump the network regularly
    network.update();

    // If it's time to send a message, send it!
    unsigned long now = millis();

    S_message message;
    message.measure = measure_brightness(); 
    message.voltage_reading = measure_voltage(); 
    message.lost_packets = min(lost_packets,0xff);
    message.sensorType = BRIGHTNESS;
    char message_type = 'M';


    // By default send to the router
    uint16_t to_node = 0;

   printf_P(PSTR("---------------------------------\n\r"));
    printf_P(PSTR("%lu: APP Sending type-%c %s to 0%o...\n\r"),millis(),message_type,message.toString(),to_node);
   
    RF24NetworkHeader header(to_node,message_type);

    bool ok = network.write(header,&message,sizeof(message));
    if (ok)
    {
      lost_packets = 0;
      printf_P(PSTR("%lu: APP Send ok\n\r"),millis());
    }
    else
    {
      ++lost_packets;
      printf_P(PSTR("%lu: APP Send failed\n\r"),millis());
    }


    delay(4000);

}

uint32_t measure_brightness(){

    uint32_t reading = 2;
    reading = analogRead(BRIGHTNESS_SENSOR_PIN);
    
    return reading;
}

uint32_t measure_voltage(){
  uint32_t reading = 3;

    return reading;
}



