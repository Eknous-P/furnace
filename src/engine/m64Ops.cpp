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

#include "engine.h"
#include "../fileutils.h"
#include "../ta-log.h"

SafeWriter* DivEngine::saveM64(unsigned char volumeScale) {
  stop();
  repeatPattern=false;
  shallStop=false;
  setOrder(0);
  BUSY_BEGIN_SOFT;
  // determine loop point
  int loopOrder=0;
  int loopRow=0;
  int loopEnd=0;
  walkSong(loopOrder,loopRow,loopEnd);
  logI("loop point: %d %d",loopOrder,loopRow);

  SafeWriter* w=new SafeWriter;
  w->init();

  // note to self: .m64 is BIG ENDIAN

  w->write("\xd3\x20",2); // mute behavior -> lower volume
  w->write("\xd5\x3f",2); // mute mode multiplier -> 3F

  unsigned short chanMask = (1<<getSystemDef(song.system[0])->channels)-1;
  w->write("\xd7",1);
  w->writeS_BE(chanMask); // init channels

  w->write("\xdb",1);
  w->writeC(volumeScale); // set vol scale

  w->write("\xdd",1);
  w->writeC((unsigned char)(curSubSong->hz*2.5)); // set song tempo

  w->write("\xff",1); // end sequence
  
  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;
  BUSY_END;

  return w;
}