#include "ESPmDNS.h"

ESPmDNS::ESPmDNS() {
  mDnsAddress = IPAddress(224,0,0,251);
}

/*  Returns an ip address from a given hostname.local
 *  Returns 255.255.255.255 on error
 *  
 *  Based off documentation from:
 *  - https://en.wikipedia.org/wiki/Multicast_DNS
 *  - https://tools.ietf.org/html/rfc6762
*/

#ifndef PRINT_HEX
#define PRINT_HEX(h,l) {int d_i; Serial.print(h[l-1],HEX);for(d_i = l-2; d_i >= 0; d_i--){Serial.print(":");Serial.print(h[d_i],HEX);}}
#endif
IPAddress ESPmDNS::getIpFromHostname(const char * hostname)
{
  udp.beginMulticast(WiFi.localIP(), mDnsAddress, MDNS_PORT);
  int length = strlen(hostname);
  // ### SEND REQUEST ###
  
  // Create Request Packet
  int mdnsPacketSize = 24+length;
  byte requestPacketBuffer[mdnsPacketSize]; // malloc would be cleaner I guess, but this is safer for now.
  memset(requestPacketBuffer, 0, mdnsPacketSize);

  // Header
  requestPacketBuffer[0] = 0x00; // ID
  requestPacketBuffer[1] = 0x00;
  requestPacketBuffer[2] = 0x00; // Flags
  requestPacketBuffer[3] = 0x00;
  requestPacketBuffer[4] = 0x00; // QDCOUNT
  requestPacketBuffer[5] = 0x01;
  requestPacketBuffer[6] = 0x00; // ANCOUNT
  requestPacketBuffer[7] = 0x00;
  requestPacketBuffer[8] = 0x00; // NSCOUNT
  requestPacketBuffer[9] = 0x00;
  requestPacketBuffer[10] = 0x00; // ARCOUNT
  requestPacketBuffer[11] = 0x00;

  // Hostname data
  requestPacketBuffer[12] = length;
  memcpy(requestPacketBuffer+13,hostname,length);

  // Footer
  requestPacketBuffer[13+length] = 0x05; // Length
  requestPacketBuffer[14+length] = 0x6c; // l
  requestPacketBuffer[15+length] = 0x6f; // o
  requestPacketBuffer[16+length] = 0x63; // c
  requestPacketBuffer[17+length] = 0x61; // a
  requestPacketBuffer[18+length] = 0x6c; // l
  requestPacketBuffer[19+length] = 0x00; // NULL
  requestPacketBuffer[20+length] = 0x00; // QTYPE
  requestPacketBuffer[21+length] = 0x01;
  requestPacketBuffer[22+length] = 0x00; // QCLASS
  requestPacketBuffer[23+length] = 0x01;

  // Send Request Packet
  udp.beginPacketMulticast(mDnsAddress, MDNS_PORT, WiFi.localIP());
  udp.write(requestPacketBuffer, mdnsPacketSize);
  udp.endPacket();

  
  // ### RECV RESPONSE ###
  
  int packetLength = 0;
  while (!packetLength) {
    packetLength = udp.parsePacket();
  }
  

  if (packetLength > MIN_MDNS_PACKET_LENGTH) {
    // Read in packet
    byte recvPacketBuffer[packetLength];
    udp.read(recvPacketBuffer, packetLength);

    // Check there is an answer
    if (recvPacketBuffer[7] > 0) {
      
      // Check if ans has correct hostname
      if (memcmp(recvPacketBuffer+12,requestPacketBuffer+12,length) == 0) { // Hostname matches
        // Get IP from bytes
        int ipStartPoint = packetLength-4;
        udp.stop();
        return IPAddress(recvPacketBuffer[ipStartPoint], recvPacketBuffer[ipStartPoint+1], recvPacketBuffer[ipStartPoint+2], recvPacketBuffer[ipStartPoint+3]);
      }
    }
  }
  udp.stop();
  return INADDR_NONE;
}

