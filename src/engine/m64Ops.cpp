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

/* # note: .m64 is BIG ENDIAN */

#include "engine.h"
#include "../fileutils.h"
#include "../ta-log.h"

enum m64Commands {
  // shared commands
  CMD_END=0xff,
  CMD_LOOPEND=0xf7,
  CMD_LOOP=0xf8,
  CMD_BLTZ=0xf9,
  CMD_BEQZ=0xfa,
  CMD_JUMP=0xfb,
  CMD_CALL=0xfc,
  CMD_DELAY=0xfd, // note used in layer
  CMD_DELAY1=0xfe, // note used in layer
  // sequence
  CMD_SEQ_TESTCHDISABLED=0x00,
  CMD_SEQ_SUBVARIATION=0x50,
  CMD_SEQ_SETVARIATION=0x70,
  CMD_SEQ_GETVARIATION=0x80,
  CMD_SEQ_STARTCHANNEL=0x90,
  CMD_SEQ_SUBTRACT=0xc8,
  CMD_SEQ_BITAND=0xc9,
  CMD_SEQ_SETVAL=0xcc,
  CMD_SEQ_SETNOTEALLOCATIONPOLICY=0xd0,
  CMD_SEQ_SETSHORTNOTEDURATIONTABLE=0xd1,
  CMD_SEQ_SETSHORTNOTEVELOCITYTABLE=0xd2,
  CMD_SEQ_SETMUTEBHV=0xd3,
  CMD_SEQ_MUTE=0xd4,
  CMD_SEQ_SETMUTESCALE=0xd5,
  CMD_SEQ_DISABLECHANNELS=0xd6,
  CMD_SEQ_INITCHANNELS=0xd7,
  CMD_SEQ_CHANGEVOL=0xda,
  CMD_SEQ_SETVOL=0xdb,
  CMD_SEQ_ADDTEMPO=0xdc,
  CMD_SEQ_SETTEMPO=0xdd,
  CMD_SEQ_TRANSPOSEREL=0xde,
  CMD_SEQ_TRANSPOSE=0xdf,
  CMD_SEQ_UNRESERVENOTES=0xf1,
  CMD_SEQ_RESERVENOTES=0xf2,
  CMD_SEQ_BGEZ=0xf5,
  // channel
  CMD_CHANNEL_TESTLAYERFINISHED=0x00,
  CMD_CHANNEL_STARTCHANNEL=0x10,
  CMD_CHANNEL_DISABLECHANNEL=0x20,
  CMD_CHANNEL_IOWRITEVAL2=0x30,
  CMD_CHANNEL_IOREADVAL2=0x40,
  CMD_CHANNEL_IOREADVALSUB=0x50,
  CMD_CHANNEL_SETNOTEPRIORITY=0x60,
  CMD_CHANNEL_IOWRITEVAL=0x70,
  CMD_CHANNEL_IOREADLAV=0x80,
  CMD_CHANNEL_SETLAYER=0x90,
  CMD_CHANNEL_FREELAYER=0xa0,
  CMD_CHANNEL_DYNSETLAYER=0xb0,
  CMD_CHANNEL_SETINSTR=0xc1,
  CMD_CHANNEL_SETDYNTABLE=0xc2,
  CMD_CHANNEL_LARGENOTESOFF=0xc3,
  CMD_CHANNEL_LARGENOTESON=0xc4,
  CMD_CHANNEL_DYNSETDYNTABLE=0xc5,
  CMD_CHANNEL_SETBANK=0xc6,
  CMD_CHANNEL_WRITESEQ=0xc7,
  CMD_CHANNEL_SUBTRACT=0xc8,
  CMD_CHANNEL_BITAND=0xc9,
  CMD_CHANNEL_SETMUTEBHV=0xca,
  CMD_CHANNEL_READSEQ=0xcb,
  CMD_CHANNEL_SETVAL=0xcc,
  CMD_CHANNEL_STEREOHEADSETEFFECTS=0xd0,
  CMD_CHANNEL_SETNOTEALLOCATIONPOLICY=0xd1,
  CMD_CHANNEL_SETSUSTAIN=0xd2,
  CMD_CHANNEL_PITCHBEND=0xd3,
  CMD_CHANNEL_SETUPDATESPERFRAME=0xd6,
  CMD_CHANNEL_SETVIBRATORATE=0xd7,
  CMD_CHANNEL_SETVIBRATOEXTENT=0xd8,
  CMD_CHANNEL_SETDECAYRELEASE=0xd9,
  CMD_CHANNEL_SETENVELOPE=0xda,
  CMD_CHANNEL_TRANSPOSE=0xdb,
  CMD_CHANNEL_SETPANCHANWEIGHT=0xdc,
  CMD_CHANNEL_SETPAN=0xdd,
  CMD_CHANNEL_DREQSCALE=0xde,
  CMD_CHANNEL_SETVOL=0xdf,
  CMD_CHANNEL_SETVOLSCALE=0xe0,
  CMD_CHANNEL_SETVIBRATORATELINEAR=0xe1,
  CMD_CHANNEL_SETVIBRATOEXTENTLINEAR=0xe2,
  CMD_CHANNEL_SETVIBRATODELAY=0xe3,
  CMD_CHANNEL_DYNCALL=0xe4,
  CMD_CHANNEL_UNRESERVENOTES=0xf1,
  CMD_CHANNEL_RESERVENOTES=0xf2,
  CMD_CHANNEL_HANG=0xf3,
  CMD_CHANNEL_BGEZ=0xf5,
  CMD_CHANNEL_BREAK=0xf6,
  // layer
  CMD_LAYER_NOTE0=0x00,
  CMD_LAYER_SMALLNOTE0=0x00,
  CMD_LAYER_NOTE1=0x40,
  CMD_LAYER_SMALLNOTE1=0x40,
  CMD_LAYER_NOTE2=0x80,
  CMD_LAYER_SMALLNOTE2=0x80,
  CMD_LAYER_DELAY=0xc0,
  CMD_LAYER_SETSHORTNOTEVELOCITY=0xc1,
  CMD_LAYER_TRANSPOSE=0xc2,
  CMD_LAYER_SETSHORTNOTEDEFAULTPLAYPERCENTAGE=0xc3,
  CMD_LAYER_SOMETHINGON=0xc4,
  CMD_LAYER_SOMETHINGOFF=0xc5,
  CMD_LAYER_SETINSTR=0xc6,
  CMD_LAYER_PORTAMENTO=0xc7,
  CMD_LAYER_DISABLEPORTAMENTO=0xc8,
  CMD_LAYER_SETSHORTNOTEDURATION=0xc9,
  CMD_LAYER_SETPAN=0xca,
  CMD_LAYER_SETSHORTNOTEVELOCITYFROMTABLE=0xd0,
  CMD_LAYER_SETSHORTNOTEDURATIONFROMTABLE=0xe0
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

// 'var' is a special variable(?) which is either one or two bytes long
// if the last bit of the first byte is set, then its 2 bytes (unsigned short)
// otherwise, its 1 byte (unsigned char)
void writeVar(SafeWriter* w, unsigned short v) {
  if (v > 0x7f) {
    w->writeS_BE(v | (1<<15));
  } else {
    w->writeC(v);
  }
}

SafeWriter* DivEngine::saveM64(unsigned char muteBhv, unsigned char volumeScale, unsigned char muteVolScale) {
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

  /* .m64 "header"? (sequence data ~ global setup) */

  w->writeC(CMD_SEQ_SETMUTEBHV); // mute behavior
  w->writeC(muteBhv);
  w->writeC(CMD_SEQ_SETMUTESCALE); // mute volume scale
  w->writeC(muteVolScale);

  unsigned short chanMask = (1<<channels)-1;
  w->writeC(CMD_SEQ_INITCHANNELS);
  w->writeS_BE(chanMask); // init channels

  w->writeC(CMD_SEQ_SETVOL);
  w->writeC(volumeScale); // set vol scale

  chanDataBegin = w->tell();
  // write temporary channel data pointers
  for (unsigned char i=0; i<channels; i++) {
    w->writeC(CMD_SEQ_STARTCHANNEL+i);
    w->writeS_BE(0); // pad with 0s
  }

  w->writeC(CMD_SEQ_SETTEMPO);
  w->writeC((unsigned char)calcBPM()); // set song tempo

  w->writeC(CMD_DELAY); // delay
  writeVar(w,0x7fff);

  w->writeC(CMD_SEQ_DISABLECHANNELS); // deinit channels
  w->writeS_BE(chanMask);

  w->writeC(CMD_END); // end sequence

  /* channel data ~ orders */

  for (unsigned char i=0; i<channels; i++) {
    chanData[i]=w->tell();
    
    unsigned char layerCount=getUniquePatternCount(this,i);
    unsigned short* layerData=new unsigned short[layerCount];
    unsigned short* layerDataBegin=new unsigned short[layerCount];

    // channel data stuff here

    w->writeC(CMD_CHANNEL_LARGENOTESON); // enable "large notes"
    // w->writeC(CMD_CHANNEL_LARGENOTESOFF); // disable "large notes"
    w->writeC(CMD_CHANNEL_SETVOL);
    w->writeC(0x7f); // set chan volume
  
    w->writeC(CMD_CHANNEL_SETINSTR);
    w->writeC(0x00); // ins


    // write temporary layer data pointers

    for (unsigned char j=0; j<layerCount; j++) {
      layerDataBegin[j]=w->tell();
      w->writeC(CMD_CHANNEL_SETLAYER+j);
      w->writeS_BE(0);
    }

    w->writeC(CMD_CHANNEL_SETPAN);
    w->writeC(0x3f); // panning

    w->writeC(CMD_CHANNEL_SETVOL);
    w->writeC(0x7f); // volume 2

    w->writeC(CMD_CHANNEL_SETNOTEPRIORITY); //priority

    w->writeC(CMD_END);

    for (unsigned char j=0; j<layerCount; j++) {
      /* layer/track data ~ patterns */
      layerData[j]=w->tell();

      // w->writeC(0xc0); 
      // w->writeC(0x1); // temporary 1 tick delay

      // DivPattern* currPat=curSubSong->pat[i].getPattern((curOrders->ord[i][j]),false);
      // PatternRowCondensed r;

      // for (int k=0;k<curSubSong->patLen;k++) {
      //   r.getPatRow(currPat->data[k]);
      //   if (r.nonEmpty|2) {
      //     w->writeC(0xc6);
      //     w->writeC(r.instrument); // set ins
      //   }

      //   if (r.nonEmpty|4) {
      //     w->writeC(0xc1);
      //     w->writeC(r.volume>>1); // set volume
      //   }

      //   if (r.nonEmpty|1) {
      //     w->writeC(0x27); //note
      //     w->writeC(0x80);
      //     w->writeC(0xc0); // play percentage... whatever that is
      //   }

      // }
      w->writeC(CMD_LAYER_TRANSPOSE);
      w->writeC(0x00); // transpose

      w->writeC(CMD_LAYER_NOTE1+0x27); // note
      writeVar(w,0x7fff); // play percentage
      w->writeC(0x7f); // vel

      w->writeC(CMD_LAYER_DELAY); // delay
      w->writeC(0x7f);

      w->writeC(CMD_END);
    }



    /* rewrite layer data pointers */

    for (unsigned char j=0; j<channels; j++) {
      w->seek(layerDataBegin[j],SEEK_SET);
      w->writeC(CMD_CHANNEL_SETLAYER+j);
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
    w->writeC(CMD_SEQ_STARTCHANNEL+i);
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