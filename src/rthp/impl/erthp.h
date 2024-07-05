#include "../rthp.h"
#include "serial/serial.h"

#ifndef ERTHP_H
#define ERTHP_H

class ERTHP: public RTHPImpl {
  private:
    serial::Serial port;
    std::vector<serial::PortInfo> ports;
    unsigned int baudrate;
    unsigned int timeout;

    std::vector<RTHPDevice> devs;
    int currectDev;
  public:
    RTHPImplInfo getInfo();
    int listDevices();
    void setParams(unsigned int rate, unsigned int tout);
    int init(int dev);
    int sendRegWrite(uint16_t addr, uint16_t value);
    int sendRaw(char* data, size_t len);
    int deinit();
    ERTHP();
    ~ERTHP();
};

#endif
