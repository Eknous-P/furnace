/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

ERTHP erthp;

void RTHPContainer::init(RTHPImplementation setImpl) {
  impl=setImpl;
  logI("RTHP: begin init");
  logI("RTHP: using impl %s", RTHPImplementationNames[impl]);
  switch (impl) {
    case RTHP_ERTHP: {
      if (erthp.initSerial("/dev/ttyUSB0",9600,1000)) { // temporary port
        logE(erthp.getLastLog().c_str());
      }
      break;
    }
    default: {
      break;
    }
  }
  initialized=true;
}

void RTHPContainer::write(unsigned short a,unsigned short v) {
  // get reg wirte
  if (!initialized) {
    logE("RTHP: not initialized!");
    return;
  }
  String dump="addr: ";
  dump+=a;
  dump+=", val: ";
  dump+=v;
  switch (impl) {
    case RTHP_ERTHP: {
      erthp.sendSerial(dump);
    }
    default: break;
  }
}