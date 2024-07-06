#include "../rthp.h"

class RTHPDummy: public RTHPImpl {
  private:
    std::vector<RTHPDevice> devs;
    int currectDev;
    unsigned int chip;
    unsigned int rate;
    unsigned int timeout;
    bool running;
  public:
    RTHPImplInfo getInfo();
    int listDevices();
    int init(int dev, unsigned int _rate, unsigned int tout);
    bool isRunning();
    void setChip(int _chip);
    int sendRegWrite(uint16_t addr, uint16_t value, RTHPPacketTypes packetType);
    int sendRaw(char* data, size_t len);
    int deinit();
    RTHPDummy();
    ~RTHPDummy();
};