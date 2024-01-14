/**
 * Furnace Tracker - multi-system chiptune tracker
 * Copyright (C) 2021-2023 tildearrow and contributors
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

void FurnaceGUI::drawRTHPWindow(){
  rthp->setImpl(RTHP_ERTHP);
  RTHPInitialized=rthp->getRTHPState();
  dumpedChip=rthp->getDumpedChip();
  if (nextWindow==GUI_WINDOW_RTHP) {
    rthpWindowOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!rthpWindowOpen) return;
  if (ImGui::Begin("Real-time Hardware Playback",&rthpWindowOpen,globalWinFlags)) {
    if (ImGui::BeginCombo("implementation",RTHPImplementationNames[RTHPImplementation])) {
      for (int i=0; i<2;i++) {
        if (i==RTHP_NONE) continue;
        if (ImGui::Selectable(RTHPImplementationNames[i])) RTHPImplementation=i;
      }
      ImGui::EndCombo();
    }
    if (ImGui::Button("Scan ports") || RTHPAvailPorts.empty()) {
      rthp->scanAvailPorts();
      RTHPAvailPorts=rthp->getAvailPortNames();
    }
    ImGui::BeginDisabled(RTHPInitialized);

    if (ImGui::BeginCombo("ports",RTHPPort.c_str())) {
      for (String i:RTHPAvailPorts) {
        if (ImGui::Selectable(i.c_str())) RTHPPort=i;
      }
      ImGui::EndCombo();
    }
    if (ImGui::Button("Init")) rthp->init(RTHPImplementations(RTHPImplementation),RTHPPort);
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!RTHPInitialized);
    if (ImGui::Button("Disconnect")) {
      rthp->deinit();
    }
    if (ImGui::BeginCombo("chip to dump",e->getSystemName(e->song.system[dumpedChip]))) {
      for (int i=0;i<e->song.systemLen;i++) {
        if (ImGui::Selectable(e->getSystemName(e->song.system[i]))) rthp->setDumpedChip(i);
      }
      ImGui::EndCombo();
    }
    ImGui::EndDisabled();
    ImGui::End();
  }
}
