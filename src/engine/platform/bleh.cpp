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

#define _USE_MATH_DEFINES
#include "bleh.h"
#include "../engine.h"
#include "../../ta-log.h"

#define rWrite(a,v) if (!skipRegisterWrites) {writes.push(QueuedWrite(a,v)); if (dumpWrites) {addWrite(a,v);} }

#define CHIP_FREQBASE 512
#define CHIP_DIVIDER 1024

#define NOTE_IDK(x) NOTE_FREQUENCY(x)

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
    while (!writes.empty()) {
      QueuedWrite& w=writes.front();
      bleh.write(w.addr,w.val);
      writes.pop_front();
    }

    bleh.tick();
    oscBuf[0]->putSample(h,bleh.getChanOutput(0)<<7);
    oscBuf[1]->putSample(h,bleh.getChanOutput(1)<<7);
    buf[0][h]=(bleh.getOutput()-256)<<7;
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
      chan[i].volume=VOL_SCALE_LOG(chan[i].vol,MIN(8,chan[i].std.vol.val),8);
      CLAMP_VAR(chan[i].volume, 0, 7)
      chan[i].state=(chan[i].state&~7)|(6-chan[i].volume);
      rWrite(BLEH_ADDR_CHAN1STATE+i, chan[i].state)
    }
    if (chan[i].std.wave.had) {
      chan[i].waveNum=chan[i].std.wave.val;
      CLAMP_VAR(chan[i].waveNum, 0, 3)
      chan[i].state=(chan[i].state&~0b11000)|(chan[i].waveNum<<3);
      rWrite(BLEH_ADDR_CHAN1STATE+i, chan[i].state)
    }
    if (chan[i].std.ex1.had) {
      chan[i].control=chan[i].std.ex1.val;
      CLAMP_VAR(chan[i].control, 0, 7)
      chan[i].state=(chan[i].state&~0b11100000)|(chan[i].control<<5);
      rWrite(BLEH_ADDR_CHAN1STATE+i, chan[i].state)
    }
    if (chan[i].std.duty.had) {
      chan[i].noiseFreq=chan[i].std.duty.val;
      rWrite(BLEH_ADDR_CHAN1NOISEFREQ+i, chan[i].noiseFreq)
    }
    if (NEW_ARP_STRAT) {
      chan[i].handleArp();
    } else if (chan[i].std.arp.had) {
      if (!chan[i].inPorta) {
        chan[i].baseFreq=NOTE_IDK(parent->calcArp(chan[i].note,chan[i].std.arp.val));
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
      chan[i].freq=parent->calcFreq(chan[i].baseFreq,chan[i].pitch,chan[i].fixedArp?chan[i].baseNoteOverride:chan[i].arpOff,chan[i].fixedArp,false,0,chan[i].pitch2,chipClock,CHIP_DIVIDER)-1;
      if (chan[i].freq<0) chan[i].freq=0;
      if (chan[i].freq>65535) chan[i].freq=65535;

      rWrite(BLEH_ADDR_CHAN1FREQHIGH+2*i, chan[i].keyOff?0:(chan[i].freq)>>8)
      rWrite(BLEH_ADDR_CHAN1FREQLOW+2*i, chan[i].keyOff?0:(chan[i].freq)&0xff)
      rWrite(BLEH_ADDR_CHAN1NOISEFREQ+i, chan[i].keyOff?0:chan[i].noiseFreq)
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
        chan[c.chan].baseFreq=NOTE_IDK(c.value);
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
      rWrite(BLEH_ADDR_CHAN1FREQHIGH+2*c.chan, 0);
      rWrite(BLEH_ADDR_CHAN1FREQLOW+2*c.chan, 0);
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
        // if (chan[c.chan].active) {
        //   on=chan[c.chan].vol;
        // }
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
      int destFreq=NOTE_IDK(c.value2);
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
      chan[c.chan].baseFreq=NOTE_IDK(c.value+((HACKY_LEGATO_MESS)?(chan[c.chan].std.arp.val):(0)));
      chan[c.chan].freqChanged=true;
      chan[c.chan].note=c.value;
      break;
    case DIV_CMD_PRE_PORTA:
      if (chan[c.chan].active && c.value2) {
        if (parent->song.resetMacroOnPorta) chan[c.chan].macroInit(parent->getIns(chan[c.chan].ins,DIV_INS_BLEH));
      }
      if (!chan[c.chan].inPorta && c.value && !parent->song.brokenPortaArp && chan[c.chan].std.arp.will && !NEW_ARP_STRAT) chan[c.chan].baseFreq=NOTE_IDK(chan[c.chan].note);
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
      unsigned char stateCopy=chan[c.chan].state&0b11100111;
      stateCopy|=chan[c.chan].waveNum<<3;
      chan[c.chan].state=stateCopy;
      rWrite(BLEH_ADDR_CHAN1STATE+c.chan, stateCopy);
      break;
    }
    case DIV_CMD_BLEH_NOISEFREQ:
      chan[c.chan].noiseFreq=c.value;
      rWrite(BLEH_ADDR_CHAN1NOISEFREQ+c.chan, c.value);
      break;
    case DIV_CMD_BLEH_CONTROL: {
      chan[c.chan].control=c.value&7;
      unsigned char stateCopy=chan[c.chan].state&0b00011111;
      stateCopy|=chan[c.chan].control<<5;
      chan[c.chan].state=stateCopy;
      rWrite(BLEH_ADDR_CHAN1STATE+c.chan, stateCopy);
      break;
    }
    default:
      break;
  }
  return 1;
}

void DivPlatformBleh::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  rWrite(BLEH_ADDR_CHAN1FREQHIGH+2*ch, isMuted[ch]?0:(chan[ch].freq)>>8);
  rWrite(BLEH_ADDR_CHAN1FREQLOW+2*ch, isMuted[ch]?0:(chan[ch].freq)&0xff);
}

void DivPlatformBleh::forceIns() {
  for (int i=0; i<2; i++) {
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
  return regPool;
}

int DivPlatformBleh::getRegisterPoolSize() {
  return 8;
}

void DivPlatformBleh::reset() {
  bleh.reset();
  for (int i=0; i<2; i++) {
    chan[i]=DivPlatformBleh::Channel();
    chan[i].std.setEngine(parent);
  }
  if (dumpWrites) {
    addWrite(0xffffffff,0);
  }
}

bool DivPlatformBleh::keyOffAffectsArp(int ch) {
  return true;
}

bool DivPlatformBleh::hasAcquireDirect() {
  return false;
}

void DivPlatformBleh::setFlags(const DivConfig& flags) {
  // CHECK_CUSTOM_CLOCK;
  chipClock=100000;
  rate=chipClock;
  for (int i=0; i<2; i++) {
    oscBuf[i]->setRate(chipClock);
  }
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
  regPool=bleh.getRegSpace();
  return 2;
}

void DivPlatformBleh::quit() {
  for (int i=0; i<2; i++) {
    delete oscBuf[i];
  }
}

DivPlatformBleh::~DivPlatformBleh() {
}
