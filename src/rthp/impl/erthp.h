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
#include "serial/serial.h"

#ifndef ERTHP_H
#define ERTHP_H

class ERTHP: public RTHPImpl {
  private:
    serial::Serial port;
    std::vector<serial::PortInfo> ports;

    std::vector<RTHPDevice> devs;
    unsigned int chip;
    unsigned int rate;
    unsigned int timeout;
    int currectDev;
    bool running;
  public:
    RTHPImplInfo getInfo();
    int listDevices();
    std::vector<RTHPDevice> getDeviceList();
    int init(int dev, unsigned int rate, unsigned int tout);
    bool isRunning();
    void setChip(int _chip);
    int sendPacket(RTHPPacketLegacy p);
    int sendPacket(RTHPPacketShort p);
    int sendPacket(RTHPPacketInfo p);
    int sendRaw(char* data, size_t len);
    int receive(char* buf, uint8_t len);
    uint8_t receive();
    int deinit();
    ERTHP();
    ~ERTHP();
};

#endif