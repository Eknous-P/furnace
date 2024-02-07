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

#include "e-rthp.h"

String ERTHP::getImplDescription() {
  return "ERTHP Version 1.\n"
         "for sending regwrites via a serial port\n";
}

bool ERTHP::getOSCompat() {
#ifdef _WIN32 // Windows
  return true;
#elseif __APPLE__ // Mac
  return true;
#else // Linux
  return true;
#endif
}

int ERTHP::scanDevices() {
  deviceNames.clear();
  p.availPorts=serial::list_ports();
  for (serial::PortInfo p:p.availPorts) deviceNames.push_back(p.port);
  return p.availPorts.size();
}

void ERTHP::setDeviceId(int id) {
  deviceId=id;
}

int ERTHP::getDeviceId() {
  return deviceId;
}

String ERTHP::getDeviceName() {
  return deviceNames[deviceId];
}

void ERTHP::init() {
  
}

void ERTHP::send(RTHPPacketShort p) {

}

void ERTHP::send(RTHPPacketLong p) {

}

void ERTHP::send(unsigned char c) {

}

void ERTHP::send(String s) {

}

void ERTHP::quit() {

}
