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

#include "ne555.h"
#include "../engine.h"
#include "furIcons.h"
#include <math.h>


void DivPlatformNE555::acquire(short** buf, size_t len) {
  int chanOut;
  for (size_t i=0; i<len; i++) {
    ne->tick(1.0f/chipClock);
    int out=0;
    if (chan[0].active) {
      if (!isMuted[0]) {
        chanOut=ne->out?0x3fff:0;
        oscBuf->data[oscBuf->needle++]=chanOut;
        out+=chanOut;
      } else {
        oscBuf->data[oscBuf->needle++]=0;
      }
    } else {
      oscBuf->data[oscBuf->needle++]=0;
    }
    if (out<-32768) out=-32768;
    if (out>32767) out=32767;
    buf[0][i]=out;
  }
}

void DivPlatformNE555::tick(bool sysTick) {
  for (int i=0; i<1; i++) {
    chan[i].std.next();
    if (chan[i].std.vol.had) {
      chan[i].outVol=(chan[i].vol && chan[i].std.vol.val);
      ne->reset=chan[i].outVol;
    }
  }
}

int DivPlatformNE555::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      chan[c.chan].active=true;
      chan[c.chan].keyOn=true;
      ne->reset=chan[c.chan].vol>0;
      if (!parent->song.brokenOutVol && !chan[c.chan].std.vol.will) {
        chan[c.chan].outVol=chan[c.chan].vol;
      }
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].active=false;
      chan[c.chan].keyOff=true;
      ne->reset=0;
      break;
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].std.release();
      ne->reset=0;
      break;
    case DIV_CMD_VOLUME:
      ne->reset=chan[c.chan].vol>0;
      break;
    case DIV_CMD_GET_VOLUME:
      return chan[c.chan].vol;
      break;
    default:
      break;
  }
  return 1;
}

void DivPlatformNE555::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
}

void DivPlatformNE555::forceIns() {
  // ne555 haveth no instruments lol
}

void* DivPlatformNE555::getChanState(int ch) {
  return &chan[ch];
}

DivMacroInt* DivPlatformNE555::getChanMacroInt(int ch) {
  return &chan[ch].std;
}


DivDispatchOscBuffer* DivPlatformNE555::getOscBuffer(int ch) {
  return oscBuf;
}

int DivPlatformNE555::mapVelocity(int ch, float vel) {
  return round(vel/127);
}

float DivPlatformNE555::getGain(int ch, int vol) {
  return vol;
}

unsigned char* DivPlatformNE555::getRegisterPool() {
  return NULL;
}

int DivPlatformNE555::getRegisterPoolSize() {
  return 0;
}

void DivPlatformNE555::reset() {
  ne->reset=0;
}

int DivPlatformNE555::getOutputCount() {
  return 1;
}

bool DivPlatformNE555::keyOffAffectsArp(int ch) {
  return true;
}

void DivPlatformNE555::notifyInsDeletion(void* ins) {
  for (int i=0; i<1; i++) {
    chan[i].std.notifyInsDeletion((DivInstrument*)ins);
  }
}

void DivPlatformNE555::setFlags(const DivConfig& flags) {
  rate=chipClock/2;
  oscBuf->rate=rate;

  R1=flags.getFloat("R1", 1000.0f);
  R2=flags.getFloat("R2", 1000.0f);
  C=flags.getDouble("C", 0.001f); // mF

  if (ne!=NULL) {
    delete ne;
    ne=NULL;
  }
  ne=new ne555(R1,R2,C);
  ne->reset=0;
}

void DivPlatformNE555::poke(unsigned int addr, unsigned short val) {
}

void DivPlatformNE555::poke(std::vector<DivRegWrite>& wlist) {
}

void DivPlatformNE555::setCoreQuality(unsigned char q) {
}

int DivPlatformNE555::init(DivEngine* p, int channels, int sugRate, const DivConfig& flags) {
  parent=p;
  isMuted[0]=false;
  chipClock=1000000;
  rate=chipClock/4;
  oscBuf=new DivDispatchOscBuffer;
  for (int i=0; i<1; i++) {
    isMuted[i]=false;
  }
  ne=NULL;
  setFlags(flags);
  reset();
  return 6;
}

void DivPlatformNE555::quit() {
  delete oscBuf;
  if (ne!=NULL) {
    delete ne;
    ne=NULL;
  }
}

DivPlatformNE555::~DivPlatformNE555() {
}
