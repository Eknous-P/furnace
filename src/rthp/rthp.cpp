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

// implementations
#include "impl/e-rthp/e-rthp.h"

const char* RTHPImplementationNames[]={
  "*NONE*",
  "E-RTHP"
};

ERTHP erthp;

int initERTHP(std::string port) {
  logI("RTHP: serial ports found: %d", erthp.scanAvailPorts());
  try {
    if (erthp.initSerial(port,9600,1000)) { // temporary port
      logE(erthp.getLastLog().c_str());
      return 1;
    }
  } catch (std::exception& xc) {
    logE("RTHP: falied to connect to %s",port);
    logE("RTHP exception: %s",xc.what());
    return 1;
  }
  logI("RTHP: successfully connected to %s",port);
  return 0;
}

void RTHPContainer::init(RTHPImplementation setImpl) {
  container.impl=setImpl;
  container.initialized=false;
  logI("RTHP: begin init");
  logI("RTHP: using impl %s", RTHPImplementationNames[setImpl]);
  switch (container.impl) {
    case RTHP_ERTHP: {
      if(initERTHP("/dev/ttyUSB0")) return;
      break;
    }
    default: {
      break;
    }
  }
  container.initialized=true;
}

void RTHPContainer::write(unsigned short a,unsigned short v) {
  // get reg wirte
  if (!container.initialized) {
    // logE("RTHP: not initialized!");
    return;
  }
  String dump="addr: ";
  dump+=a;
  dump+=", val: ";
  dump+=v;
  switch (container.impl) {
    case RTHP_ERTHP: {
      erthp.sendSerial(dump);
      break;
    }
    default: break;
  }
}