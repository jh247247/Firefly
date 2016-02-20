/*
  Basic ESP8266 MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
  it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
  else switch it off

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
  http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/


#include <Arduino.h>
#include <Esp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <RF24.h>
#include "ESPmDNS.h"

// magics.
#define PRINT_HEX(h,l) {int d_i; Serial.print(h[l-1],HEX);for(d_i = l-2; d_i >= 0; d_i--){Serial.print(":");Serial.print(h[d_i],HEX);}}
#define WAIT_FOR_PACKET(r,t,to) {int d_start = micros(); while(!r.available()){if(micros() - d_start > t*1000){to=true;break;}}}

// Prototypes
void setup_wifi();
void setup_host();
void callback(char* topic, byte* payload, unsigned int length);
void recvFakePacket();


// nrf globals
RF24 g_radio_nrf(16, 15);
byte rx_address[6] = {0x78, 0x70, 0x24, 0x72, 0x74};
byte tx_address[6] = {0x70, 0x70, 0x24, 0x72, 0x74};
byte mac[32];
byte rx_packet[9];

// Update these with values suitable for your network.
const char* ssid = "linksys";
const char* password = "0000000000";
const char* monitorHostname = "raspberrypi";

// Networking globals
WiFiClient espClient;
PubSubClient client(espClient);
ESPmDNS espmDNS;
IPAddress monIp = INADDR_NONE;

// MQTT globals
char msg[128];

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);

  g_radio_nrf.begin();
  g_radio_nrf.printDetails();

  g_radio_nrf.setPALevel(RF24_PA_MAX);
  g_radio_nrf.setAutoAck(false);
  g_radio_nrf.setPayloadSize(32); // we want to send the mac address so we can confirm.

  g_radio_nrf.openWritingPipe(tx_address);
  g_radio_nrf.openReadingPipe(1, rx_address); // use pipe 1 instead of pipe 0 to ensure rx addr is not overwritten

  g_radio_nrf.startListening();

  WiFi.macAddress(mac);
  setup_wifi();

  setup_host();

  client.setServer(monIp, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_host() {
  Serial.println("Discovering Monitor IP...");
  monIp = INADDR_NONE;
  while(monIp == INADDR_NONE) {
    monIp = espmDNS.getIpFromHostname(monitorHostname);
  }


  Serial.print("Monitor IP: ");
  Serial.println(monIp);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void reconnect() {
  setup_host();

  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void loop() {
  bool timeout = false;

  WAIT_FOR_PACKET(g_radio_nrf,1000,timeout);
  if (!timeout) {
    Serial.print("Packet received: -");
    g_radio_nrf.read(rx_packet, sizeof(rx_packet));

    PRINT_HEX(rx_packet,sizeof(rx_packet));
    Serial.print(" @ ");
    Serial.println(millis());

    snprintf (msg, 128,
              "{\"nodeId\":\"%X%X%X\",\"firefly\":{\"fireflyID\":\"%X%X%X%X\",\"status\":\"%X\",\"bat\":%d,\"temp\":%d,\"timer\":%d}}",
              mac[0],mac[1],mac[2], // nodeid
              rx_packet[0],rx_packet[1],rx_packet[2],rx_packet[3], // fireflyid
              rx_packet[4], // status
              (uint16_t)rx_packet[5]+((uint16_t)rx_packet[6]<<8), // bat
              rx_packet[7], // temp
              rx_packet[8]); // wdt

    g_radio_nrf.stopListening();
    if(!g_radio_nrf.write(mac,6,true)){
      Serial.print("Sending ");
      PRINT_HEX(mac,sizeof(mac));
      Serial.println(" via NRF radio failed!");
    }
    g_radio_nrf.startListening();
  } else {
    // just send heartbeat
    snprintf (msg, 128, "{\"nodeId\":\"%X%X%X\"}", mac[0],mac[1],mac[2]);
    Serial.println("No packet received!");
  }



  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  Serial.print("Publish message with node data: ");
  Serial.println(msg);
  client.publish("node", msg);

  delay(500);
}
