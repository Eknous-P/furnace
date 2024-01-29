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

#ifndef _RTHP_H
#define _RTHP_H

#include "../ta-log.h"
#include "../ta-utils.h"

enum RTHPImplementations {
  RTHP_NONE=0,
  RTHP_ERTHP
};

extern const char* RTHPImplementationNames[];

class RTHPContainer {
  public:
    struct container {
      bool initialized;
      RTHPImplementations impl;
      String port;
      int chipToDump;
      String readBuffer;
      String lastWrite;
      container():
        initialized(false),
        impl(RTHP_NONE),
        port(""),
        chipToDump(0),
        readBuffer(""),
        lastWrite("") {}
    } container;

    void setImpl(RTHPImplementations impl);
    int init(RTHPImplementations setImpl, String setPort);
    void scanAvailPorts();
    std::vector<String> getAvailPortNames();
    auto getAvailPorts();
    void writePlain(String s);
    void write(unsigned short a, unsigned short v);
    String read();
    String getReadBuffer();
    void clearReadBuffer();
    int deinit();
    bool getRTHPState();
    void setDumpedChip(int chip);
    int getDumpedChip();
    String getLastWrite();
};

// implementation-specific helper functions

int initERTHP(std::string port);

#endif
