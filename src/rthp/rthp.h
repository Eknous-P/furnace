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

#ifndef _RTHP_H
#define _RTHP_H

#include "../ta-log.h"
#include "../ta-utils.h"

enum RTHPImplementations {
  RTHP_NONE=0,

  RTHP_ERTHP
};

struct RTHPPacketShort {
  // 4 bytes long
  unsigned char key;
  unsigned char data;
  unsigned char addrlow;
  unsigned char addrhigh;
};

struct RTHPPacketLong {
  // 8 bytes long
  unsigned char key;
  unsigned short data;
  unsigned short addr;
  unsigned char res1,res,res3; // reserved. i currently dont have plans for these bytes
};

class RTHP {
  protected: // (totally not dispatch.h)
    /**
     * the "device id" (a very vague way of saying "port" because
     * some impls may not operate based on ports) of the impl.
     */
    int deviceId;
    std::vector<String> deviceNames;

  public:
    /**
     * get the impl description (can be multiline).
     * @return the description.
     */
    virtual String getImplDescription();

    /**
     * get the OS compatibility.
     * @return a bool. true if compatible.
     */
    virtual bool getOSCompat();

    /**
     * set the devide id.
     * @param id the id to set to.
     */
    virtual void setDeviceId(int id);

    /**
     * get the devide id.
     * @return the id.
     */
    virtual int getDeviceId();

    /**
     * get the device name.
     * @return the name as a string.
     */
    virtual String getDeviceName();

    /**
     * initialize the implementation.
     */
    virtual void init();
};

#endif
