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

#include "gui.h"
#include "guiConst.h"
#include "imgui.h"
#include "IconsFontAwesome4.h"
#include <fmt/printf.h>

#define chipName(c) getSystemName(e->song.system[c])

void FurnaceGUI::drawRthpControl() {
  if (nextWindow==GUI_WINDOW_RTHP_CONTROL) {
    rthpControlOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!rthpControlOpen) return;
  if (ImGui::Begin("RTHP Control",&rthpControlOpen,globalWinFlags,"RTHP Control")) {
    ImGui::BeginDisabled(rthp->isRunning());
    if (ImGui::BeginCombo("implementation",rthpImplementationNames[currentImpl+1])) {
      for (int i=0; i<RTHP_IMPL_MAX; i++) {
        if (ImGui::Selectable(rthpImplementationNames[i+1],currentImpl==i)) {
          currentImpl = RTHPImplementations(i);
          rthp->reset();
          rthp->setup(currentImpl);
        }
      }
      ImGui::EndCombo();
    }
    ImGui::EndDisabled();

    if (rthp->isSet()) {
      if (ImGui::TreeNode("Information")) {
        
        ImGui::Text(
          "name: %s\n"
          "description: %s\n",
          rthp->getImplInfo().name,rthp->getImplInfo().description
        );
        if (ImGui::TreeNode("more...")) {
          ImGui::Text("flags...");
          ImGui::Indent();
          for (unsigned char i=0; i<11; i++) {
            // ImGui::Text("%s %s",
            // ((rthp->getImplInfo().flags>>i)&1)>0?ICON_FA_CHECK:ICON_FA_TIMES,
            // rthpImplFlagNames[i]);
            bool flagValue = ((rthp->getImplInfo().flags>>i)&1);
            if (flagValue) {
              ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(uiColors[GUI_COLOR_TOGGLE_ON]));
              ImGui::Text(ICON_FA_CHECK);
            } else {
              ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(uiColors[GUI_COLOR_ERROR]));
              ImGui::Text(ICON_FA_TIMES);
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::Text("%s",rthpImplFlagNames[i]);
          }
          ImGui::TreePop();
          ImGui::Unindent();
          ImGui::Text("supported chips");
          ImGui::Indent();
          for (DivSystem s:rthp->getImplInfo().chipWhitelist) {
            ImGui::Text("%s",getSystemName(s));
          }
          ImGui::Unindent();
        }
        ImGui::TreePop();
      }
    }
    ImGui::BeginDisabled(currentImpl<0 || rthp->isRunning());
    if (ImGui::InputInt("rate",&rthpRate)) {
      if (rthpRate<0) rthpRate=0;
    }
    if (ImGui::InputInt("timeout",&rthpTimeout)) {
      if (rthpTimeout<0) rthpTimeout=0;
    }
    std::vector<RTHPDevice> devs = rthp->getDevices();
    if (devs.size()>0) {
      if (ImGui::BeginCombo("device",devs[currentRTHPDevice].name)) {
        for (unsigned long int i=0; i<devs.size(); i++) {
          if (ImGui::Selectable(devs[i].name, currentRTHPDevice==i)) {
            currentRTHPDevice = i;
          }
        }
        ImGui::EndCombo();
      }
      ImGui::SameLine();
    }
    if (rthp->isSet()) {
      if (ImGui::Button(devs.size()>0?"rescan":"scan devices")) rthp->scanDevices();
    }
    if (!rthp->isRunning()) {
      if (ImGui::Button("init")) {
        rthp->init(currentRTHPDevice,rthpRate,rthpTimeout);
        rthp->scanWhitelist(&(e->song),dumpedChip);
        rthp->sendInfo(&(e->song));
      }
    }
    ImGui::EndDisabled();
    if (rthp->isRunning()) {
      if ((rthp->getImplInfo().flags&RTHPIMPLFLAGS_MULTICHIP)==0) {
        if (ImGui::BeginCombo("chip",
          fmt::sprintf("%d: %s",dumpedChip,chipName(dumpedChip)).c_str()
        )) {
          for (int i=0; i<e->song.systemLen; i++) {
            if (ImGui::Selectable(
              fmt::sprintf("%d: %s",i,chipName(i)).c_str(),
              dumpedChip==i)
            ) {
              dumpedChip=i;
              rthp->scanWhitelist(&(e->song),dumpedChip);
              rthp->setDumpedChip(dumpedChip);
            }
          }
          ImGui::EndCombo();
        }
        if (!rthp->canDumpChip()) {
          ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(uiColors[GUI_COLOR_ERROR]));
          ImGui::Text("this chip is not supported by %s",rthp->getImplInfo().name);
          ImGui::PopStyleColor();
        } else {
          if (e->song.system[dumpedChip] == DIV_SYSTEM_PCSPKR) {
          ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetColorU32(uiColors[GUI_COLOR_WARNING]));
          ImGui::Text("PC Speaker is a special case: go to chip manager and set speaker type to \"use system beeper\" ");
          ImGui::PopStyleColor();
          }
        }
      }
      rthp->setPacketType(RTHP_PACKET_SHORT);
    }
    if (ImGui::Button("reset")) {
      rthp->reset();
      currentImpl=RTHP_IMPL_NONE;
    }

  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_RTHP_CONTROL;
  ImGui::End();
}
