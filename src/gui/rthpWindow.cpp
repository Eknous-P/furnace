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
#include "../ta-log.h"
#include <imgui.h>
#include "rthp.h"

void FurnaceGUI::drawRTHPWindow(){
  rthp->setImpl(RTHP_ERTHP);
  RTHPInitialized=rthp->getRTHPState();
  if (nextWindow==GUI_WINDOW_RTHP) {
    rthpWindowOpen=true;
    ImGui::SetNextWindowFocus();
    nextWindow=GUI_WINDOW_NOTHING;
  }
  if (!rthpWindowOpen) return;
  if (ImGui::Begin("Real-time Hardware Playback",&rthpWindowOpen,globalWinFlags)) {
    ImGui::BeginDisabled(RTHPInitialized);
    if (ImGui::Button("Scan ports") || RTHPAvailPorts.empty()) {
      rthp->scanAvailPorts();
      RTHPAvailPorts=rthp->getAvailPortNames();
    }

    if (ImGui::BeginCombo("ports",RTHPPort.c_str())) {
      for (String i:RTHPAvailPorts) {
        if (ImGui::Selectable(i.c_str())) RTHPPort=i;
      }
      ImGui::EndCombo();
    }
    if (ImGui::Button("Init")) rthp->init(RTHP_ERTHP,RTHPPort);
    ImGui::EndDisabled();
    ImGui::BeginDisabled(!RTHPInitialized);
    if (ImGui::Button("Disconnect")) {
      rthp->deinit();
    }
    ImGui::EndDisabled();
    ImGui::End();
  }
}