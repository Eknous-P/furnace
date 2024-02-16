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

#include "engine.h"
#include "rthp.h"
#include "../ta-log.h"

#include "../rthp/impl/e-rthp.h"

const char* RTHPContainer::getImplName(int i) {
  switch (i) {
    case RTHP_NONE: return "*NONE*";
    case RTHP_ERTHP: return "E-RTHP";
  }
}

void RTHPContainer::preinit(RTHPImplementations impl, int deviceId) {
  RTHPImpl=NULL;
  impln=impl;
  switch (impl) {
    case RTHP_ERTHP:
      RTHPImpl=new ERTHP;
      RTHPImpl->setDeviceId(deviceId);
      RTHPImpl->scanDevices();
      state=0x01;
      break;
    case RTHP_NONE: default:
      state=0xff;
      break;
  }
}

void RTHPContainer::init() {
  switch (impln) {
    case RTHP_ERTHP:
      RTHPImpl->init();
      state=0x00;
      break;
    case RTHP_NONE: default:
      state=0xff;
      break;
  }
}

unsigned char RTHPContainer::getState() {
  return state;
}

int RTHPContainer::getImplId() {
  return impln;
}

void RTHPContainer::quit() {
  RTHPImpl->quit();
  RTHPImpl=NULL;
  state=0xff;
}
