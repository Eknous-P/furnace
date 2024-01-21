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

int initERTHP(String port) {
  try {
    if (erthp.initSerial(port,1000000,1000)) {
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

void RTHPContainer::scanAvailPorts() {
  switch (container.impl) {
    case RTHP_ERTHP: {  
      logI("RTHP: serial ports found: %d", erthp.scanAvailPorts());
      return;
    }
    default: return;
  }
}

std::vector<String> RTHPContainer::getAvailPortNames() {
  switch (container.impl) {
    case RTHP_ERTHP: {
      return erthp.getAvailPortNames();
    }
    default: return {""};
  }
}

void RTHPContainer::setImpl(RTHPImplementations impl) {container.impl=impl;}

int RTHPContainer::init(RTHPImplementations setImpl, String setPort) {
  container.port=setPort;
  container.initialized=false;
  logI("RTHP: begin init");
  logI("RTHP: using impl %s", RTHPImplementationNames[setImpl]);
  switch (container.impl) {
    case RTHP_ERTHP: {
      if(initERTHP(container.port)) return 1;
      break;
    }
    default: return 2;
  }
  container.initialized=true;
  return 0;
}

void RTHPContainer::write(unsigned short a,unsigned short v) {
  // get reg wirte
  if (!container.initialized) {
    // logE("RTHP: not initialized!");
    return;
  }
  String dump=">";
  dump+=a;
  dump+=v;
  switch (container.impl) {
    case RTHP_ERTHP: {
      if (erthp.sendSerial(dump)==-1) {
        logE("RTHP: %s",erthp.getLastLog());
        RTHPContainer::deinit();
      }
      break;
    }
    default: break;
  }
}

void RTHPContainer::read() {
  switch (container.impl) {
    case RTHP_ERTHP: {
      container.readBuffer+=erthp.receiveSerial();
      return;
    }
    default: return;
  }
}

String RTHPContainer::getReadBuffer() {return container.readBuffer;}
void RTHPContainer::clearReadBuffer() {container.readBuffer="";}

int RTHPContainer::deinit() {
  if (!container.initialized) return 0;
  switch (container.impl) {
    case RTHP_ERTHP: {
      erthp.closeSerial();
      container.initialized=false;
      break;
    }
    default: break;
  }
  logI("RTHP: successfully deinited port %s", container.port);
  return 0;
}

bool RTHPContainer::getRTHPState() {
  return container.initialized;
}

void RTHPContainer::setDumpedChip(int chip) {
  container.chipToDump=chip;
}

int RTHPContainer::getDumpedChip() {
  return container.chipToDump;
}
