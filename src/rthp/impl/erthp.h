#include "../rthp.h"
#include "serial/serial.h"

#ifndef ERTHP_H
#define ERTHP_H

class ERTHP: public RTHPImpl {
  private:
    serial::Serial port;
    std::vector<serial::PortInfo> ports;

    std::vector<RTHPDevice> devs;
    unsigned int chip;
    unsigned int rate;
    unsigned int timeout;
    int currectDev;
    bool running;
  public:
    RTHPImplInfo getInfo();
    int listDevices();
    int init(int dev, unsigned int rate, unsigned int tout);
    void setChip(int _chip);
    int sendRegWrite(uint16_t addr, uint16_t value, RTHPPacketTypes packetType);
    int sendRaw(char* data, size_t len);
    int deinit();
    ERTHP();
    ~ERTHP();
};

#endif
