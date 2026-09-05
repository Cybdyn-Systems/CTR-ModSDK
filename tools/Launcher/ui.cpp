#include "ui.h"
#include "dataManager.h"
#include "IconsFontAwesome6.h"

#include <misc/cpp/imgui_stdlib.h>
#include <portable-file-dialogs.h>
#include <filesystem>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <iostream>

namespace
{
  // Shared by CalcColumns() (which sizes the label column to the widest of
  // them) and by the rows that draw them.
  constexpr const char* c_usernameLabel = "Username";
  constexpr const char* c_biosLabel = "BIOS Path";
  constexpr const char* c_gameLabel = "Game Path";
  constexpr const char* c_browseLabel = "...";
}

UI::UI()
{
  g_dataManager.BindData(&m_biosPath, DataType::STRING, "BiosPath");
  g_dataManager.BindData(&m_gamePath, DataType::STRING, "GamePath");
  g_dataManager.BindData(&m_version, DataType::STRING, "GameVersion");
  g_dataManager.BindData(&m_username, DataType::STRING, "Username");
  m_updater.CheckForUpdates(m_status, m_version);
}

static int FilterUsernameChar(ImGuiInputTextCallbackData* data)
{
  if (data->EventChar >= 'a' && data->EventChar <= 'z') { return 0; }
  if (data->EventChar >= 'A' && data->EventChar <= 'Z') { return 0; }
  if (data->EventChar >= '0' && data->EventChar <= '9') { return 0; }
  return 1;
}

UI::Columns UI::CalcColumns()
{
  const ImGuiStyle& style = ImGui::GetStyle();
  const float contentRight = ImGui::GetWindowWidth() - style.WindowPadding.x;

  float labelWidth = .0f;
  for (const char* label : {c_usernameLabel, c_biosLabel, c_gameLabel})
  {
    const float width = ImGui::CalcTextSize(label).x;
    if (width > labelWidth) { labelWidth = width; }
  }

  // The tick and the cross are not necessarily the same width, so reserve the
  // wider one and the column never shifts as a path becomes valid.
  float iconWidth = ImGui::CalcTextSize(ICON_FA_CIRCLE_CHECK).x;
  const float crossWidth = ImGui::CalcTextSize(ICON_FA_CIRCLE_XMARK).x;
  if (crossWidth > iconWidth) { iconWidth = crossWidth; }

  const float browseWidth = ImGui::CalcTextSize(c_browseLabel).x + (style.FramePadding.x * 2.0f);

  Columns col;
  col.browse = contentRight - browseWidth;
  col.icon = col.browse - style.ItemSpacing.x - iconWidth;
  col.label = col.icon - style.ItemInnerSpacing.x - labelWidth;
  col.inputWidth = col.label - style.ItemInnerSpacing.x - style.WindowPadding.x;
  // Action buttons fill the whole label/tick/browse band, so they are as wide
  // as that band and flush with the browse buttons above them.
  col.buttonWidth = contentRight - col.label;
  return col;
}

