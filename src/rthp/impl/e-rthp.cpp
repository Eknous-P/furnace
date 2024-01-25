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

#include "e-rthp.h"

void ERTHP::clearLog() {
  for (unsigned char i=0; i<64; i++) {
    erthp_log.logBuffer[i]="";
  }
}

void ERTHP::writeLog(std::string logm) {
  erthp_log.lastLog=logm;
  erthp_log.lastLogNum++;
  erthp_log.lastLogNum%=64;
  erthp_log.logBuffer[erthp_log.lastLogNum]=erthp_log.lastLog;
}

std::string ERTHP::getLastLog() {
  return erthp_log.lastLog;
}

int ERTHP::scanAvailPorts() {
  erthp_serial.availPorts=serial::list_ports();
  return (erthp_serial.availPorts.end() - erthp_serial.availPorts.begin());
}

std::vector<serial::PortInfo> ERTHP::getAvailPorts() {
  return erthp_serial.availPorts;
}

std::vector<std::string> ERTHP::getAvailPortNames() {
  std::vector<std::string> ret{};
  for (serial::PortInfo i:erthp_serial.availPorts) ret.push_back(i.port);
  return ret;
}

int ERTHP::initSerial(std::string port, unsigned int baudrate, unsigned int timeout) {
  erthp_serial.portName=port;
  erthp_serial.serialBaudrate=baudrate;
  erthp_serial.serialTimeout=timeout;
  try {
    serialPort.setPort(erthp_serial.portName.c_str());
    serialPort.setBaudrate(erthp_serial.serialBaudrate);
    serialPort.setTimeout(erthp_serial.serialTimeout,erthp_serial.serialTimeout,0,erthp_serial.serialTimeout,0);
  } catch (std::exception& xc) {
    ERTHP::writeLog(xc.what());
    return 1;
  }
  serialPort.open();

  if (!serialPort.isOpen()) {
    ERTHP::writeLog("ERTHP: could not open serial port!");
    return 1;
  }
  serialPort.write("RTHP");
  return 0;
}

int ERTHP::sendSerial(std::string msg) {
  try {
    return (int)serialPort.write(msg);
  } catch (std::exception& xc) {
    ERTHP::writeLog(xc.what());
    return -1;
  }
}

std::string ERTHP::receiveSerial() {
  try {
    return serialPort.read();
  } catch (std::exception& xc) {
    ERTHP::writeLog(xc.what());
    return "";
  }
}

void ERTHP::closeSerial() {
  return serialPort.close();
}
