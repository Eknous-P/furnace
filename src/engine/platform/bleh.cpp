/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2025 tildearrow and contributors
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

#include "fmshared_OPM.h"
#define _USE_MATH_DEFINES
#include "bleh.h"
#include "../engine.h"
#include "../../ta-log.h"

#define CHIP_FREQBASE 0x10000
#define CHIP_DIVIDER 1

const char* regCheatSheetBleh[]={
  "Channel 1 freq high", "0",
  "Channel 1 freq low", "1",
  "Channel 2 freq high", "2",
  "Channel 2 freq low", "3",
  "Channel 1 state", "4",
  "Channel 2 state", "5",
  "Channel 1 noise freq", "6",
  "Channel 2 noise freq", "7",
  NULL
};

const char** DivPlatformBleh::getRegisterSheet() {
  return regCheatSheetBleh;
}

void DivPlatformBleh::acquire(short** buf, size_t len) {
  for (int i=0; i<2; i++) {
    oscBuf[i]->begin(len);
  }

  for (size_t h=0; h<len; h++) {
    bleh.tick();
    oscBuf[0]->putSample(h,bleh.getChanOutput(0)<<8);
    oscBuf[1]->putSample(h,bleh.getChanOutput(1)<<8);
    buf[0][h]=bleh.getOutput()<<7;
  }

  for (int i=0; i<2; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformBleh::acquireDirect(blip_buffer_t** bb, size_t len) {
}

void DivPlatformBleh::tick(bool sysTick) {
  for (int i=0; i<2; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=VOL_SCALE_LOG(chan[i].vol,MIN(8,chan[i].std.vol.val),8);
    }
    if (chan[i].std.duty.had) {
      chan[i].noise=chan[i].std.duty.val&2;
      chan[i].square=chan[i].std.duty.val&1;
      chan[i].freqChanged=true;
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_PERIODIC(parent->calcArp(chan[i].note,chan[i].std.arp.val));
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].std.pitch.had) {
      if (chan[i].std.pitch.mode) {
        chan[i].pitch2+=chan[i].std.pitch.val;
        CLAMP_VAR(chan[i].pitch2,-32768,32767);
      } else {
        chan[i].pitch2=chan[i].std.pitch.val;
      }
      chan[i].freqChanged=true;
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,true,0,chan[i].pitch2,chipClock,CHIP_DIVIDER)-1;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>1023) chan[i].freq=1023;

      if (chan[i].keyOn) chan[i].keyOn=false;
      if (chan[i].keyOff) chan[i].keyOff=false;
      chan[i].freqChanged=false;
    }
  }
}

int DivPlatformBleh::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].baseFreq=NOTE_PERIODIC(c.value);
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_BLEH));
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      chan[c.chan].macroInit(NULL);
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      break;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
      }
      break;
    case DIV_CMD_VOLUME:
      if (chan[c.chan].vol!=c.value) {
        chan[c.chan].vol=c.value;
        if (!chan[c.chan].std.vol.has) {
          chan[c.chan].outVol=c.value;
        }
        if (chan[c.chan].active) {
          on=chan[c.chan].vol;
        }
      }
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    case DIV_CMD_PITCH:
      chan[c.chan].pitch=c.value;
      chan[c.chan].freqChanged=true;
      break;
    case DIV_CMD_NOTE_PORTA: {
      int destFreq=NOTE_PERIODIC(c.value2);
      bool return2=false;
      if (destFreq>chan[c.chan].baseFreq) {
        chan[c.chan].baseFreq+=c.value;
        if (chan[c.chan].baseFreq>=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      } else {
        chan[c.chan].baseFreq-=c.value;
        if (chan[c.chan].baseFreq<=destFreq) {
          chan[c.chan].baseFreq=destFreq;
          return2=true;
        }
      }
      chan[c.chan].freqChanged=true;
      if (return2) {
        chan[c.chan].inPorta=false;
        return 2;
      }
      break;
    }
    case DIV_CMD_LEGATO:
      if (c.chan==3) break;
      chan[c.chan].baseFreq=NOTE_PERIODIC(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_BLEH));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_PERIODIC(chan[c.chan].note);
      chan[c.chan].inPorta=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 7;
      break;
    case DIV_CMD_MACRO_OFF:
      chan[c.chan].std.mask(c.value,true);
      break;
    case DIV_CMD_MACRO_ON:
      chan[c.chan].std.mask(c.value,false);
      break;
    case DIV_CMD_MACRO_RESTART:
      chan[c.chan].std.restart(c.value);
      break;
    case DIV_CMD_BLEH_WAVE: {
      chan[c.chan].waveNum=c.value&3;
      unsigned char stateCopy=chan[c.chan].state&=0b11100111;
      stateCopy|=chan[c.chan].waveNum<<3;
      chan[c.chan].state=stateCopy;
      bleh.write(BLEH_ADDR_CHAN1STATE+c.chan, stateCopy);
      break;
    }
    case DIV_CMD_BLEH_NOISEFREQ:
      chan[c.chan].noiseFreq=c.value;
      bleh.write(BLEH_ADDR_CHAN1NOISEFREQ+c.chan, c.value);
      break;
    case DIV_CMD_BLEH_CONTROL: {
      chan[c.chan].control=c.value&7;
      unsigned char stateCopy=chan[c.chan].state&=0b00011111;
      stateCopy|=chan[c.chan].control<<5;
      chan[c.chan].state=stateCopy;
      bleh.write(BLEH_ADDR_CHAN1STATE+c.chan, stateCopy);
      break;
    }
    default:
      break;
  }
  return 1;
}

void DivPlatformBleh::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformBleh::forceIns() {
  for (int i=0; i<1; i++) {
    chan[i].insChanged=true;
  }
}

void* DivPlatformBleh::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformBleh::getChanMacroInt(int ch) {
  return &chan[ch].std;
}

DivDispatchOscBuffer* DivPlatformBleh::getOscBuffer(int ch) {
  return oscBuf[ch];
}

unsigned char* DivPlatformBleh::getRegisterPool() {
  return (unsigned char*)(((unsigned char*)&bleh)+sizeof(void*));
}

int DivPlatformBleh::getRegisterPoolSize() {
  return 8;
}

void DivPlatformBleh::reset() {
  for (int i=0; i<1; i++) {
    chan[i]=DivPlatformBleh::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }


  memset(regPool,0,8);
}

bool DivPlatformBleh::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformBleh::hasAcquireDirect() {
  return false;
}

void DivPlatformBleh::setFlags(const DivConfig& flags) {
}

void DivPlatformBleh::notifyInsDeletion(void* ins) {
  for (int i=0; i<2; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformBleh::notifyPlaybackStop() {
  bleh.write(4,0);
  bleh.write(5,0);
}

void DivPlatformBleh::poke(unsigned int addr, unsigned short val) {
  bleh.write(addr, val);
}

void DivPlatformBleh::poke(std::vector<DivRegWrite>& wlist) {
  for (DivRegWrite i:wlist) {
    bleh.write(i.addr,i.val);
  }
}

int DivPlatformBleh::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  dumpWrites=false;
  skipRegisterWrites=false;
  for (int i=0; i<2; i++) {
    isMuted[i]=false;
    oscBuf[i]=new DivDispatchOscBuffer;
  }
  setFlags(flags);

  bleh.setROM((unsigned char*)blehROM);
  bleh.reset();
  return 5;
}

void DivPlatformBleh::quit() {
  for (int i=0; i<2; i++) {
    delete oscBuf[i];
  }
}

DivPlatformBleh::~DivPlatformBleh() {
}
