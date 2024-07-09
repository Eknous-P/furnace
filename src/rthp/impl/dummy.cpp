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

#include "dummy.h"

RTHPImplInfo RTHPDummy::getInfo() {
  return RTHPImplInfo("RTHP Dummy Implementation","does absolutely nothing",0,{});
}

int RTHPDummy::listDevices() {
  return 0;
}

std::vector<RTHPDevice> RTHPDummy::getDeviceList() {
  return devs;
}

int RTHPDummy::init(int dev, unsigned int _rate, unsigned int tout) {
  running=true;
  return RTHP_SUCCESS;
}

bool RTHPDummy::isRunning() {
  return running;
}

void RTHPDummy::setChip(int _chip) {
  chip=_chip;
}

int RTHPDummy::sendRegWrite(uint16_t addr, uint16_t value, RTHPPacketTypes packetType) {
  return RTHP_SUCCESS;
}

int RTHPDummy::sendRaw(char* data, size_t len) {
  return RTHP_SUCCESS;
}

int RTHPDummy::deinit() {
  running=false;
  return RTHP_SUCCESS;
}

RTHPDummy::RTHPDummy() {
  devs.clear();
  currectDev=0;
  chip=0;
  rate=0;
  timeout=0;
  running=false;
}

RTHPDummy::~RTHPDummy() {

}