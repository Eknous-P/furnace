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
      ImGui::Text(
        "INFORMATION:\n"
        "name: %s\n"
        "description: %s\n",
        rthp->getImplInfo().name,rthp->getImplInfo().description
      );
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
    }
    if (ImGui::Button("init")) rthp->init(currentRTHPDevice,rthpRate,rthpTimeout);
    ImGui::EndDisabled();

  }
  if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) curWindow=GUI_WINDOW_RTHP_CONTROL;
  ImGui::End();
}
