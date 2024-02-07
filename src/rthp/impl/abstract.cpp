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

String RTHP::getImplDescription() {
  return ""; // haha good luck localizing
}
bool RTHP::getOSCompat() {
#ifdef _WIN32 // Windows
  return true;
#elseif __APPLE__ // Mac
  return true;
#else // Linux
  return true;
#endif
}

int RTHP::scanDevices() {
  return 0;
}

void RTHP::setDeviceId(int id) {
  deviceId=id;
}

int RTHP::getDeviceId() {
  return deviceId;
}

String RTHP::getDeviceName() {
  return deviceNames[deviceId];
}

void RTHP::init() {
  
}

void RTHP::send(RTHPPacketShort p) {

}

void RTHP::send(RTHPPacketLong p) {

}

void RTHP::send(unsigned char c) {

}

void RTHP::send(String s) {

}

void RTHP::quit() {

}
