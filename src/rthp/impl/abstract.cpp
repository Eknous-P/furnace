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

#include "../rthp.h"

RTHPImplInfo RTHPImpl::getInfo() {
  return RTHPImplInfo(NULL,NULL,0,{},0);
}

int RTHPImpl::listDevices() {
  return 0;
}

std::vector<RTHPDevice> RTHPImpl::getDeviceList() {
  return {};
}

int RTHPImpl::init(int dev, unsigned int _rate, unsigned int tout) {
  return RTHP_SUCCESS;
}

bool RTHPImpl::isRunning() {
  return false;
}

void RTHPImpl::setChip(int _chip) {
  // . . .
}

int RTHPImpl::sendPacket(RTHPPacketLegacy p) {
  return RTHP_SUCCESS;
}

int RTHPImpl::sendPacket(RTHPPacketShort p) {
  return RTHP_SUCCESS;
}

int RTHPImpl::sendPacket(RTHPPacketInfo p) {
  return RTHP_SUCCESS;
}

int RTHPImpl::sendRaw(char* data, size_t len) {
  return RTHP_SUCCESS;
}

int RTHPImpl::receive(char* buf, uint8_t len) {
  return RTHP_SUCCESS;
}

uint8_t RTHPImpl::receive() {
  return 0;
}

int RTHPImpl::deinit() {
  return RTHP_SUCCESS;
}

RTHPImpl::~RTHPImpl() {

}