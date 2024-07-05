#include <stdint.h>
#include "../ta-utils.h"

struct RTHPPacketShort {
  uint8_t key;
  uint8_t data;
  uint8_t addr_low;
  uint8_t addr_high;
};

enum RTHPInplFeatureFlags {
  RTHPIMPLFLAGS_BIDIRECTIONAL = 1,
  RTHPIMPLFLAGS_VARIABLESPEED = 2,
  RTHPIMPLFLAGS_USESHORTPACKET = 4,
  RTHPIMPLFLAGS_USELONGPACKET = 8,
  RTHPIMPLFLAGS_USECUSTOMPACKET = 16,
  RTHPIMPLFLAGS_USERAWPACKET = 32,
  RTHPIMPLFLAGS_PLUGNPLAY = 64
};

struct RTHPImplInfo {
  const char* name, description;
  int flags;
};

struct RTHPDevice {
  int id;
  const char* name;

};

enum RTHPErrors {
  RTHP_SUCCESS = 0,

  RTHP_PORTERROR = -100,
  RTHP_PORT_CLOSED,
  
  RTHP_WRITEERROR = -200

};

class RTHPImpl {
  std::vector<RTHPDevice> devs;
  int currectDev;
    /*
     * 
     */
  public:
    /*
     * get the information about the impl
     * @return a RTHPImplInfo struct
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
    virtual int sendRaw(char* data);
    virtual int deinit();
};