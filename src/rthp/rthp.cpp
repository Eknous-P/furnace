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

#include "rthp.h"
#include "impl/erthp.h"
#include "impl/dummy.h"

RTHP::RTHP() {
  set=false;
  running=false;
  impl=0;
  i=NULL;
  dumpedChip=0;
  canDump=false;
}

RTHP::~RTHP() {
  reset();
}

int RTHP::setup(int _impl) {
  if (set) {
    logW("RTHP: implementation already set!");
    return RTHP_ERROR;
  }
  switch (_impl) {
    case RTHP_IMPL_DUMMY:
      i=new RTHPDummy;
      break;
    case RTHP_IMPL_ERTHP:
      i=new ERTHP;
      break;
    default:
      logE("RTHP: no impl given to set!");
      return RTHP_ERROR;
  }
  if (!i) {
    logE("RTHP: failed to set implementation!");
    return RTHP_ERROR;
  }

  set=true;
  i->listDevices();
  logV("RTHP: implementation %d set successfully", _impl);
  return RTHP_SUCCESS;
}

int RTHP::init(int dev, unsigned int _rate, unsigned int tout) {
  if (i) return i->init(dev, _rate, tout);
  logE("RTHP: no impl set to init!");
  return RTHP_ERROR;
}

int RTHP::scanWhitelist(DivSong* s, int c) {
  if (!i) return RTHP_ERROR;
  std::vector<DivSystem> whitelist = i->getInfo().chipWhitelist;
  canDump = false;
  if (i->getInfo().flags&RTHPIMPLFLAGS_MULTICHIP) {
    for (DivSystem c:whitelist) {
      for (int j=0; j<s->systemLen; j++) {
        if (c == s->system[j]) {
          canDump = true;
          return RTHP_SUCCESS;
        }
      }
    }
  } else {
    for (DivSystem ch:whitelist) {
      if (ch == s->system[c]) {
        canDump = true;
        return RTHP_SUCCESS;
      }
    }
  }
  return RTHP_SUCCESS;
}

int RTHP::reset() {
  if (set) {
    i->deinit();
    delete i;
    i=NULL;
    set=false;
    logV("RTHP: impl unset successfully");
  }
  return RTHP_SUCCESS;
}

RTHPImplInfo RTHP::getImplInfo() {
  return i->getInfo();
}

std::vector<RTHPDevice> RTHP::getDevices() {
  if (i) return i->getDeviceList();
  return {};
}

int RTHP::scanDevices() {
  if (i) return i->listDevices();
  return 0;
}

void RTHP::setDumpedChip(int c) {
  dumpedChip = c;
}
  
int RTHP::getDumpedChip() {
  return dumpedChip;
}

bool RTHP::canDumpChip() {
  return canDump;
}

bool RTHP::isSet() {
  return set;
}

bool RTHP::isRunning() {
  if (i) return i->isRunning();
  return false;
}

void RTHP::setPacketType(int type) {
  packetType = RTHPPacketTypes(type);
}

int RTHP::send(uint16_t addr, uint16_t value) {
  return send(0,addr,value);
}

int RTHP::send(int chip, uint16_t addr, uint16_t value) {
  if (!i) return RTHP_ERROR;
  if (!canDump) return RTHP_CANNOTDUMP;
  switch (packetType) {
    // first two packet types are single-chip only!
    case RTHP_PACKET_LEGACY:
      if (chip == dumpedChip) {
        return i->sendPacket(RTHPPacketLegacy(addr,value));
      }
      break;
    case RTHP_PACKET_SHORT:
      if (chip == dumpedChip) {
        return i->sendPacket(RTHPPacketShort(addr,value));
      }
      break;
    default: break;
  }
   return RTHP_SUCCESS;
}

int RTHP::sendRaw(char* data, size_t len) {
  if (!i) return RTHP_ERROR;
  if (!canDump) return RTHP_CANNOTDUMP;
  return i->sendRaw(data,len);
}

int RTHP::sendInfo(DivSong* s) {
  if (!i) return RTHP_ERROR;
  if ((i->getInfo().flags&RTHPIMPLFLAGS_USEINFOPACKET)==0) return RTHP_CANNOTDUMP;
  return i->sendPacket(RTHPPacketInfo(s->name,s->author));
}

int RTHP::sendParam(uint8_t param, uint8_t value) {
  if (!i) return RTHP_ERROR;
  if ((i->getInfo().customParamCount)<=param) return RTHP_ERROR;
  return i->sendPacket(RTHPPacketParameter(param,value));
}
