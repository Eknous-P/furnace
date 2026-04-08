/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2026 tildearrow and contributors
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

#ifndef _FLASHSYNTH_H
#define _FLASHSYNTH_H

#include "../dispatch.h"
extern "C" {
#include "../../extern/flash-synth-core/Src/flashsynth.h"
}

class DivPlatformFlashSynth : public DivDispatch {
  struct Channel : public SharedChannel<unsigned char>{
    struct flashsynth_instance::flashsynth_oscillator* osc;
    struct flashsynth_instance::flashsynth_channel* chan;
    Channel():
      SharedChannel<unsigned char>(127),
      osc(NULL),
      chan(NULL)
    {}
  };
  Channel chan[16];
  DivDispatchOscBuffer* oscBuf[16];
  bool isMuted[16];
  flashsynth_instance fs;
  unsigned char alg;
  unsigned char chans;  
  friend void putDispatchChip(void*,int);
  friend void putDispatchChan(void*,int,int);
  public:
    void acquire(short** buf, size_t len);
    void muteChannel(int ch, bool mute);
    int dispatch(DivCommand c);
    // void notifyInsDeletion(void* ins);
    void notifyInsChange(int ins);
    void* getChanState(int i);
    DivDispatchOscBuffer* getOscBuffer(int ch);
    int getOutputCount();
    unsigned char* getRegisterPool();
    int getRegisterPoolSize();
    void getPaired(int ch, std::vector<DivChannelPair>& ret);
    void reset();
    void tick(bool sysTick=true);
    int init(DivEngine* parent, int channels, int sugRate, const DivConfig& flags);
    void quit();
    ~DivPlatformFlashSynth();
};

#endif
