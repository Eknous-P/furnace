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

void FurnaceGUI::drawPianoRoll(ImDrawList* dl, ImRect rect) {
  int& rollOff=(e->isPlaying() || pianoSharePosition)?pianoOffset:pianoOffsetEdit;
  int& rollOct=(e->isPlaying() || pianoSharePosition)?pianoOctaves:pianoOctavesEdit;
  if (pianoRollData.updateTex) {
    pianoRollData.width=rollOct*12*pianoRollData.noteWidth;
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
  if (pianoRollData.texture==NULL || pianoRollData.surface==NULL) return;
  SDL_LockSurface(pianoRollData.surface);
  unsigned int* px=(unsigned int*)pianoRollData.surface->pixels;
  memcpy(((unsigned char*)px)+pianoRollData.width*4,px,pianoRollData.width*4*(pianoRollData.height-1));
  memset(px,0,pianoRollData.width*4);

  // the draw code
  // SDL_Rect r={30,30,60,60};
  // SDL_FillRect(pianoRollData.surface,&r,-1);
  // memset(px,-1,pianoRollData.width*pianoRollData.height*4);

    // std::vector<DivCommand> cmdStream;
    if (!fancyPattern)
      e->getCommandStream(cmdStream);
    
    for (DivCommand& i: cmdStream) {
      switch (i.cmd) {
        case DIV_CMD_NOTE_ON:
          pianoRollData.notes[i.chan].active=true;
          pianoRollData.notes[i.chan].note=i.value;
          pianoRollData.notes[i.chan].width=pianoRollData.noteWidth;
          break;
        case DIV_CMD_NOTE_OFF:
          pianoRollData.notes[i.chan].active=false;
          pianoRollData.notes[i.chan].note=0;
          break;
        case DIV_CMD_VOLUME: {
          float scaledVol=(float)i.value/(float)e->getMaxVolumeChan(i.chan);
          if (scaledVol>1.0f) scaledVol=1.0f;
          pianoRollData.notes[i.chan].width=scaledVol*pianoRollData.noteWidth;
          break;
        }

        default: continue;
      }
    }
    SDL_Rect noteRect={0,0,0,1};
    for (int ch=0; ch<e->getTotalChannelCount(); ch++) {
      RollNote i=pianoRollData.notes[ch];
      if (!e->curSubSong->chanShow[ch] || !i.active) {
        ch++;
        continue;
      }
      float x=((float)(i.note+60-rollOff*12)/(rollOct*12))+i.noteFine;
      noteRect.x=(int)round(pianoRollData.width*x-i.width/2.0f);
      noteRect.w=i.width;
      int color=e->curSubSong->chanColor[ch];
      if (color==0) color=ImGui::GetColorU32(uiColors[GUI_COLOR_CHANNEL_FM+e->getChannelType(ch)]);
      SDL_FillRect(pianoRollData.surface,&noteRect,color);
    }
  
  rend->updateTexture(pianoRollData.texture,pianoRollData.surface->pixels,pianoRollData.width*4);
  SDL_UnlockSurface(pianoRollData.surface);

  dl->AddImage(
    rend->getTextureID(pianoRollData.texture),
    rect.Min,rect.Max,
    ImVec2(0,0),
    ImVec2(rend->getTextureU(pianoRollData.texture),rend->getTextureV(pianoRollData.texture)));
  {
    String debugText="piano debug";
    debugText+=fmt::sprintf("\nw/h: %d:%d", pianoRollData.width,pianoRollData.height);
    debugText+=fmt::sprintf("\ncmds: %lu", cmdStream.size());
    for (int ch=0; ch<e->getTotalChannelCount(); ch++) {
      RollNote i=pianoRollData.notes[ch];
      debugText+=fmt::sprintf("\nwidth:%f, note: %d, noteFine: %f, active: %d",
      i.width,i.note,i.noteFine,(int)i.active);
      debugText+=fmt::sprintf(" pos: %f, width: %f",
        pianoRollData.width*(float)(i.note+60-rollOff*12)/(rollOct*12)+i.noteFine, i.width);
    }
    dl->AddText(rect.Min,-1,debugText.c_str());
  }
}