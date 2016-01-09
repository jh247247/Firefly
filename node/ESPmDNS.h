#ifndef ESPmDNS_h
#define ESPmDNS_h

#include "Esp.h"
#include "ESP8266WiFi.h"
#include "WiFiUdp.h"

#define MDNS_PORT 5353
#define MIN_MDNS_PACKET_LENGTH 34

class ESPmDNS
{
  public:
    ESPmDNS(void);
    IPAddress getIpFromHostname(const char * hostname);
    
  private:
    WiFiUDP udp;
    IPAddress mDnsAddress;
};

#endif

