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

// E-RTHP VERSION 0
// REAL-TIME HARDWARE PLAYBACK HOST IMPLEMENTATION

#include <string>
#include <vector>
#include "serial/serial.h"

class ERTHP: public RTHP {
  struct Port {
    std::vector<serial::PortInfo> availPorts;
    unsigned long int baudrate, timeout;
    std::string portName;

    Port():
      baudrate(0),
      timeout(0),
      portName("") {}
  } port;
  
}
