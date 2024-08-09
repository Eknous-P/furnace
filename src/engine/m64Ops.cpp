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

struct PatternRowCondensed {
  unsigned char note;
  unsigned char instrument;
  unsigned char volume;

  unsigned char nonEmpty;
  void getPatRow(short* p) {
    nonEmpty=0;
    note=(p[0]%12)+12*p[1];
    instrument=p[2];
    volume=p[3];
    nonEmpty|=((p[0]==-1)?1:0<<0)&1;
    nonEmpty|=((p[2]==-1)?1:0<<1)&1;
    nonEmpty|=((p[3]==-1)?1:0<<2)&1;
  }
};

int getUniquePatternCount(DivEngine* e, int chan) {
  int count=0;
  int chanOrdCount=e->curSubSong->ordersLen;
  std::vector<unsigned char> ords;
  for (int i=0; i<chanOrdCount; i++) {
    bool have=false;
    //find if already found
    for (unsigned char j:ords) {
      if (e->curOrders->ord[chan][i] == j) {
        have=true;
        break;
      }
    }
    if (!have) {
      ords.push_back(i);
      count++;
    }
      
  }
  ords.clear();
  return count;
}

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

  w->writeC(0xff); // end sequence

  /* channel data ~ orders */

  for (unsigned char i=0; i<channels; i++) {
    chanData[i]=w->tell();
    
    unsigned char layerCount=getUniquePatternCount(this,i);
    unsigned short* layerData=new unsigned short[layerCount];
    unsigned short* layerDataBegin=new unsigned short[layerCount];

    // channel data stuff here

    // w->writeC(0xc4); // enable "large notes"
    w->writeC(0xc3); // disable "large notes"
    w->writeC(0xdf);
    w->writeC(0x7f); // set chan volume


    // write temporary layer data pointers

    for (unsigned char j=0; j<layerCount; j++) {
      layerDataBegin[j]=w->tell();
      w->writeC(0x90+j);
      w->writeS_BE(0);
    }

    w->writeC(0xff);

    for (unsigned char j=0; j<layerCount; j++) {
      /* layer/track data ~ patterns */
      layerData[j]=w->tell();

      // w->writeC(0xc0); 
      // w->writeC(0x1); // temporary 1 tick delay

      DivPattern* currPat=curSubSong->pat[i].getPattern((curOrders->ord[i][j]),false);
      PatternRowCondensed r;

      for (int k=0;k<curSubSong->patLen;k++) {
        r.getPatRow(currPat->data[k]);
        if (r.nonEmpty|2) {
          w->writeC(0xc6);
          w->writeC(r.instrument); // set ins
        }

        if (r.nonEmpty|4) {
          w->writeC(0xc1);
          w->writeC(r.volume>>1); // set volume
        }

        if (r.nonEmpty|1) {
          w->writeC(0x27); //note
          w->writeC(0x80);
          w->writeC(0xc0); // play percentage... whatever that is
        }

      }

      w->writeC(0xff);
    }



    /* rewrite layer data pointers */

    for (unsigned char j=0; j<channels; j++) {
      w->seek(layerDataBegin[j],SEEK_SET);
      w->writeC(0x90+j);
      w->writeS_BE(layerData[j]);
    }

    delete[] layerData;
    delete[] layerDataBegin;
    layerData=NULL;
  }

  


  /* rewrite channel data pointers */
  // go back
  w->seek(chanDataBegin,SEEK_SET);
  // write correct pointers
  for (unsigned char i=0; i<channels; i++) {
    w->writeC(0x90+i);
    w->writeS_BE(chanData[i]);
  }

  delete[] chanData;
  chanData=NULL;
  
  remainingLoops=-1;
  playing=false;
  freelance=false;
  extValuePresent=false;
  BUSY_END;

  return w;
}