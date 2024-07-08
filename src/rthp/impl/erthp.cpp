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
    RTHPIMPLFLAGS_VARIABLESPEED|RTHPIMPLFLAGS_USESHORTPACKET|RTHPIMPLFLAGS_USERAWPACKET|RTHPIMPLFLAGS_PLUGNPLAY
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

int ERTHP::sendRegWrite(uint16_t addr, uint16_t value, RTHPPacketTypes packetType) {
  try {
    switch (packetType) {
      case RTHP_PACKET_SHORT:
        port.write(fmt::sprintf("%c%c%c%c",RTHPPACKETSHORT_KEY,value&0xff,addr&0xff,addr>>8));
        break;
      default: return RTHP_ERROR;
    }
  } catch (std::exception& e) {
    logE("ERTHP: send failed! %s",e.what());
    deinit();
    return RTHP_WRITEERROR;
  }
  return RTHP_SUCCESS;
}

int ERTHP::sendRaw(char* data, size_t len) {
  try {
    port.write((uint8_t*)data,len);
  } catch (std::exception& e) {
    logE("ERTHP: send failed! %s",e.what());
    deinit();
    return RTHP_WRITEERROR;
  }
  return RTHP_SUCCESS;
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