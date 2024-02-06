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

// implementations
#include "impl/e-rthp.h"

const char* RTHPImplementationNames[]={
  "*NONE*",
  "E-RTHP"
};

ERTHP erthp;

int initERTHP(String port) {
  try {
    if (erthp.initSerial(port,57600,10)) {
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

bool writeERTHP(RTHPWrite w) {
  bool serialerr=false;
  if (erthp.sendSerial(w.key)==-1) serialerr=true;
  if (erthp.sendSerial(w.data)==-1) serialerr=true;
  if (erthp.sendSerial(w.addrlow)==-1) serialerr=true;
  if (erthp.sendSerial(w.addrhigh)==-1) serialerr=true;
  // just to be sure each "packet" is exactly 4 bytes (>DAA)
  return serialerr;
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

void RTHPContainer::setImpl(RTHPImplementations impl) {
  container.impl=impl;
}

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

void RTHPContainer::write(unsigned short a, unsigned short v) {
  if (container.writing) return;
  container.writing=true;
  if (!container.initialized) return;
  switch (container.impl) {
    case RTHP_ERTHP: {
      RTHPWrite lastWrite;
      lastWrite.key='>';
      lastWrite.data=(v&0xff);
      lastWrite.addrlow=(a&0xff);
      lastWrite.addrhigh=((a>>8)&0xff);
      if (container.lastWrites.size()>256) container.lastWrites.erase(container.lastWrites.begin());
      container.lastWrites.push_back(lastWrite);
      if (writeERTHP(lastWrite)) {
        logE("RTHP: %s",erthp.getLastLog());
        RTHPContainer::deinit();
      }
      break;
    }
    default: break;
  }
  container.writing=false;
}

String RTHPContainer::read() {
  switch (container.impl) {
    case RTHP_ERTHP: {
      String buf=erthp.receiveSerial(1);
      container.readBuffer+=buf;
      return buf;
    }
    default: return "";
  }
}

String RTHPContainer::getReadBuffer() {
  return container.readBuffer;
}
void RTHPContainer::clearReadBuffer() {
  container.readBuffer="";
}

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

std::vector<RTHPWrite> RTHPContainer::getLastWrites() {
  return container.lastWrites;
}

void RTHPContainer::clearLastWrites() {
  container.lastWrites.clear();
}