void UI::Render(int width, int height)
{
  ImGui::SetNextWindowPos(ImVec2(.0f, .0f), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(static_cast<float>(width), static_cast<float>(height)), ImGuiCond_Always);
  ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

  const ImGuiStyle& style = ImGui::GetStyle();
  const Columns col = CalcColumns();

  const std::string icon = m_username.empty() ? ICON_FA_CIRCLE_XMARK : ICON_FA_CIRCLE_CHECK;
  ImGui::SetNextItemWidth(col.inputWidth);
  ImGui::InputText("##Username", &m_username, ImGuiInputTextFlags_CallbackCharFilter, FilterUsernameChar);
  ImGui::SameLine(col.label);
  ImGui::TextUnformatted(c_usernameLabel);
  ImGui::SameLine(col.icon);
  ImGui::TextUnformatted(icon.c_str());
  if (m_username.size() > 9) { m_username = m_username.substr(0, 9); }

  static bool readBios = true;
  bool updateReady = true;
  updateReady &= SelectFile(m_biosPath, c_biosLabel, col, {".bin"}, {"PSX Bios File", "*.bin"}, "Path to a PS1 NTSC-U bios.");
  if (updateReady)
  {
    if (readBios)
    {
      if (m_updater.IsValidBios(m_biosPath)) { readBios = false; }
      else { updateReady = false; }
    }
  }
  else { readBios = true; }
  updateReady &= SelectFile(m_gamePath, c_gameLabel, col, {".bin", ".img", ".iso"}, {"Game Files", "*.bin *.img *.iso"}, "Path to the clean NTSC-U version of CTR");

  // Both action rows sit at the bottom of the window: the version line with
  // Update beside it, then the status line with Launch Game beside it.
  const float rowHeight = ImGui::GetFrameHeight();
  const float statusRowY = ImGui::GetWindowHeight() - style.WindowPadding.y - rowHeight;
  const float versionRowY = statusRowY - style.ItemSpacing.y - rowHeight;
  if (versionRowY > ImGui::GetCursorPosY()) { ImGui::SetCursorPosY(versionRowY); }

  ImGui::AlignTextToFramePadding();
  ImGui::TextUnformatted(("Version: " + m_version).c_str());
  ImGui::SameLine(col.label);
  ImGui::BeginDisabled(m_updater.IsBusy() || !updateReady);
  if (ImGui::Button("Update", ImVec2(col.buttonWidth, .0f))) { m_updater.Update(m_status, m_version, m_gamePath, m_biosPath); }
  ImGui::EndDisabled();

  if (statusRowY > ImGui::GetCursorPosY()) { ImGui::SetCursorPosY(statusRowY); }
  if (!m_status.empty())
  {
    // Error strings carry file paths and can be longer than their column, so
    // clip them where the buttons start rather than letting them run under.
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const float rowTop = ImGui::GetCursorPosY();
    ImGui::AlignTextToFramePadding();
    ImGui::PushClipRect(ImVec2(windowPos.x + style.WindowPadding.x, windowPos.y + rowTop),
                        ImVec2(windowPos.x + col.label - style.ItemSpacing.x, windowPos.y + rowTop + rowHeight), true);
    ImGui::TextUnformatted(m_status.c_str());
    ImGui::PopClipRect();
    ImGui::SameLine(col.label);
  }
  else { ImGui::SetCursorPosX(col.label); }

  ImGui::BeginDisabled(m_updater.IsBusy() || !m_updater.IsUpdated());
  if (ImGui::Button("Launch Game", ImVec2(col.buttonWidth, .0f)))
  {
    const std::string s_clientPath = GetClientPath(m_version);
    const std::string s_patchedPath = GetPatchedGamePath(m_version);
    if (!std::filesystem::exists(s_clientPath)) { m_status = "Error: could not find " + s_clientPath; }
    else if (!std::filesystem::exists(s_patchedPath)) { m_status = "Error: could not find " + s_patchedPath; }
    else
    {
      m_status = "Launching game...";
      g_dataManager.SaveData();
      const std::string duckCommand = "start /b \"\" \"" + g_duckExecutable + "\" \"" + s_patchedPath + "\" &";
      std::system(duckCommand.c_str());
      {
        using namespace std;
        std::this_thread::sleep_for(7000ms); //it takes about this long for it to boot to the main menu.
      }
      //if you try to start the client immediately, it may attempt to access shmem immediately (and fail)
      const std::string clientCommand = "start /b \"\" \"" + std::filesystem::current_path().string() + "/" + GetClientPath(m_version) + "\" " + m_username + " &";
      std::system(clientCommand.c_str());
      m_status = "Game launched...";
    }
  }
  ImGui::EndDisabled();

  ImGui::End();
}

bool UI::SelectFile(std::string& str, const std::string& label, const Columns& col, const std::vector<std::string>& ext, const std::vector<std::string>& filters, const std::string& tip)
{

  std::string lowercaseStr;
  for (char c : str)
  {
    if (c <= 'Z' && c >= 'A') { c = c - ('Z' - 'z'); };
    lowercaseStr += c;
  }

  auto checkValidPath = [&]
    {
      if (std::filesystem::exists(str))
      {
        for (const std::string& s : ext)
        {
          if (lowercaseStr.ends_with(s)) { return true; }
        }
      }
      return false;
    };
  std::string icon = checkValidPath() ? ICON_FA_CIRCLE_CHECK : ICON_FA_CIRCLE_XMARK;
  ImGui::SetNextItemWidth(col.inputWidth);
  ImGui::InputText(("##" + label).c_str(), &str);
  if (!tip.empty()) { ImGui::SetItemTooltip("%s", tip.c_str()); }
  ImGui::SameLine(col.label);
  ImGui::TextUnformatted(label.c_str());
  ImGui::SameLine(col.icon);
  ImGui::TextUnformatted(icon.c_str());
  ImGui::SameLine(col.browse);
  if (ImGui::Button((std::string(c_browseLabel) + "##" + label).c_str()))
  {
    auto selection = pfd::open_file(label, str, filters).result();
    if (selection.empty()) { return false; }
    str = selection.front();
  }
  return checkValidPath();
}

bool UI::SelectFolder(std::string& str, const std::string& label, const Columns& col, const std::string& tip)
{
  bool validPath = std::filesystem::is_directory(str);
  std::string icon = validPath ? ICON_FA_CIRCLE_CHECK : ICON_FA_CIRCLE_XMARK;
  ImGui::SetNextItemWidth(col.inputWidth);
  ImGui::InputText(("##" + label).c_str(), &str);
  if (!tip.empty()) { ImGui::SetItemTooltip("%s", tip.c_str()); }
  ImGui::SameLine(col.label);
  ImGui::TextUnformatted(label.c_str());
  ImGui::SameLine(col.icon);
  ImGui::TextUnformatted(icon.c_str());
  ImGui::SameLine(col.browse);
  if (ImGui::Button((std::string(c_browseLabel) + "##" + label).c_str()))
  {
    auto selection = pfd::select_folder(label).result();
    if (selection.empty()) { return false; }
    str = selection;
  }
  return validPath;
}
