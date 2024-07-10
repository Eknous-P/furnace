/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2024 tildearrow and contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdint.h>
#include "../ta-utils.h"
#include "../ta-log.h"
#include <fmt/printf.h>
#include "../engine/song.h"

#ifndef RTHP_H
#define RTHP_H

#define RTHPPACKETINFO_KEY '$'

enum RTHPImplementations {
  RTHP_IMPL_NONE=-1,
  RTHP_IMPL_DUMMY=0,

  RTHP_IMPL_ERTHP,
// future impls here

  RTHP_IMPL_MAX
};

enum RTHPPacketTypes {
  RTHP_PACKET_LEGACY,
  RTHP_PACKET_SHORT,
  RTHP_PACKET_LONG, // WIP
  RTHP_PACKET_INFO
};

/* old RTHP packet format
 * not that good as every write have to be EXACTLY 4 bytes
 * otherwise getting the start and end of each packet was pure guesswork
 * (especially worse when writes used to "drop")
 * - note: devices supporting this packet may not receive other packets simultaneously
 * #### packet key: 0x3E (>)
 */
struct RTHPPacketLegacy {
  uint8_t key;
  uint8_t data;
  uint8_t addr_low;
  uint8_t addr_high;
  RTHPPacketLegacy(uint16_t a, uint8_t d):
    key(0x3e)
    {
      addr_low=a&0xff;
      addr_high=a>>8;
      data=d;
    }
};

/* new(!) RTHP short packet format 
 * hopefully the 2 bytes for the key would make receiving it easier
 * #### packet key: 0xfff0
 */
struct RTHPPacketShort {
  uint8_t key1, key2;
  uint8_t addr;
  uint8_t data;
  RTHPPacketShort(uint8_t a, uint8_t d):
  key1(0xff),key2(0xf0)
  {
    addr=a;
    data=d;
  }
};

/* song information packet
 * sort-of wip, may be updated a lot
 * #### packet key: 0xfff7
 */
struct RTHPPacketInfo {
  uint8_t key1;
  uint8_t key2;
  String sname;
  String sauth;
  RTHPPacketInfo(String sn, String sa):
  key1(0xff),key2(0xf7)
  {
    sname=sn;
    sauth=sa;
  }
};

enum RTHPInplFeatureFlags {
  // data can be both sent and received
  RTHPIMPLFLAGS_BIDIRECTIONAL = 1,
  // data transfer speed can be changed
  RTHPIMPLFLAGS_VARIABLESPEED = 2,
  // can read legacy packet (rthp v1)
  RTHPIMPLFLAGS_USELSHORTPACKET = 4,
  // can read long packet (WIP)
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
  RTHPIMPLFLAGS_SAMPLEDUMP = 256,
  // can read info packet
  RTHPIMPLFLAGS_USEINFOPACKET = 512,
  // can read legacy packet (rthp v1)
  RTHPIMPLFLAGS_USELEGACYPACKET = 1024,
};

enum RTHPOSCompatFlags {
  RTHPOSFLAGS_LINUX = 1,
  RTHPOSFLAGS_WINDOWS = 2
};

struct RTHPImplInfo {
  const char* name;
  const char* description;
  int flags;
  short osCompat;
  std::vector<DivSystem> chipWhitelist;
  RTHPImplInfo(const char* n, const char* d, int f, std::vector<DivSystem> l, short o) {
    name = n;
    description = d;
    flags = f;
    chipWhitelist = l;
    osCompat = o;
  };
};

struct RTHPDevice {
  int id;
  const char* name;
  RTHPDevice( int _id, const char* _name) {
    id = _id;
    name = _name;
  };
};

enum RTHPExitCodes {
  RTHP_SUCCESS = 0,
  RTHP_UNKNOWN = 1,
  RTHP_ERROR = -1,

  RTHP_PORTERROR = -100,
  RTHP_PORT_CLOSED,
  
  RTHP_WRITEERROR = -200,
  RTHP_WRITEBAD,
  RTHP_CANNOTDUMP,

  RTHP_INITERROR = -300,

};

class RTHPImpl {
  // protected:
    // the list of available devices
    std::vector<RTHPDevice> devs;
    // the id of the current device
    int currectDev;
    // (optional) the chip being dumped
    unsigned int chip;
    // (optional) the communication speed
    unsigned int rate;
    // (optional) the communication timeout
    unsigned int timeout;
    // whether the implementation is running and connected to a device
    bool running;
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
     * get the list of the deivces
     * @return a std::vector of RTHPDevice's
     */
    virtual std::vector<RTHPDevice> getDeviceList();
    /*
     * initialize the implementation
     * @param dev the device number to connect to
     * @param _rate the communication speed
     * @param tout the communication timeout
     * @return a RTHP exit code
     */
    virtual int init(int dev, unsigned int _rate, unsigned int tout);
    /*
     * whether the implementation is connected to a device and is active
     * @return the bool `running`
     */
    virtual bool isRunning();
    /*
     * sets the chip to be dumped (ineffective for "multichip" impls)
     * @param _chip number of the chip to set
     */
    virtual void setChip(int _chip);
    /*
     * 
     */
    virtual int sendPacket(RTHPPacketLegacy p);
    virtual int sendPacket(RTHPPacketShort p);
    virtual int sendPacket(RTHPPacketInfo p);
    virtual int sendRaw(char* data, size_t len);
    virtual int receive(char* buf, uint8_t len);
    virtual uint8_t receive();
    virtual int deinit();
    virtual ~RTHPImpl();
};

class RTHP {
  private:
    RTHPImpl* i;
    bool set, running, canDump;
    int impl, dumpedChip;
    RTHPPacketTypes packetType;
  public:
    bool isSet();
    bool isRunning();
    void setPacketType(int type);
    int getPacketType();
    RTHPImplInfo getImplInfo();
    std::vector<RTHPDevice> getDevices();
    int scanDevices();
    void setDumpedChip(int c);
    int getDumpedChip();
    bool canDumpChip();

    int setup(int _impl);
    int init(int dev, unsigned int _rate, unsigned int tout);
    int scanWhitelist(DivSong* s, int c);
    int reset();
    int send(uint16_t addr, uint16_t value);
    int send(int chip, uint16_t addr, uint16_t value);
    int sendRaw(char* data, size_t len);
    int sendInfo(DivSong* s);
    int dumpSamples();
    RTHP();
    ~RTHP();
};

#endif
