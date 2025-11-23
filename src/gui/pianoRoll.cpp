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

#include "gui.h"
#include "SDL_rect.h"
#include "SDL_pixels.h"
#include "SDL_surface.h"
#include <fmt/printf.h>
#include <imgui.h>

/*
TODO:
- add draw interface to struct
  - may need to be updated by the engine cuz um... tickrate, or no?
- moar cmds
- maybe use chanstate for everything
*/

void FurnaceGUI::drawPianoRoll(ImDrawList* dl, ImRect rect) {
  int& rollOff=(e->isPlaying() || pianoSharePosition)?pianoOffset:pianoOffsetEdit;
  int& rollOct=(e->isPlaying() || pianoSharePosition)?pianoOctaves:pianoOctavesEdit;
  float noteDrawWidth=((float)pianoRollData.width/rollOct)/13;
  if (pianoRollData.updateTex) {
    pianoRollData.width=rollOct*7*pianoRollData.noteWidth;
    pianoRollData.height=pianoRollData.rollTime;
    SDL_FreeSurface(pianoRollData.surface);
    pianoRollData.surface=NULL;
    rend->destroyTexture(pianoRollData.texture);
    pianoRollData.texture=NULL;
    pianoRollData.updateTex=false;
  }
  if (pianoRollData.texture==NULL) pianoRollData.texture=rend->createTexture(
    true,
    pianoRollData.width,
    pianoRollData.height,
    false,
    bestTexFormat);
  if (pianoRollData.surface==NULL) pianoRollData.surface=SDL_CreateRGBSurfaceWithFormat(
    0,
    pianoRollData.width,
    pianoRollData.height,
    32,
    SDL_PIXELFORMAT_ARGB8888);
  if (!fancyPattern) {
    e->enableCommandStream(true);
    e->getCommandStream(cmdStream);
  }
  if (pianoRollData.texture==NULL || pianoRollData.surface==NULL) return;
  SDL_LockSurface(pianoRollData.surface);
  unsigned int* px=(unsigned int*)pianoRollData.surface->pixels;
  memcpy(((unsigned char*)px)+pianoRollData.width*4,px,pianoRollData.width*4*(pianoRollData.height-1));
  memset(px,0,pianoRollData.width*4);

  // the draw code
  if (e->isRunning()) {
    for (DivCommand& i: cmdStream) {
      switch (i.cmd) {
        case DIV_CMD_NOTE_ON:
          pianoRollData.notes[i.chan].active=true;
          pianoRollData.notes[i.chan].note=i.value+60;
          pianoRollData.notes[i.chan].width=noteDrawWidth;
          pianoRollData.notes[i.chan].notePorta=0;
          pianoRollData.notes[i.chan].noteHit=1;
          break;
        case DIV_CMD_NOTE_OFF:
        case DIV_CMD_ENV_RELEASE:
        case DIV_CMD_NOTE_OFF_ENV:
          pianoRollData.notes[i.chan].active=false;
          pianoRollData.notes[i.chan].note=0;
          break;
        case DIV_CMD_VOLUME: {
          float scaledVol=(float)i.value/(float)e->getMaxVolumeChan(i.chan);
          if (scaledVol>1.0f) scaledVol=1.0f;
          pianoRollData.notes[i.chan].width=scaledVol*noteDrawWidth;
          break;
        }
        case DIV_CMD_LEGATO:
          pianoRollData.notes[i.chan].note=i.value+60;
          break;
        default: continue;
      }
    }
    for (int i=0; i<e->getTotalChannelCount(); i++) {
      if (!e->curSubSong->chanShow[i]) continue;
      if (pianoRollData.notes[i].noteHit>0) {
        pianoRollData.notes[i].noteHit-=10.f*ImGui::GetIO().DeltaTime;
        if (pianoRollData.notes[i].noteHit<0) pianoRollData.notes[i].noteHit=0;
      }
      DivChannelState* s=e->getChanState(i);
        if (s->vibratoDepth>0) {
#define vibTable(x) sin(((double)(x)/64.0)*(2*M_PI))
          // look-up table
          float vibratoOut=0;
          switch (s->vibratoShape) {
            case 1: // sine, up only
              vibratoOut=MAX(0,vibTable(s->vibratoPos));
              break;
            case 2: // sine, down only
              vibratoOut=MIN(0,vibTable(s->vibratoPos));
              break;
            case 3: // triangle
              vibratoOut=(s->vibratoPos&31);
              if (s->vibratoPos&16) {
                vibratoOut=32-(s->vibratoPos&31);
              }
              if (s->vibratoPos&32) {
                vibratoOut=-vibratoOut;
              }
              vibratoOut*=8;
              break;
            case 4: // ramp up
              vibratoOut=s->vibratoPos<<1;
              break;
            case 5: // ramp down
              vibratoOut=-s->vibratoPos<<1;
              break;
            case 6: // square
              vibratoOut=(s->vibratoPos>=32)?-127:127;
              break;
            case 7: // random (TODO: use LFSR)
              vibratoOut=(rand()&255)-128;
              break;
            case 8: // square up
              vibratoOut=(s->vibratoPos>=32)?0:127;
              break;
            case 9: // square down
              vibratoOut=(s->vibratoPos>=32)?0:-127;
              break;
            case 10: // half sine up
              vibratoOut=vibTable(s->vibratoPos>>1);
              break;
            case 11: // half sine down
              vibratoOut=vibTable(32|(s->vibratoPos>>1));
              break;
            default: // sine
              vibratoOut=vibTable(s->vibratoPos);
              break;
          }
#undef vibTable
          pianoRollData.notes[i].noteVib=vibratoOut*s->vibratoDepth;
        } else {
          pianoRollData.notes[i].noteVib=0;
        }
        // aaahhhh help pitch slides are such a mess
        // e1/e2 are backwards aaa
        if (s->portaSpeed>0 && !s->inPorta) { // pitch slide
          pianoRollData.notes[i].notePorta+=s->portaSpeed*((s->portaNote<=s->note)?-.5f:.5f);
        } else if (s->portaSpeed>0 && s->inPorta) { // portamento
          pianoRollData.notes[i].notePorta+=s->portaSpeed*((s->portaNote<=s->oldNote)?-.5f:.5f);
        } else {
          pianoRollData.notes[i].notePorta=0;
        }
    }
  } else {
    for (int i=0; i<e->getTotalChannelCount(); i++) {
      pianoRollData.notes[i].active=false;
    }
  }
  SDL_Rect noteRect={0,0,0,1};
  for (int ch=0; ch<e->getTotalChannelCount(); ch++) {
    RollNote i=pianoRollData.notes[ch];
    if (!e->curSubSong->chanShow[ch] || !i.active) continue;
    if (e->isChannelMuted(ch)) continue;
    float x=(float)(floor(i.note/12.0f)-rollOff)/rollOct;
    x+=(i.note%12)/12.0f/rollOct;
    noteRect.x=(int)round(pianoRollData.width*x+i.noteVib+i.notePorta+(pianoRollData.noteWidth-i.width)/2.0f);
    noteRect.w=i.width;
    int color=e->curSubSong->chanColor[ch];
    if (color==0) color=ImGui::GetColorU32(uiColors[GUI_COLOR_CHANNEL_FM+e->getChannelType(ch)]);
    if (i.noteHit>0) {
      ImVec4 color4=ImGui::ColorConvertU32ToFloat4(color);
      color=ImGui::ColorConvertFloat4ToU32(ImVec4(
        ImLerp(color4.x,1.0f,i.noteHit),
        ImLerp(color4.y,1.0f,i.noteHit),
        ImLerp(color4.z,1.0f,i.noteHit),
        color4.w
      ));
    }
    SDL_FillRect(pianoRollData.surface,&noteRect,color);
  }
  
  rend->updateTexture(pianoRollData.texture,pianoRollData.surface->pixels,pianoRollData.width*4);
  SDL_UnlockSurface(pianoRollData.surface);

  dl->AddImage(
    rend->getTextureID(pianoRollData.texture),
    rect.Min,rect.Max,
    ImVec2(0,0),
    ImVec2(rend->getTextureU(pianoRollData.texture),rend->getTextureV(pianoRollData.texture)));
  if (1) {
    String debugText="piano roll debug";
    debugText+=fmt::sprintf("\nw/h: %d:%d", pianoRollData.width,pianoRollData.height);
    debugText+=fmt::sprintf("\ncmds: %lu", cmdStream.size());
    for (int ch=0; ch<e->getTotalChannelCount(); ch++) {
      RollNote i=pianoRollData.notes[ch];
      debugText+=fmt::sprintf("\nwidth:%f, note: %d, noteVib: %f, notePorta: %f",
      i.width,i.note,i.noteVib,i.notePorta);
      if (i.active) debugText+=fmt::sprintf(" pos: %f, width: %f",
        pianoRollData.width*(float)(i.note+60-rollOff*12)/(rollOct*12)+i.noteVib, i.width);
    }
    dl->AddText(rect.Min,-1,debugText.c_str());
  }
}