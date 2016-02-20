<> -*- mode: org -*-

* Packet structure

** Outgoing
   Upon waking from sleep, the firefly will send a packet containing:
   - Unique ID (4 bytes)
   - Status byte
     - Bit 0: if button has been pressed this iteration
       - Enables fireflies to be used as remote?
     - Bit 1-3: Power of transmit, try to mirror this on the node.
   - Current battery life (unsigned 2 byte)
     - Note that this is VREF being measured, so it may have some
       fluctuation from one device to another.
   - Current temperature (signed 1 byte, celcius)
     - This is integrated into the chip, why not include it?
     - May be used to determine area of very high/low temps?
     - Would be cool to make a temperature map of building maybe...
   - Current watchdog timeout (2 bytes)

** Incoming
   Immediately after sending the previous packet, the firefly listens
   for a packet formatted as follows:
   - Firefly unique ID (4 bytes)
     - used so only the sending firefly will respond
   - LED action byte (1 byte)
     - Bit 7-5 Red bit color value
     - Bit 4-2 Green bit color value
     - Bit 1-0 Blue bit color value
     - Set to 0 to keep current setting
   - General action byte (1 byte)
     - Bit 0 LED enable
       - Note that if the led is flashing, pressing the button will
         turn it off as well.
   - New watchdog timer value
     - If keeping the same watchdog value, set to 0xFFFF in node code
