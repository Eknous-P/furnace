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
  p.port=deviceId;
  p.baudrate=57600;
  p.timeout=10;
  try {
    p.sp.setPort(deviceNames[p.port].c_str());
    p.sp.setBaudrate(p.baudrate);
    p.sp.setTimeout(serial::Timeout::max(),p.timeout,0,p.timeout,0);
    p.sp.open();
  } catch (std::exception& xc) {

  }

}

void ERTHP::send(RTHPPacketShort pac) {
  p.sp.write(&pac.key,1);
  p.sp.write(&pac.data,1);
  p.sp.write(&pac.addrlow,1);
  p.sp.write(&pac.addrhigh,1);
}

void ERTHP::send(RTHPPacketLong pac) {
}

void ERTHP::send(unsigned char c) {
  p.sp.write(&c,1);
}

void ERTHP::send(String s) {
  p.sp.write(s);
}

void ERTHP::quit() {
  p.sp.close();
}
