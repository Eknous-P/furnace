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
#include "misc/cpp/imgui_stdlib.h"
#include "IconsFontAwesome4.h"
#include <imgui.h>

void FurnaceGUI::drawChannels() {
  if (nextWindow==GUI_WINDOW_CHANNELS) {
    channelsOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!channelsOpen) return;
  if (mobileUI) {
    patWindowPos=(portrait?ImVec2(0.0f,(mobileMenuPos*-0.65*canvasH)):ImVec2((0.16*canvasH)+0.5*canvasW*mobileMenuPos,0.0f));
    patWindowSize=(portrait?ImVec2(canvasW,canvasH-(0.16*canvasW)):ImVec2(canvasW-(0.16*canvasH),canvasH));
    ImGui::SetNextWindowPos(patWindowPos);
    ImGui::SetNextWindowSize(patWindowSize);
  } else {
    //ImGui::SetNextWindowSizeConstraints(ImVec2(440.0f*dpiScale,400.0f*dpiScale),ImVec2(canvasW,canvasH));
  }
  if (ImGui::Begin("Channels",&channelsOpen,globalWinFlags,_("Channels"))) {
    if (ImGui::BeginTable("ChannelList",7)) {
      ImGui::TableSetupColumn("c0",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed,0.0);
      ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthStretch,0.0);
      ImGui::TableSetupColumn("c5",ImGuiTableColumnFlags_WidthFixed,48.0f*dpiScale);
      ImGui::TableSetupColumn("c6",ImGuiTableColumnFlags_WidthFixed,0.0f);
      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::Text(_("All"));
      ImGui::TableNextColumn();
      ImGui::Text(_("Pat"));
      ImGui::TableNextColumn();
      ImGui::Text(_("Osc"));
      ImGui::TableNextColumn();
      ImGui::Text(_("Swap"));
      ImGui::TableNextColumn();
      ImGui::Text(_("Name"));
      ImGui::TableNextColumn();
      ImGui::TableNextColumn();
      ImGui::Text(_("Color"));
      for (int i=0; i<e->getTotalChannelCount(); i++) {
        ImGui::PushID(i);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        bool chanShowAll = e->curSubSong->chanShow[i] || e->curSubSong->chanShowChanOsc[i];
        bool chanShowIndeterminate = e->curSubSong->chanShow[i] ^ e->curSubSong->chanShowChanOsc[i];
        ImGui::PushItemFlag(ImGuiItemFlags_MixedValue, chanShowIndeterminate);
        if (ImGui::Checkbox("##VisibleAll",&chanShowAll)) {
          e->curSubSong->chanShow[i] = chanShowAll;
          e->curSubSong->chanShowChanOsc[i] = chanShowAll;
          MARK_MODIFIED;
        }
        ImGui::PopItemFlag();
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##VisiblePat",&e->curSubSong->chanShow[i])) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Show in pattern"));
        }
        ImGui::TableNextColumn();
        if (ImGui::Checkbox("##VisibleChanOsc",&e->curSubSong->chanShowChanOsc[i])) {
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("Show in per-channel oscilloscope"));
        }
        ImGui::TableNextColumn();
        if (ImGui::Button(ICON_FA_ARROWS)) {
        }
        if (ImGui::BeginDragDropSource()) {
          chanToMove=i;
          ImGui::SetDragDropPayload("FUR_CHAN",NULL,0,ImGuiCond_Once);
          ImGui::Button(ICON_FA_ARROWS "##ChanDrag");
          ImGui::EndDragDropSource();
        } else if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip(_("%s #%d\n(drag to swap channels)"),e->getSystemName(e->sysOfChan[i]),e->dispatchChanOfChan[i]);
        }
        if (ImGui::BeginDragDropTarget()) {
          const ImGuiPayload* dragItem=ImGui::AcceptDragDropPayload("FUR_CHAN");
          if (dragItem!=NULL) {
            if (dragItem->IsDataType("FUR_CHAN")) {
              if (chanToMove!=i && chanToMove>=0) {
                e->swapChannelsP(chanToMove,i);
                MARK_MODIFIED;
              }
              chanToMove=-1;
            }
          }
          ImGui::EndDragDropTarget();
        }
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputTextWithHint("##ChanName",e->getChannelName(i),&e->curSubSong->chanName[i])) {
          MARK_MODIFIED;
        }
        ImGui::TableNextColumn();
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputTextWithHint("##ChanShortName",e->getChannelShortName(i),&e->curSubSong->chanShortName[i])) {
          MARK_MODIFIED;
        }
        ImGui::TableNextColumn();
        ImVec4 curColor=e->curSubSong->chanColor[i]?ImGui::ColorConvertU32ToFloat4(e->curSubSong->chanColor[i]):uiColors[GUI_COLOR_CHANNEL_FM+e->getChannelType(i)];
        ImGui::ColorButton("##ChanColor",curColor);
        if (ImGui::BeginPopupContextItem("##ChanColorEditPopup", ImGuiPopupFlags_MouseButtonLeft)) {
          ImGui::ColorPicker4("##ChanColorEdit", (float*)&curColor);
          e->curSubSong->chanColor[i]=ImGui::ColorConvertFloat4ToU32(curColor);
          MARK_MODIFIED;
          ImGui::EndPopup();
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(e->curSubSong->chanColor[i]==0);
        if (ImGui::Button(ICON_FA_REFRESH)) {
          e->curSubSong->chanColor[i]=0;
          MARK_MODIFIED;
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("reset color");
        }
        ImGui::EndDisabled();
        ImGui::PopID();
      }
      ImGui::EndTable();
    }
  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_CHANNELS;
  ImGui::End();
}
