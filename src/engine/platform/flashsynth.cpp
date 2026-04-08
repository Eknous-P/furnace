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
#include "../engine.h"

// #define CHIP_FREQBASE 2048

struct fsAlgoFlags {
  int oscs;
  bool mono;
};

fsAlgoFlags algFlags[]={
  {1,false},
  {2,false},
  {2,true},
  {2,false},
  {1,false},
  {2,false},
  {2,true},
  {4,false},
  {1,false},
  {3,true},
};

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

void DivPlatformFlashSynth::muteChannel(int ch, bool mute) {
  isMuted[ch]=mute;
  flashsynth_muteChannel(&fs, ch, mute);
}

void DivPlatformFlashSynth::tick(bool sysTick) {
  for (int i=0; i<16; i++) {
    if (chan[i].insChanged) {
      chan[i].insChanged=false;
      DivInstrument* ins=parent->getIns(chan[i].ins,DIV_INS_FLASHSYNTH);
      if (ins->flash.usePatch) {
        flashsynth_loadPatch(&fs, ins->flash.patch);
      } else {
        DivInstrumentFlashSynth f=ins->flash;
        // literal copypase from parameterChange...
        flashsynth_parameterChange(&fs, i, cc_sustain, f.sustain);
        fs.channels[i].mod=f.lfoDepth;
        fs.channels[i].lfo_depth=f.lfoDepth*8.0f;
        fs.lfo_freq=f.lfoFreq==127? 0.0: 204.8 + (float)(f.lfoFreq*4);
        fs.pwm_freq=(float)(f.lfoFreq*f.lfoFreq)*0.04419368838737677; //[0..712.8]
        fs.attackrate_cc=f.attack;
        fs.releaserate_cc=f.release;
        fs.outputGain = f.gain==0 ? 1.0 : f.gain*0.125;
        fs.attackRate = (0.128 / (float)(fs.attackrate_cc + 1)) * fs.outputGain;
        fs.releaseRate = -(0.128 / (float)(fs.releaserate_cc + 1)) * fs.outputGain;
        fs.fm_freq_cc[0]=f.fmFreq;
        fs.fm_freq_cc[1]=f.fmFreqFine;
        fs.fm_freq = (float)((fs.fm_freq_cc[0] << 7) + fs.fm_freq_cc[1]) / 1024;
        fs.fm_depth=25.0f*f.fmDepth/127.0f;
        fs.fm_attack_cc=f.fmAttack;
        fs.fm_attack = (fs.fm_depth * 0.001 / ((float)fs.fm_attack_cc + 0.5));
        fs.fm_decay=1.0f-((float)(f.fmDecay*f.fmDecay)/25400000.f);
        float detune= ((float)f.detune/10160.0);
        fs.detuneUp = 1.0 + detune;
        fs.detuneDown = 1.0 - detune;
        fs.pwm_depth=1795.f*f.pwmDepth/127.f;
        if (f.waveform!=f.oldWaveform || f.waveformParam!=f.oldWaveformParam) {
          flashsynth_setWaveform(&fs, f.waveform, f.waveformParam);
          f.oldWaveform=f.waveform;
          f.oldWaveformParam=f.waveformParam;
        }
        if (f.oldAlg!=f.alg) {
          flashsynth_parameterChange(&fs, 0, cc_algo, f.alg);
          f.oldAlg=f.alg;
          alg=f.alg;
        }
      }
    }
    if (chan[i].freqChanged || chan[i].keyOn || chan[i].keyOff) {
      int note=chan[i].note;
      chan[i].freqChanged=false;
      if (chan[i].keyOn) {
        fs.noteOn(&fs, i, note, chan[i].vol, i);
        chan[i].keyOn=false;
      }
      if (chan[i].keyOff) {
        fs.noteOff(&fs, i, note, i);
        chan[i].keyOff=false;
      }
    }
  }
}

int DivPlatformFlashSynth::dispatch(DivCommand c) {
  switch (c.cmd) {
    case DIV_CMD_NOTE_ON: {
      chan[c.chan].keyOn=true;
      if (c.value!=DIV_NOTE_NULL) {
        chan[c.chan].freqChanged=true;
        chan[c.chan].note=c.value;
      }
      break;
    }
    case DIV_CMD_NOTE_OFF:
    case DIV_CMD_NOTE_OFF_ENV:
    case DIV_CMD_ENV_RELEASE:
      chan[c.chan].keyOff=true;
      break;
    case DIV_CMD_VOLUME:
      chan[c.chan].vol=c.value;
      break;
    case DIV_CMD_GET_VOLMAX:
      return 127;
    case DIV_CMD_INSTRUMENT:
      if (chan[c.chan].ins!=c.value || c.value2==1) {
        chan[c.chan].ins=c.value;
        chan[c.chan].insChanged=true;
      }
      break;
    default: break;
  }
  return 1;
}

void DivPlatformFlashSynth::notifyInsChange(int ins) {
  for (int i=0; i<16; i++) {
    if (chan[i].ins==ins) {
      chan[i].insChanged=true;
    }
  }
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
void DivPlatformFlashSynth::getPaired(int ch, std::vector<DivChannelPair>& ret) {
  ret.clear();
  switch (algFlags[alg].oscs) {
    case 2:
      if ((algFlags[alg].mono && ch==0) || (!algFlags[alg].mono && (ch&1)==0)) {
        ret.push_back(DivChannelPair("stereo", ch+1));
      }
      return;
    case 3:
      if ((algFlags[alg].mono && ch==0)) {
        ret.push_back(DivChannelPair("tri", ch+1,ch+2,-1,-1,-1,-1,-1,-1));
      }
      return;
    case 4:
      if ((algFlags[alg].mono && ch==0) || (!algFlags[alg].mono && (ch&3)==0)) {
        ret.push_back(DivChannelPair("quad", ch+1,ch+2,ch+3,-1,-1,-1,-1,-1));
      }
      return;
    case 1:
      if (algFlags[alg].mono && ch==0) {
        ret.push_back(DivChannelPair("mono", ch+1));
      }
      return;
  }
}

void DivPlatformFlashSynth::reset() {
  flashsynth_reset(&fs);
  flashsynth_parameterChange(&fs, 0, cc_all_channels_tuning, 0);
  // flashsynth_setWaveform(&fs, 0, 0);
  for (int i=0; i<16; i++) {
    chan[i]=Channel();
    isMuted[i]=false;
    // flashsynth_parameterChange(&fs, i, cc_output_gain, 127);
    flashsynth_parameterChange(&fs, i, cc_sustain, 0);
    chan[i].osc=&fs.oscillators[i];
    chan[i].chan=&fs.channels[i];
    fs.noteOn(&fs,0,0,0,i);
    fs.noteOff(&fs,i,0,i);
  }
  flashsynth_loadPatch(&fs, 0);
  alg=0;
}

int DivPlatformFlashSynth::init(DivEngine* p, int ch, int sugRate, const DivConfig& flags) {
  parent=p;
  rate=chipClock=44107;
  for (int i=0; i<16; i++) {
    oscBuf[i]=new DivDispatchOscBuffer;
    oscBuf[i]->setRate(44107);
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
