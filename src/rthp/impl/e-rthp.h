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

class ERTHP {
  serial::Serial serialPort;
  struct ERTHP_Serial {
    std::vector<serial::PortInfo> availPorts;

    unsigned long int serialBaudrate, serialTimeout;
    std::string portName;

    ERTHP_Serial():
      serialBaudrate(1000000),
      serialTimeout(1000),
      portName("") {}
  } erthp_serial;
  struct ERTHP_Log {
    std::string logBuffer[64]; // cyclical buffer, not intended as a main log buffer
    std::string lastLog;
    unsigned char lastLogNum;

    ERTHP_Log():
      lastLog(""),
      lastLogNum(0) {}
  } erthp_log;

  public:
    // serial
    int scanAvailPorts();
    std::vector<serial::PortInfo> getAvailPorts();
    std::vector<std::string> getAvailPortNames();

    int initSerial(std::string port, unsigned int baudrate, unsigned int timeout);
    int sendSerial(unsigned char chr);
    std::string receiveSerial(size_t s);
    void closeSerial();

    // logging
    void clearLog();
    void writeLog(std::string log);
    std::string getLastLog();
};
