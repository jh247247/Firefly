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
IPAddress ESPmDNS::getIpFromHostname(const char * hostname, int length)
{
  udp.beginMulticast(WiFi.localIP(), mDnsAddress, MDNS_PORT);

  // ### SEND REQUEST ###
  
  // Create Request Packet
  int mdnsPacketSize = 29;
  byte requestPacketBuffer[mdnsPacketSize];
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
  int i;
  for (i = 0; i < length; i++) {
    requestPacketBuffer[13+i] = hostname[i];
  }

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
    delay(500);
    packetLength = udp.parsePacket();
  }

  if (packetLength > MIN_MDNS_PACKET_LENGTH) {
    // Read in packet
    byte recvPacketBuffer[packetLength];
    udp.read(recvPacketBuffer, packetLength);

    // Check there is an answer
    if (recvPacketBuffer[7] > 0) {
      
      // Check if ans has correct hostname
      int match = 1;
      int j;
      for (j = 12; j < 12+length; j++) {
        if (recvPacketBuffer[j] != requestPacketBuffer[j]) {
          match = 0;
          break;
        }
      }
      if (match) { // Hostname matches
        
        // Get IP from bytes
        int ipStartPoint = 12+length+1+17;
        return IPAddress(recvPacketBuffer[ipStartPoint], recvPacketBuffer[ipStartPoint+1], recvPacketBuffer[ipStartPoint+2], recvPacketBuffer[ipStartPoint+3]);
      }
    }
  }

  return -1; // 255.255.255.255
}

