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

#define FAKE_FF_ID 1234
#define FAKE_FF_BATTERY 7

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
byte address[6] = {0xe7, 0xe7, 0xe7, 0xe7, 0xe7};
byte mac[6];
byte rx_packet[6];

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
char msg[50];
int value = 0;



void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);

  g_radio_nrf.begin();
  g_radio_nrf.printDetails();


  g_radio_nrf.setPALevel(RF24_PA_MIN);
  g_radio_nrf.setAutoAck(false);
  g_radio_nrf.setPayloadSize(32); // we want to send the mac address so we can confirm.

  g_radio_nrf.openWritingPipe(address);
  g_radio_nrf.openReadingPipe(0, address);

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
  uint8_t packetrec = 0;


  g_radio_nrf.stopListening();

  if(!g_radio_nrf.write(mac,6,true)){
    Serial.print("Sending ");
    PRINT_HEX(mac,sizeof(mac));
    Serial.println(" via NRF radio failed!");
  }

  g_radio_nrf.startListening();
  delay(200);
  WAIT_FOR_PACKET(g_radio_nrf,500,timeout);
  if (g_radio_nrf.available()) {
    Serial.print("Packet received: -");
    g_radio_nrf.read(rx_packet, sizeof(rx_packet));
    PRINT_HEX(rx_packet,sizeof(rx_packet));
    Serial.print(" @ ");
    Serial.println(millis());
    packetrec = 1;
  } else if (value % 5 == 0) {
    Serial.println("Fake packet received!");
    recvFakePacket();
  } else {
    Serial.println("No packet received!");
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  ++value;
  snprintf (msg, 75, "{\"nodeId\":\"%X%X%X\",\"val\":%d, \"prec\":%d}", mac[0],mac[1],mac[2],value,packetrec);
  Serial.print("Publish message with node data: ");
  Serial.println(msg);
  client.publish("node", msg);

  delay(500);
}

void recvFakePacket() {
  snprintf (msg, 75, "{\"fireflyId\":\"%d\",\"bat\":%d}", FAKE_FF_ID, FAKE_FF_BATTERY);
  Serial.print("Publish message with firefly data: ");
  Serial.println(msg);
  client.publish("firefly", msg);
}
