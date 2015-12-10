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


#include <Esp.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <RF24.h>

// nrf setup
RF24 g_radio_nrf(16, 15); // TODO: change depending on hardware config.
byte address[6] = "TEST1";
byte mac[6];
byte rx_packet[6];

// magics.
#define PRINT_MAC(m) {int d_i; Serial.print(mac[5],HEX);for(d_i = 4; d_i >= 0; d_i--){Serial.print(":");Serial.print(mac[d_i],HEX);}}
#define WAIT_FOR_PACKET(r,t,to) {int d_start = micros(); while(!r.available()){if(micros() - d_start > t*1000){to=true;break;}}} 

// Update these with values suitable for your network.

const char* ssid = "linksys";
const char* password = "0000000000";
const char* mqtt_server = "192.168.1.10"; // todo: resolve mDNS addr.



WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);

void setup() {
  ESP.wdtDisable();
  
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(18, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(15, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  g_radio_nrf.begin();
  g_radio_nrf.printDetails();

  g_radio_nrf.setPayloadSize(sizeof(mac)); // we want to send the mac address so we can confirm.
  g_radio_nrf.setPALevel(RF24_PA_LOW);
  g_radio_nrf.setAutoAck(false);



  g_radio_nrf.openWritingPipe(address);
  g_radio_nrf.openReadingPipe(1, address);

  g_radio_nrf.startListening();

  WiFi.macAddress(mac);
  //setup_wifi();
  //client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);
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
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
  yield();
  
  bool timeout = false;
  
  g_radio_nrf.stopListening();
  
  Serial.print("Sending: ");
  PRINT_MAC(mac);
  Serial.println(" via NRF radio");
  if(!g_radio_nrf.write(mac,6,true)){
    Serial.println("Packet write failed!");
  } else {
    Serial.println("Wrote packet!");
  }
  
  g_radio_nrf.startListening();
  delay(100);
//  delay(200);
//  //WAIT_FOR_PACKET(g_radio_nrf,500,timeout);
//  if (g_radio_nrf.available()) {
//    Serial.println("Packet recieved!");
//    //g_radio_nrf.read(rx_packet, sizeof(rx_packet));
//  } else {
//    Serial.println("No packet recieved!");
//  }

  //  if (!client.connected()) {
  //    reconnect();
  //  }
  //  client.loop();
  //
  //  long now = millis();
  //  if (now - lastMsg > 2000) {
  //    lastMsg = now;
  //    ++value;
  //    snprintf (msg, 75, "hello world #%ld", value);
  //    Serial.print("Publish message: ");
  //    Serial.println(msg);
  //    client.publish("outTopic", msg);
  //  }



}
