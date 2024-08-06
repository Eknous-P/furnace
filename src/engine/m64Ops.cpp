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

  unsigned char channels=getSystemDef(song.system[0])->channels;
  unsigned short* chanData=new unsigned short[channels]; // list of channel data pointers
  memset(chanData,0,2*channels);
  unsigned short chanDataBegin; // pointer to channel data pointers

  /* # note to self: .m64 is BIG ENDIAN */

  /* .m64 "header"? (sequence data ~ global setup) */

  // maybe make these a param?
  w->write("\xd3\x20",2); // mute behavior -> lower volume
  w->write("\xd5\x3f",2); // mute mode multiplier -> 3F

  unsigned short chanMask = (1<<channels)-1;
  w->writeC(0xd7);
  w->writeS_BE(chanMask); // init channels

  w->writeC(0xdb);
  w->writeC(volumeScale); // set vol scale

  w->writeC(0xdd);
  w->writeC((unsigned char)(curSubSong->hz*2.5)); // set song tempo
  chanDataBegin = w->tell();
  // write temporary channel data pointers
  for (unsigned char i=0; i<channels; i++) {
    w->writeC(0x90+i);
    w->writeS_BE(0); // pad with 0s
  }

  /* channel data ~ orders*/

  for (unsigned char i=0; i<channels; i++) {
    chanData[i] = w->tell()+1;
  }

  /* layer/track data ~ patterns*/


  /* rewrite channel data pointers */
  // go back
  w->seek(chanDataBegin,SEEK_SET);
  // write correct pointers
  for (unsigned char i=0; i<channels; i++) {
    w->writeC(0x90+i);
    w->writeS_BE(chanData[i]);
  }

  w->writeC(0xff); // end sequence

  delete[] chanData;
  chanData=NULL;
  
  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;
  BUSY_END;

  return w;
}