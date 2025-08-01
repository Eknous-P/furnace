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

#include "fileOpsCommon.h"

static const char* trueFalse[2]={
  "no", "yes"
};

static const char* gbEnvDir[2]={
  "down", "up"
};

static const char* notes[12]={
  "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"
};

static const char* notesNegative[12]={
  "c_", "c+", "d_", "d+", "e_", "f_", "f+", "g_", "g+", "a_", "a+", "b_"
};

static const char* sampleLoopModes[4]={
  "forward", "backward", "ping-pong", "invalid"
};

void writeTextMacro(SafeWriter* w, DivInstrumentMacro& m, const char* name, bool& wroteMacroHeader) {
  if ((m.open&6)==0 && m.len<1) return;
  if (!wroteMacroHeader) {
    w->writeText("- macros:\n");
    wroteMacroHeader=true;
  }
  w->writeText(fmt::sprintf("  - %s:",name));
  int len=m.len;
  switch (m.open&6) {
    case 2:
      len=16;
      w->writeText(" [ADSR]");
      break;
    case 4:
      len=16;
      w->writeText(" [LFO]");
      break;
  }
  if (m.mode) {
    w->writeText(fmt::sprintf(" [MODE %d]",m.mode));
  }
  if (m.delay>0) {
    w->writeText(fmt::sprintf(" [DELAY %d]",m.delay));
  }
  if (m.speed>1) {
    w->writeText(fmt::sprintf(" [SPEED %d]",m.speed));
  }
  for (int i=0; i<len; i++) {
    if (i==m.loop) {
      w->writeText(" |");
    }
    if (i==m.rel) {
      w->writeText(" /");
    }
    w->writeText(fmt::sprintf(" %d",m.val[i]));
  }
  w->writeText("\n");
}

