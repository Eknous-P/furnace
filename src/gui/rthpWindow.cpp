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
#include <imgui.h>
#include "rthp.h"
#include "misc/cpp/imgui_stdlib.h"

void FurnaceGUI::drawRTHPWindow(){
  RTHPState=e->getRTHP()->getState();
  if (nextWindow==GUI_WINDOW_RTHP) {
    rthpWindowOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!rthpWindowOpen) return;
  if (ImGui::Begin("Real-time Hardware Playback",&rthpWindowOpen,globalWinFlags)) {
    if (ImGui::BeginCombo("Implementation",e->getRTHP()->getImplName(RTHPImplementation))) { // RTHPDevices[RTHPImplementation].c_str()
      for (int i=0; i<RTHP_IMPL_LEN;i++) {
        if (i==RTHP_NONE) continue;
        if (ImGui::Selectable(e->getRTHP()->getImplName(i))) {
          RTHPImplementation=i;
          e->getRTHP()->preinit(RTHPImplementations(i),RTHPDevice);
        }
      }
      ImGui::EndCombo();
    }
    ImGui::BeginDisabled((RTHPState==0x01));

    if (ImGui::Button("Scan ports") || (RTHPDevices.empty() && (RTHPState==0x01))) {
      RTHPDevices.clear();
      for (int i=0; i<e->getRTHP()->RTHPImpl->scanDevices(); i++) {
        RTHPDevices.push_back(e->getRTHP()->RTHPImpl->getDeviceName());
      }
    }
    ImGui::EndDisabled();
    // ImGui::BeginDisabled(!RTHPState);

    if (ImGui::BeginCombo("Devices",RTHPDevices[RTHPDevice].c_str())) { // 
      for (int i=0;i<RTHPDevices.size();i++) {
        if (ImGui::Selectable(RTHPDevices[i].c_str())) RTHPDevice=i; // 
      }
      ImGui::EndCombo();
    }
    if (ImGui::Button("Init")) {
      e->getRTHP()->RTHPImpl->init();
    }
    // ImGui::EndDisabled();
    // ImGui::BeginDisabled(RTHPState);
    if (ImGui::Button("Disconnect")) {
      stop();
      // rthp->RTHPImpl->deinit();
    }

    // if (dumpedChip>e->song.systemLen-1) {
    //   dumpedChip=e->song.systemLen-1;
    //   rthp->RTHPImpl->setDumpedChip(dumpedChip);
    // }
    // ImGui::EndDisabled();
    ImGui::End();
  }
}
