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

#include "flashsynth.h"

// #define CHIP_FREQBASE 2048

void DivPlatformFlashSynth::acquire(short** buf, size_t len) {
  // unsigned short left, right;
  for (int i=0; i<16; i++) {
    oscBuf[i]->begin(len);
  }
  for (size_t i=0; i<len; i++) {
    fs.generate(&fs, &buf[0][i], &buf[1][i]);
    for (int j=0; j<16; j++) {
      oscBuf[j]->putSample(i, fs.channels[j].lastSample);
    }
  }
  for (int i=0; i<16; i++) {
    oscBuf[i]->end(len);
  }
}

void DivPlatformFlashSynth::tick(bool sysTick) {
  for (int i=0; i<16; i++) {
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      int note=chan[i].note;
      chan[i].freqChanged=false;
      if (chan[i].keyOn) {
        fs.noteOn(&fs, i, note, chan[i].vol, i);
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        fs.noteOff(&fs, i, note, chan[i].vol);
        chan[i].keyOff=false;
      }
    }
    if (chan[i].std.ex1.had) {
      flashsynth_loadPatch(&fs, chan[i].std.ex1.val);
    }
  }
}

int DivPlatformFlashSynth::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON:
      chan[c.chan].keyOn=true;
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      break;
    case DIV_CMD_NOTE_OFF:
      chan[c.chan].keyOff=true;
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
    default: break;
  }
  return 1;
}

void* DivPlatformFlashSynth::getChanState(int i) {
  return &chan[i];
}

DivDispatchOscBuffer* DivPlatformFlashSynth::getOscBuffer(int ch) {
  return oscBuf[ch];
}

int DivPlatformFlashSynth::getOutputCount() {
  return 2;
}

unsigned char* DivPlatformFlashSynth::getRegisterPool() {
  return (unsigned char*)&fs;
}

int DivPlatformFlashSynth::getRegisterPoolSize() {
  return sizeof (flashsynth_instance);
}

void DivPlatformFlashSynth::reset() {
  fs=flashsynth_defaultInstance();
  flashsynth_parameterChange(&fs, 0, cc_all_channels_tuning, 0);
  flashsynth_setWaveform(&fs, 0, 0);
  for (int i=0; i<16; i++) {
    chan[i]=Channel();
    isMuted[i]=false;
    flashsynth_parameterChange(&fs, i, cc_algo, 0);
    flashsynth_parameterChange(&fs, i, cc_sustain, 0);
  }
  flashsynth_loadPatch(&fs, 0);
}

int DivPlatformFlashSynth::init(DivEngine* p, int ch, int sugRate, const DivConfig& flags) {
  parent=p;
  rate=chipClock=44107;
  for (int i=0; i<16; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
    oscBuf[i]->setRate(44107);
    isMuted[i]=false;
  }
  reset();
  return 16;
}

void DivPlatformFlashSynth::quit() {
  for (int i=0; i<16; i++) {
    delete oscBuf[i];
  }
}

DivPlatformFlashSynth::~DivPlatformFlashSynth() {
}