SafeWriter* DivEngine::saveText(bool separatePatterns) {
  saveLock.lock();

  SafeWriter* w=new SafeWriter;
  w->init();

  w->writeText(fmt::sprintf("# Furnace Text Export\n\ngenerated by Furnace %s (%d)\n\n# Song Information\n\n",DIV_VERSION,DIV_ENGINE_VERSION));
  w->writeText(fmt::sprintf("- name: %s\n",song.name));
  w->writeText(fmt::sprintf("- author: %s\n",song.author));
  w->writeText(fmt::sprintf("- album: %s\n",song.category));
  w->writeText(fmt::sprintf("- system: %s\n",song.systemName));
  w->writeText(fmt::sprintf("- tuning: %g\n\n",song.tuning));
  
  w->writeText(fmt::sprintf("- instruments: %d\n",song.insLen));
  w->writeText(fmt::sprintf("- wavetables: %d\n",song.waveLen));
  w->writeText(fmt::sprintf("- samples: %d\n\n",song.sampleLen));
  
  w->writeText("# Sound Chips\n\n");

  for (int i=0; i<song.systemLen; i++) {
    w->writeText(fmt::sprintf("- %s\n",getSystemName(song.system[i])));
    w->writeText(fmt::sprintf("  - id: %.2X\n",(int)song.system[i]));
    w->writeText(fmt::sprintf("  - volume: %g\n",song.systemVol[i]));
    w->writeText(fmt::sprintf("  - panning: %g\n",song.systemPan[i]));
    w->writeText(fmt::sprintf("  - front/rear: %g\n",song.systemPanFR[i]));
    
    String sysFlags=song.systemFlags[i].toString();

    if (!sysFlags.empty()) {
      w->writeText(fmt::sprintf("  - flags:\n```\n%s\n```\n",sysFlags));
    }
  }

  if (!song.notes.empty()) {
    w->writeText("\n# Song Comments\n\n");
    w->writeText(song.notes);
  }

  w->writeText("\n# Instruments\n\n");

  for (int i=0; i<song.insLen; i++) {
    DivInstrument* ins=song.ins[i];

    w->writeText(fmt::sprintf("## %.2X: %s\n\n",i,ins->name));

    w->writeText(fmt::sprintf("- type: %d\n",(int)ins->type));

    if (ins->type==DIV_INS_FM || ins->type==DIV_INS_OPL || ins->type==DIV_INS_OPLL || ins->type==DIV_INS_OPZ || ins->type==DIV_INS_OPL_DRUMS || ins->type==DIV_INS_OPM || ins->type==DIV_INS_ESFM) {
      int opCount=4;
      if (ins->type==DIV_INS_OPLL) {
        opCount=2;
      } else if (ins->type==DIV_INS_OPL) {
        opCount=(ins->fm.ops==4)?4:2;
      }

      w->writeText("- FM parameters:\n");
      w->writeText(fmt::sprintf("  - ALG: %d\n",ins->fm.alg));
      w->writeText(fmt::sprintf("  - FB: %d\n",ins->fm.fb));
      w->writeText(fmt::sprintf("  - FMS: %d\n",ins->fm.fms));
      w->writeText(fmt::sprintf("  - AMS: %d\n",ins->fm.ams));
      w->writeText(fmt::sprintf("  - FMS2: %d\n",ins->fm.fms2));
      w->writeText(fmt::sprintf("  - AMS2: %d\n",ins->fm.ams2));
      w->writeText(fmt::sprintf("  - operators: %d\n",opCount));
      w->writeText(fmt::sprintf("  - OPLL patch: %d\n",ins->fm.opllPreset));
      w->writeText(fmt::sprintf("  - fixed drum freq: %s\n",trueFalse[ins->fm.fixedDrums?1:0]));
      w->writeText(fmt::sprintf("  - kick freq: %.4X\n",ins->fm.kickFreq));
      w->writeText(fmt::sprintf("  - snare/hat freq: %.4X\n",ins->fm.snareHatFreq));
      w->writeText(fmt::sprintf("  - tom/top freq: %.4X\n",ins->fm.tomTopFreq));
      
      for (int j=0; j<opCount; j++) {
        DivInstrumentFM::Operator& op=ins->fm.op[j];

        w->writeText(fmt::sprintf("  - operator %d:\n",j));
        w->writeText(fmt::sprintf("    - enabled: %s\n",trueFalse[op.enable?1:0]));
        w->writeText(fmt::sprintf("    - AM: %d\n",op.am));
        w->writeText(fmt::sprintf("    - AR: %d\n",op.ar));
        w->writeText(fmt::sprintf("    - DR: %d\n",op.dr));
        w->writeText(fmt::sprintf("    - MULT: %d\n",op.mult));
        w->writeText(fmt::sprintf("    - RR: %d\n",op.rr));
        w->writeText(fmt::sprintf("    - SL: %d\n",op.sl));
        w->writeText(fmt::sprintf("    - TL: %d\n",op.tl));
        w->writeText(fmt::sprintf("    - DT2: %d\n",op.dt2));
        w->writeText(fmt::sprintf("    - RS: %d\n",op.rs));
        w->writeText(fmt::sprintf("    - DT: %d\n",op.dt));
        w->writeText(fmt::sprintf("    - D2R: %d\n",op.d2r));
        w->writeText(fmt::sprintf("    - SSG-EG: %d\n",op.ssgEnv));
        w->writeText(fmt::sprintf("    - DAM: %d\n",op.dam));
        w->writeText(fmt::sprintf("    - DVB: %d\n",op.dvb));
        w->writeText(fmt::sprintf("    - EGT: %d\n",op.egt));
        w->writeText(fmt::sprintf("    - KSL: %d\n",op.ksl));
        w->writeText(fmt::sprintf("    - SUS: %d\n",op.sus));
        w->writeText(fmt::sprintf("    - VIB: %d\n",op.vib));
        w->writeText(fmt::sprintf("    - WS: %d\n",op.ws));
        w->writeText(fmt::sprintf("    - KSR: %d\n",op.ksr));
        w->writeText(fmt::sprintf("    - TL volume scale: %d\n",op.kvs));
      }
    }

    if (ins->type==DIV_INS_ESFM) {
      w->writeText("- ESFM parameters:\n");
      w->writeText(fmt::sprintf("  - noise mode: %d\n",ins->esfm.noise));

      for (int j=0; j<4; j++) {
        DivInstrumentESFM::Operator& opE=ins->esfm.op[j];

        w->writeText(fmt::sprintf("  - operator %d:\n",j));
        w->writeText(fmt::sprintf("    - DL: %d\n",opE.delay));
        w->writeText(fmt::sprintf("    - OL: %d\n",opE.outLvl));
        w->writeText(fmt::sprintf("    - MI: %d\n",opE.modIn));
        w->writeText(fmt::sprintf("    - output left: %s\n",trueFalse[opE.left?1:0]));
        w->writeText(fmt::sprintf("    - output right: %s\n",trueFalse[opE.right?1:0]));
        w->writeText(fmt::sprintf("    - CT: %d\n",opE.ct));
        w->writeText(fmt::sprintf("    - DT: %d\n",opE.dt));
        w->writeText(fmt::sprintf("    - fixed frequency: %s\n",trueFalse[opE.fixed?1:0]));
      }
    }

    if (ins->type==DIV_INS_GB) {
      w->writeText("- Game Boy parameters:\n");
      w->writeText(fmt::sprintf("  - volume: %d\n",ins->gb.envVol));
      w->writeText(fmt::sprintf("  - direction: %s\n",gbEnvDir[ins->gb.envDir?1:0]));
      w->writeText(fmt::sprintf("  - length: %d\n",ins->gb.envLen));
      w->writeText(fmt::sprintf("  - sound length: %d\n",ins->gb.soundLen));
      w->writeText(fmt::sprintf("  - use software envelope: %s\n",trueFalse[ins->gb.softEnv?1:0]));
      w->writeText(fmt::sprintf("  - always initialize: %s\n",trueFalse[ins->gb.softEnv?1:0]));
      if (ins->gb.hwSeqLen>0) {
        w->writeText("  - hardware sequence:\n");
        for (int j=0; j<ins->gb.hwSeqLen; j++) {
          w->writeText(fmt::sprintf("    - %d: %.2X %.4X\n",j,ins->gb.hwSeq[j].cmd,ins->gb.hwSeq[j].data));
        }
      }
    }

    bool header=false;
    writeTextMacro(w,ins->std.volMacro,"vol",header);
    writeTextMacro(w,ins->std.arpMacro,"arp",header);
    writeTextMacro(w,ins->std.dutyMacro,"duty",header);
    writeTextMacro(w,ins->std.waveMacro,"wave",header);
    writeTextMacro(w,ins->std.pitchMacro,"pitch",header);
    writeTextMacro(w,ins->std.panLMacro,"panL",header);
    writeTextMacro(w,ins->std.panRMacro,"panR",header);
    writeTextMacro(w,ins->std.phaseResetMacro,"phaseReset",header);
    writeTextMacro(w,ins->std.ex1Macro,"ex1",header);
    writeTextMacro(w,ins->std.ex2Macro,"ex2",header);
    writeTextMacro(w,ins->std.ex3Macro,"ex3",header);
    writeTextMacro(w,ins->std.ex4Macro,"ex4",header);
    writeTextMacro(w,ins->std.ex5Macro,"ex5",header);
    writeTextMacro(w,ins->std.ex6Macro,"ex6",header);
    writeTextMacro(w,ins->std.ex7Macro,"ex7",header);
    writeTextMacro(w,ins->std.ex8Macro,"ex8",header);
    writeTextMacro(w,ins->std.ex9Macro,"ex9",header);
    writeTextMacro(w,ins->std.ex10Macro,"ex10",header);
    writeTextMacro(w,ins->std.algMacro,"alg",header);
    writeTextMacro(w,ins->std.fbMacro,"fb",header);
    writeTextMacro(w,ins->std.fmsMacro,"fms",header);
    writeTextMacro(w,ins->std.amsMacro,"ams",header);

    // TODO: the rest
    w->writeText("\n");
  }

  w->writeText("\n# Wavetables\n\n");

  for (int i=0; i<song.waveLen; i++) {
    DivWavetable* wave=song.wave[i];

    w->writeText(fmt::sprintf("- %d (%dx%d):",i,wave->len,wave->max+1));
    for (int j=0; j<=wave->len; j++) {
      w->writeText(fmt::sprintf(" %d",wave->data[j]));
    }
    w->writeText("\n");
  }

  w->writeText("\n# Samples\n\n");

  for (int i=0; i<song.sampleLen; i++) {
    DivSample* sample=song.sample[i];

    w->writeText(fmt::sprintf("## %.2X: %s\n\n",i,sample->name));

    w->writeText(fmt::sprintf("- format: %d\n",(int)sample->depth));
    w->writeText(fmt::sprintf("- data length: %d\n",sample->getCurBufLen()));
    w->writeText(fmt::sprintf("- samples: %d\n",sample->samples));
    w->writeText(fmt::sprintf("- rate: %d\n",sample->centerRate));
    w->writeText(fmt::sprintf("- compat rate: %d\n",sample->rate));
    w->writeText(fmt::sprintf("- loop: %s\n",trueFalse[sample->loop?1:0]));
    if (sample->loop) {
      w->writeText(fmt::sprintf("  - start: %d\n",sample->loopStart));
      w->writeText(fmt::sprintf("  - end: %d\n",sample->loopEnd));
      w->writeText(fmt::sprintf("  - mode: %s\n",sampleLoopModes[sample->loopMode&3]));
    }
    w->writeText(fmt::sprintf("- BRR emphasis: %s\n",trueFalse[sample->brrEmphasis?1:0]));
    w->writeText(fmt::sprintf("- no BRR filters: %s\n",trueFalse[sample->brrNoFilter?1:0]));
    w->writeText(fmt::sprintf("- dither: %s\n",trueFalse[sample->dither?1:0]));

    // TODO' render matrix
    unsigned char* buf=(unsigned char*)sample->getCurBuf();
    unsigned int bufLen=sample->getCurBufLen();
    w->writeText("\n```");
    for (unsigned int i=0; i<bufLen; i++) {
      if ((i&15)==0) w->writeText(fmt::sprintf("\n%.8X:",i));
      w->writeText(fmt::sprintf(" %.2X",buf[i]));
    }
    w->writeText("\n```\n\n");
  }

  w->writeText("\n# Subsongs\n\n");

  for (size_t i=0; i<song.subsong.size(); i++) {
    DivSubSong* s=song.subsong[i];
    w->writeText(fmt::sprintf("## %d: %s\n\n",(int)i,s->name));

    w->writeText(fmt::sprintf("- tick rate: %g\n",s->hz));
    w->writeText(fmt::sprintf("- speeds:"));
    for (int j=0; j<s->speeds.len; j++) {
      w->writeText(fmt::sprintf(" %d",s->speeds.val[j]));
    }
    w->writeText("\n");
    w->writeText(fmt::sprintf("- virtual tempo: %d/%d\n",s->virtualTempoN,s->virtualTempoD));
    w->writeText(fmt::sprintf("- time base: %d\n",s->timeBase));
    w->writeText(fmt::sprintf("- pattern length: %d\n",s->patLen));
    w->writeText(fmt::sprintf("\norders:\n```\n"));

    for (int j=0; j<s->ordersLen; j++) {
      w->writeText(fmt::sprintf("%.2X |",j));
      for (int k=0; k<chans; k++) {
        w->writeText(fmt::sprintf(" %.2X",s->orders.ord[k][j]));
      }
      w->writeText("\n");
    }
    w->writeText("```\n\n## Patterns\n\n");

    if (separatePatterns) {
      w->writeText("TODO: separate patterns\n\n");
    } else {
      for (int j=0; j<s->ordersLen; j++) {
        w->writeText(fmt::sprintf("----- ORDER %.2X\n",j));

        for (int k=0; k<s->patLen; k++) {
          w->writeText(fmt::sprintf("%.2X ",k));

          for (int l=0; l<chans; l++) {
            DivPattern* p=s->pat[l].getPattern(s->orders.ord[l][j],false);

            int note=p->data[k][0];
            int octave=p->data[k][1];

            if (note==0 && octave==0) {
              w->writeText("|... ");
            } else if (note==100) {
              w->writeText("|OFF ");
            } else if (note==101) {
              w->writeText("|=== ");
            } else if (note==102) {
              w->writeText("|REL ");
            } else if ((octave>9 && octave<250) || note>12) {
              w->writeText("|??? ");
            } else {
              if (octave>=128) octave-=256;
              if (note>11) {
                note-=12;
                octave++;
              }
              w->writeText(fmt::sprintf("|%s%d ",(octave<0)?notesNegative[note]:notes[note],(octave<0)?(-octave):octave));
            }

            if (p->data[k][2]==-1) {
              w->writeText(".. ");
            } else {
              w->writeText(fmt::sprintf("%.2X ",p->data[k][2]&0xff));
            }

            if (p->data[k][3]==-1) {
              w->writeText("..");
            } else {
              w->writeText(fmt::sprintf("%.2X",p->data[k][3]&0xff));
            }

            for (int m=0; m<s->pat[l].effectCols; m++) {
              if (p->data[k][4+(m<<1)]==-1) {
                w->writeText(" ..");
              } else {
                w->writeText(fmt::sprintf(" %.2X",p->data[k][4+(m<<1)]&0xff));
              }
              if (p->data[k][5+(m<<1)]==-1) {
                w->writeText("..");
              } else {
                w->writeText(fmt::sprintf("%.2X",p->data[k][5+(m<<1)]&0xff));
              }
            }
          }

          w->writeText("\n");
        }
      }
    }

  }

  saveLock.unlock();
  return w;
}
