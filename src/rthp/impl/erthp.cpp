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

#include "erthp.h"

RTHPImplInfo ERTHP::getInfo() {
  return RTHPImplInfo(
    "E-RTHP",
    "use a plain serial port to send register writes to whatever",
    RTHPIMPLFLAGS_VARIABLESPEED|RTHPIMPLFLAGS_USESHORTPACKET|RTHPIMPLFLAGS_USERAWPACKET|RTHPIMPLFLAGS_PLUGNPLAY|RTHPIMPLFLAGS_USEINFOPACKET|RTHPIMPLFLAGS_USELEGACYPACKET,
    {DIV_SYSTEM_OPLL,DIV_SYSTEM_OPLL_DRUMS,DIV_SYSTEM_PCSPKR,DIV_SYSTEM_BIFURCATOR},
    RTHPOSFLAGS_LINUX|RTHPOSFLAGS_WINDOWS,
    {"Display mode"},
    1
  );
}

ERTHP::ERTHP() {
  ports.clear();
  devs.clear();
  currectDev = 0;
  running = false;
  rate = 0;
  timeout = 0;
  chip = 0;

  // port
}

int ERTHP::listDevices() {
  ports=serial::list_ports();
  devs.clear();
  for (unsigned long int i = 0; i < ports.size(); i++) {
    devs.push_back(RTHPDevice(i,ports[i].port.c_str()));
  }
  return devs.size();
}

std::vector<RTHPDevice> ERTHP::getDeviceList() {
  return devs;
}

bool ERTHP::isRunning() {
  return running;
}

void ERTHP::setChip(int _chip) {
  (void)_chip;
  chip = 0;
}

int ERTHP::init(int dev, unsigned int _rate, unsigned int tout) {
  if (running) return RTHP_SUCCESS;
  currectDev = dev;
  rate = _rate;
  timeout = tout;
  if (devs.size() < 1) return RTHP_ERROR;
  port.setPort(devs[currectDev].name);
  port.setBaudrate(rate);
  port.setTimeout(serial::Timeout::max(),timeout,0,timeout,0);
  
  try {
    port.open();
  } catch (std::exception& e) {
    logE("ERTHP: init failed! %s",e.what());
    return RTHP_INITERROR;
  }
  if (!port.isOpen()) return RTHP_PORT_CLOSED;
  running = true;
  
  return RTHP_SUCCESS;
}

int ERTHP::sendPacket(RTHPPacketLegacy p) {
  try {
    return port.write(
      fmt::sprintf("%c%c%c%c",p.key,p.data,p.addr_low,p.addr_high)
    ) == sizeof(RTHPPacketLegacy)?RTHP_SUCCESS:RTHP_WRITEBAD;
  } catch (std::exception& e) {
    logE("ERTHP: send failed! %s",e.what());
    deinit();
    return RTHP_WRITEERROR;
  }
  return RTHP_UNKNOWN;
}

int ERTHP::sendPacket(RTHPPacketShort p) {
  try {
    return port.write(
      fmt::sprintf("%c%c%c%c",p.key1,p.key2,p.data,p.addr)
    ) == sizeof(RTHPPacketShort)?RTHP_SUCCESS:RTHP_WRITEBAD;
  } catch (std::exception& e) {
    logE("ERTHP: send failed! %s",e.what());
    deinit();
    return RTHP_WRITEERROR;
  }
  return RTHP_UNKNOWN;
}

int ERTHP::sendPacket(RTHPPacketInfo p) {
  try {
    return port.write(
      fmt::sprintf("%c%c%s\xff%s\xff",p.key1,p.key2,p.sname,p.sauth)
    ) > 4?RTHP_SUCCESS:RTHP_WRITEBAD;
  } catch (std::exception& e) {
    logE("ERTHP: send failed! %s",e.what());
    deinit();
    return RTHP_WRITEERROR;
  }
  return RTHP_UNKNOWN;
}

int ERTHP::sendPacket(RTHPPacketParameter p) {
  try {
    return port.write(
      fmt::sprintf("%c%c%c%c",p.key1,p.key2,p.param,p.value)
    ) == sizeof(RTHPPacketParameter)?RTHP_SUCCESS:RTHP_WRITEBAD;
  } catch (std::exception& e) {
    logE("ERTHP: send failed! %s",e.what());
    deinit();
    return RTHP_WRITEERROR;
  }
  return RTHP_UNKNOWN;
}

int ERTHP::sendRaw(char* data, size_t len) {
  try {
    return port.write((uint8_t*)data,len) == len?RTHP_SUCCESS:RTHP_WRITEBAD;
  } catch (std::exception& e) {
    logE("ERTHP: send failed! %s",e.what());
    deinit();
    return RTHP_WRITEERROR;
  }
  return RTHP_UNKNOWN;
}

int ERTHP::receive(char* buf, uint8_t len) {
  try {
    return port.read((uint8_t*)buf,len) == len?RTHP_SUCCESS:RTHP_ERROR;
  } catch (std::exception& e) {
    logE("ERTHP: receive failed! %s",e.what());
    deinit();
    return RTHP_ERROR;
  }
  return RTHP_UNKNOWN;
}

uint8_t ERTHP::receive() {
  uint8_t ret;
  try {
    port.read(&ret,1);
    return ret;
  } catch (std::exception& e) {
    logE("ERTHP: send failed! %s",e.what());
    deinit();
    return 0;
  }
  return 0;
}

int ERTHP::deinit() {
  port.close();
  running = false;
  return RTHP_SUCCESS;
}

ERTHP::~ERTHP() {
  if (running) port.close();
  devs.clear();
  ports.clear();
}