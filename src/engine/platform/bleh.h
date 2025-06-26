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

#ifndef _BLEH_H
#define _BLEH_H

#include "../dispatch.h"
#include "../../fixedQueue.h"

#define INCLUDE_BLEH_ROM
#define INCLUDE_BLEH_REG_HELPER
#define INCLUDE_BLEH_EXTRA_FUNCTIONS
#include "sound/bleh.cpp"

class DivPlatformBleh: public DivDispatch {
  struct Channel: public SharedChannel<signed char> {
    unsigned char volume, waveNum, noiseFreq, control, state;
    Channel():
      SharedChannel<signed char>(7),
      volume(7),
      waveNum(0),
      noiseFreq(0),
      control(2),
      state(64) {}
  };
  Channel chan[2];
  DivDispatchOscBuffer* oscBuf[2];
  blehSys bleh;
  bool isMuted[2];

  unsigned char* regPool;
  struct QueuedWrite {
    unsigned short addr;
    unsigned char val;
    QueuedWrite(): addr(0), val(0) {}
    QueuedWrite(unsigned short a, unsigned char v): addr(a), val(v) {}
  };
  FixedQueue<QueuedWrite,128> writes;

  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);

  public:
    void acquireDirect(blip_buffer_t** bb, size_t len);
    void acquire(short** buf, size_t len);
    int dispatch(DivCommand c);
    void* getChanState(int chan);
    DivMacroInt* getChanMacroInt(int ch);
    DivDispatchOscBuffer* getOscBuffer(int chan);
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void reset();
    void forceIns();
    void tick(bool sysTick=true);
    void muteChannel(int ch, bool mute);
    bool keyOffAffectsArp(int ch);
    bool hasAcquireDirect();
    void setFlags(const DivConfig& flags);
    void notifyInsDeletion(void* ins);
    void notifyPlaybackStop();
    void poke(unsigned int addr, unsigned short val);
    void poke(std::vector<DivRegWrite>& wlist);
    const char** getRegisterSheet();
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformBleh();
};

#endif
