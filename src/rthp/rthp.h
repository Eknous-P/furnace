#include <stdint.h>
#include "../ta-utils.h"

#ifndef RTHP_H
#define RTHP_H

enum RTHPImplementations {
  RTHP_IMPL_DUMMY,
  RTHP_IMPL_ERTHP
};

struct RTHPPacketShort {
  uint8_t key;
  uint8_t data;
  uint8_t addr_low;
  uint8_t addr_high;
};

enum RTHPInplFeatureFlags {
  // data can be both sent and received
  RTHPIMPLFLAGS_BIDIRECTIONAL = 1,
  // data transfer speed can be changed
  RTHPIMPLFLAGS_VARIABLESPEED = 2,
  // can use the short packet
  RTHPIMPLFLAGS_USESHORTPACKET = 4,
  // can use the long packet (WIP)
  RTHPIMPLFLAGS_USELONGPACKET = 8,
  // can use a custom packet (WIP)
  RTHPIMPLFLAGS_USECUSTOMPACKET = 16,
  // can send raw data
  RTHPIMPLFLAGS_USERAWPACKET = 32,
  // device(s) can be plugged in and out during operation
  RTHPIMPLFLAGS_PLUGNPLAY = 64,
  // can dump multiple chips
  RTHPIMPLFLAGS_MULTICHIP = 128,
  // can dump sample data (WIP)
  RTHPIMPLFLAGS_SAMPLEDUMP = 256
};

struct RTHPImplInfo {
  const char* name;
  const char* description;
  int flags;
  RTHPImplInfo(const char* n, const char* d, int f) {
    name = n;
    description = d;
    flags = f;
  };
};

struct RTHPDevice {
  int id;
  const char* name;

};

enum RTHPErrors {
  RTHP_SUCCESS = 0,
  RTHP_ERROR = -1,

  RTHP_PORTERROR = -100,
  RTHP_PORT_CLOSED,
  
  RTHP_WRITEERROR = -200

};

class RTHPImpl {
  protected:
    std::vector<RTHPDevice> devs;
    int currectDev;
    /*
     * 
     */
  public:
    /*
     * get the information about the impl
     * @return a RTHPImplInfo struct containing the info
     */
    virtual RTHPImplInfo getInfo();
    /*
     * list any available devices and store in std::vector<RTHPDevice> devs
     * @return the number of devices found
     */
    virtual int listDevices();
    /*
     * 
     */
    virtual int init(int dev);
    virtual int sendRegWrite(uint16_t addr, uint16_t value);
    virtual int sendRaw(char* data, size_t len);
    virtual int deinit();
    virtual ~RTHPImpl();
};

class RTHP {
  private:
    RTHPImpl* i;
    bool set, running;
    int impl, dumpedChip;
  public:
    int setup(int _impl);
    int reset();
    int send(uint16_t addr, uint16_t value);
    int send(int chip, uint16_t addr, uint16_t value);
    int send(char* data, size_t len);
    int dumpSamples();
    RTHP();
    ~RTHP();
};

#endif
