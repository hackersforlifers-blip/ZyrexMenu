// STL
#include <string>
#include <thread>
#include <chrono>
#include <algorithm>
#include <cstdio>
#include <cstring>

// Main
#include "Menu.h"

// Dear ImGui
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

// Dear ImGui - Backends
#include "imgui/backends/imgui_impl_dx11.h"
#include "imgui/backends/imgui_impl_win32.h"

// Dear ImGui - Misc
#include "imgui/misc/imgui_freetype.h"

// Resources - Fonts
#include "fonts/exo2_medium.h"

// Feature Settings
#include <Console.h>
#include <settings.h>
#include <menu/keybind/keybind.h>
#include <features/config/config.h>


// LOCKED game features
#include <features/football/football.h>
#include <gamesupport/gamesupport.h>
#include <cache/cache.h>
#include <game/game.h>

// Definitions
#define PROJECT_NAME        "Vision"
#define PROJECT_VERSION     "2.0.0"
#define PROJECT_BUILD       __DATE__

bool Menu::Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.IniFilename = nullptr;
    io.Fonts->Clear();

    style.WindowRounding = 7;
    style.ChildRounding = 5;
    style.FrameRounding = 4;
    style.PopupRounding = 3;

    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.FrameBorderSize = 0;
    style.PopupBorderSize = 0;

    style.WindowPadding = ImVec2(16, 16);
    style.ItemSpacing = style.WindowPadding;

    style.ChildPadding = ImVec2(14, 14);
    style.ItemInnerSpacing = ImVec2(8, 6);

    style.FramePadding = ImVec2(12, 12);
    style.CellPadding = ImVec2(4, 2);

    style.ScrollbarSize = 6.0f;
    style.GrabMinSize = 14.0f;
    style.GrabRounding = style.GrabMinSize / 2.0f;

    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);

    style.Colors[ImGuiCol_WindowBg] = ImAdd::HexToColorVec4(0x0a0708);
    style.Colors[ImGuiCol_ChildBg] = ImAdd::HexToColorVec4(0x120f11, 0.6f);
    style.Colors[ImGuiCol_PopupBg] = ImAdd::HexToColorVec4(0x120f11);
    style.Colors[ImGuiCol_TitleBg] = ImAdd::HexToColorVec4(0x120f11);
    style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_TitleBg];
    style.Colors[ImGuiCol_TitleBgCollapsed] = style.Colors[ImGuiCol_TitleBg];

    style.Colors[ImGuiCol_Text] = ImAdd::HexToColorVec4(0xdadada);
    style.Colors[ImGuiCol_TextDisabled] = ImAdd::HexToColorVec4(0x555555, 0.7f);
    style.Colors[ImGuiCol_CheckMark] = ImAdd::HexToColorVec4(0xdadada);

    style.Colors[ImGuiCol_Border] = ImAdd::HexToColorVec4(0x221c1e, 0.75f);
    style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];

    style.Colors[ImGuiCol_Header] = ImAdd::HexToColorVec4(0x57beea);
    style.Colors[ImGuiCol_HeaderHovered] = ImAdd::HexToColorVec4(0x007ac8);
    style.Colors[ImGuiCol_HeaderActive] = ImAdd::HexToColorVec4(0x007ac8);

    style.Colors[ImGuiCol_SliderGrab] = style.Colors[ImGuiCol_Header];
    style.Colors[ImGuiCol_SliderGrabActive] = style.Colors[ImGuiCol_HeaderActive];
    style.Colors[ImGuiCol_TextSelectedBg] = style.Colors[ImGuiCol_Header];
    style.Colors[ImGuiCol_TextSelectedBg].w = 0.4f;

    style.Colors[ImGuiCol_FrameBg] = ImAdd::HexToColorVec4(0x1a1617, 0.9f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImAdd::HexToColorVec4(0x221e1f, 0.9f);
    style.Colors[ImGuiCol_FrameBgActive] = ImAdd::HexToColorVec4(0x151213, 0.9f);

    style.Colors[ImGuiCol_FrameBgShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.45f);
    style.Colors[ImGuiCol_ButtonShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.45f);

    style.Colors[ImGuiCol_Button] = style.Colors[ImGuiCol_FrameBg];
    style.Colors[ImGuiCol_ButtonHovered] = style.Colors[ImGuiCol_FrameBgHovered];
    style.Colors[ImGuiCol_ButtonActive] = style.Colors[ImGuiCol_FrameBgActive];

    style.Colors[ImGuiCol_Tab] = ImAdd::HexToColorVec4(0x0e0c0d);
    style.Colors[ImGuiCol_TabHovered] = ImAdd::HexToColorVec4(0x1a1617);
    style.Colors[ImGuiCol_TabActive] = ImAdd::HexToColorVec4(0x151213);

    ImFontConfig font_cfg_main;
    font_cfg_main.FontLoaderFlags = ImGuiFreeTypeLoaderFlags_ForceAutoHint;
    font_cfg_main.GlyphOffset = ImVec2(0, -1);
    font_cfg_main.SizePixels = 14.0f;
    io.Fonts->AddFontFromMemoryCompressedTTF(exo2_medium_compressed_data, exo2_medium_compressed_size, font_cfg_main.SizePixels, &font_cfg_main, io.Fonts->GetGlyphRangesDefault());

    io.Fonts->Build();

    m_bInitialized = true;
    return true;
}

// ===================== LOCKED GAME: FOOTBALL AUTO-DIVE =====================

static void TabAutoM2()
{
    ImGui::Checkbox("Enable Auto M2", &settings::football::auto_m2);
    ImGui::Text("Keybind");
    ImGui::SameLine();
    keybind::keybind_selector("##auto_m2_key", &settings::football::auto_m2_key, &settings::football::auto_m2_key_mode);
    ImGui::Spacing();

    ImGui::SeparatorText("Offsets");
    ImGui::Text("M2 Dive Offset (Normal)");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##m2_dive_off", &settings::football::auto_m2_dive_offset, 0.0f, 2.0f, "%.2f");
    ImGui::Text("M2 Dive Offset (Mode B)");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##m2_dive_off_b", &settings::football::auto_m2_mode_b_dive_offset, 0.0f, 2.0f, "%.2f");
    ImGui::Text("M2 Cooldown");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##m2_cooldown", &settings::football::auto_m2_cooldown, 0.0f, 2.0f, "%.2f");
    ImGui::Text("M2 Jump Delay (TOP zones)");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##m2_jump_delay", &settings::football::m2_jump_delay, 0.0f, 0.15f, "%.3f");
    ImGui::Spacing();

    ImGui::SeparatorText("Jump Velocity Boost");
    ImGui::Checkbox("Enable##jvb", &settings::movement::jump_velocity_boost::enabled);
    ImGui::Text("Jump Power");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##jvb_boost", &settings::movement::jump_velocity_boost::jp_boost, 1.0f, 200.0f, "%.0f");
    ImGui::Text("Keybind");
    ImGui::SameLine();
    keybind::keybind_selector("##jvb_key", &settings::movement::jump_velocity_boost::keybind,
        &settings::movement::jump_velocity_boost::activation_mode);
}

static void TabGK()
{
    ImGui::Checkbox("Enable Autodive", &settings::football::autodive_enabled);
    ImGui::Text("Keybind");
    ImGui::SameLine();
    keybind::keybind_selector("##autodive_key", &settings::football::autodive_key, &settings::football::autodive_key_mode);
    ImGui::Spacing();

    ImGui::SeparatorText("Offsets");
    ImGui::Text("Mode A Dive Offset");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##mode_a_dive_off", &settings::football::mode_a_dive_offset, 0.0f, 2.0f, "%.2f");
    ImGui::Text("Mode B Dive Offset");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##mode_b_dive_off", &settings::football::mode_b_dive_offset, 0.0f, 2.0f, "%.2f");
    ImGui::Text("Dive Cooldown");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##dive_cooldown", &settings::football::dive_cooldown, 0.0f, 2.0f, "%.2f");
    ImGui::Spacing();

    ImGui::SeparatorText("Mode B");
    ImGui::Checkbox("Effect Trigger", &settings::football::mode_b_enabled);
    if (settings::football::mode_b_enabled)
    {
        ImGui::Text("Duration");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::SliderFloat("##mode_b_duration", &settings::football::mode_b_duration, 0.1f, 3.0f, "%.1fs");
    }
    ImGui::Checkbox("Force Mode B Active", &settings::football::force_mode_b_active);
    ImGui::Spacing();

    ImGui::SeparatorText("Dive Keys");
    ImGui::Text("Top / Jump");
    ImGui::SameLine();
    keybind::keybind_selector("##dive_key_space", &settings::football::dive_key_space);
    ImGui::Text("Left");
    ImGui::SameLine();
    keybind::keybind_selector("##dive_key_left", &settings::football::dive_key_left);
    ImGui::Text("Right");
    ImGui::SameLine();
    keybind::keybind_selector("##dive_key_right", &settings::football::dive_key_right);
    ImGui::Text("Middle");
    ImGui::SameLine();
    keybind::keybind_selector("##dive_key_middle", &settings::football::dive_key_middle);
    ImGui::Spacing();

    ImGui::SeparatorText("Random Dive");
    ImGui::Checkbox("Enabled##random_dive", &settings::football::random_dive_enabled);
    ImGui::Text("Offset Reduction");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##random_dive_off_red", &settings::football::random_dive_offset_reduction, 0.0f, 1.0f, "%.2f");
    ImGui::Spacing();

    ImGui::SeparatorText("Thresholds");
    ImGui::Text("Mode A Top Y");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##mode_a_top", &settings::football::mode_a_top_threshold, 0.0f, 60.0f, "%.1f");
    ImGui::Text("Mode B Top Y");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##mode_b_top", &settings::football::mode_b_top_threshold, 0.0f, 60.0f, "%.1f");
    ImGui::Text("Middle Top Y");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##middle_top", &settings::football::mid_top_threshold, 0.0f, 60.0f, "%.1f");
    ImGui::Text("Mid Iframe Top Y");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##mid_iframe_top", &settings::football::mid_iframe_top_threshold, 0.0f, 60.0f, "%.1f");

    std::string mode_info = settings::football::mode_b_active ? "MODE B" : "MODE A";
    ImGui::Text("[%s] Ball: %s", mode_info.c_str(), settings::football::status_text.c_str());
    if (settings::football::is_tracking)
        ImGui::Text("Zone: %s | %.2fs", settings::football::current_zone.c_str(), settings::football::predicted_time);
}

static void TabZones()
{
    ImGui::SeparatorText("Zone Fractions");
    ImGui::Text("Middle Width");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##mid_frac", &settings::football::mid_fraction, 0.05f, 0.9f, "%.2f");
    ImGui::Text("Mid-Side Width");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##mid_side_frac", &settings::football::mid_side_fraction, 0.05f, 0.95f, "%.2f");
    ImGui::Spacing();

    ImGui::SeparatorText("Post Guard");
    ImGui::Checkbox("Enabled", &settings::football::post_guard_enabled);
    if (settings::football::post_guard_enabled)
    {
        ImGui::Text("Guard Distance");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::SliderFloat("##guard_dist", &settings::football::post_guard_distance, 1.0f, 30.0f, "%.1f");
        ImGui::Text("Offset Bonus");
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::SliderFloat("##offset_bonus", &settings::football::post_guard_offset_bonus, 0.0f, 0.3f, "%.2f");
    }
}

static void TabPhysics()
{
    ImGui::Text("Gravity Mult");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##grav_mult", &settings::football::gravity_mult, 1.0f, 20.0f, "%.1f");
    ImGui::Text("Bounce Friction");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##bounce_fric", &settings::football::bounce_friction, 0.1f, 1.0f, "%.2f");
    ImGui::Text("Bounce Vel Y");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##bounce_vel_y", &settings::football::bounce_vel_y, 0.1f, 1.0f, "%.2f");
    ImGui::Text("Rolling Friction");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##roll_fric", &settings::football::rolling_friction, 0.9f, 1.0f, "%.3f");
}

static void TabDisplay()
{
    ImGui::Checkbox("Show Status", &settings::football::show_prediction);
    ImGui::Checkbox("Show Path", &settings::football::show_path);
    ImGui::Checkbox("Show Panel", &settings::football::show_panel);
    ImGui::Spacing();

    ImGui::SeparatorText("Panel Position");
    ImGui::Text("Behind Dist");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##behind_dist", &settings::football::panel_behind_dist, -5.0f, 20.0f, "%.1f");
    ImGui::Text("Height Adj");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##height_adj", &settings::football::panel_height_adj, -5.0f, 5.0f, "%.1f");
    ImGui::Text("Zone Scale X");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##zone_scale_x", &settings::football::zone_scale_x, 0.1f, 5.0f, "%.1f");
    ImGui::Text("Zone Scale Y");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("##zone_scale_y", &settings::football::zone_scale_y, 0.1f, 5.0f, "%.1f");
    ImGui::Spacing();

    ImGui::SeparatorText("Performance");
    ImGui::Text("Frame Limiter (ms)");
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderInt("##frame_limiter", &settings::menu::frame_limiter_ms, 0, 33, "%d ms");
    if (ImGui::IsItemHovered())
        ImGui::SetTooltip("0 = uncapped, 8 = ~120fps, 4 = ~250fps, 16 = ~60fps");
}

static void TabKeybinds()
{
    ImGui::SeparatorText("Menu");
    ImGui::Text("Menu Toggle");
    ImGui::SameLine();
    keybind::keybind_selector("##menu_keybind", &settings::menu::menu_keybind);
    ImGui::Spacing();

    ImGui::SeparatorText("Football");
    ImGui::Text("Auto GK");
    ImGui::SameLine();
    keybind::keybind_selector("##autodive_key", &settings::football::autodive_key, &settings::football::autodive_key_mode);
    ImGui::Text("Auto M2");
    ImGui::SameLine();
    keybind::keybind_selector("##auto_m2_key", &settings::football::auto_m2_key, &settings::football::auto_m2_key_mode);
    ImGui::Text("Panel Visibility");
    ImGui::SameLine();
    keybind::keybind_selector("##panel_vis_key", &settings::football::panel_vis_key);
    ImGui::Spacing();

    ImGui::SeparatorText("Dive Keys");
    ImGui::Text("Top/Jump");
    ImGui::SameLine();
    keybind::keybind_selector("##dive_key_space", &settings::football::dive_key_space);
    ImGui::Text("Left");
    ImGui::SameLine();
    keybind::keybind_selector("##dive_key_left", &settings::football::dive_key_left);
    ImGui::Text("Right");
    ImGui::SameLine();
    keybind::keybind_selector("##dive_key_right", &settings::football::dive_key_right);
    ImGui::Text("Middle");
    ImGui::SameLine();
    keybind::keybind_selector("##dive_key_middle", &settings::football::dive_key_middle);
    ImGui::Spacing();

}

static void TabLog()
{
    static ImGuiTextFilter filter;
    static bool autoscroll = true;

    if (ImGui::SmallButton("Clear")) {
        Console::Clear();
    }
    ImGui::SameLine();
    if (ImGui::SmallButton("Copy")) {
        auto logs = Console::GetLogs();
        std::string all;
        for (auto& entry : logs)
            all += entry.text + "\n";
        if (!all.empty())
            ImGui::SetClipboardText(all.c_str());
    }
    ImGui::SameLine();
    filter.Draw("Filter", 180);

    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &autoscroll);

    ImGui::Spacing();
    ImGui::BeginChild("##log_scroll", ImVec2(0, 0), ImGuiChildFlags_Borders);

    auto logs = Console::GetLogs();
    for (auto& entry : logs) {
        if (!filter.PassFilter(entry.text.c_str()))
            continue;

        ImVec4 color;
        switch (entry.level) {
        case LogLevel::Debug: color = ImVec4(0.6f, 0.6f, 0.6f, 1); break;
        case LogLevel::Info:  color = ImVec4(0.7f, 1.0f, 0.7f, 1); break;
        case LogLevel::Warn:  color = ImVec4(1.0f, 1.0f, 0.5f, 1); break;
        case LogLevel::Error: color = ImVec4(1.0f, 0.4f, 0.4f, 1); break;
        }
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(entry.text.c_str());
        ImGui::PopStyleColor();
    }

    if (autoscroll)
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
}

static void TabFootballLocked()
{
    if (ImGui::BeginTabBar("FootballTabs", ImGuiTabBarFlags_NoTooltip))
    {
        if (ImGui::BeginTabItem("Auto M2"))
        {
            TabAutoM2();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("GK"))
        {
            TabGK();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Zones"))
        {
            TabZones();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Physics"))
        {
            TabPhysics();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Display"))
        {
            TabDisplay();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Keybinds"))
        {
            TabKeybinds();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

// ===================== MAIN MENU DRAW =====================

void Menu::DrawWatermark()
{
    if (!m_bInitialized || !settings::menu::watermark) return;
    if (game::datamodel.address == 0) return;
    ImGuiIO& io = ImGui::GetIO();
    std::string wm = std::string(PROJECT_NAME) + " v" PROJECT_VERSION;
    if (settings::watermark::show_fps)
        wm += " | " + std::to_string((int)io.Framerate) + " FPS";
    ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
    float tw = ImGui::CalcTextSize(wm.c_str()).x;
    ImGui::SetNextWindowSize(ImVec2(tw + 30, 30), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::Begin("", (bool*)0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
    ImGui::GetWindowDrawList()->AddText(ImGui::GetWindowPos() + ImVec2(10, 8), col, wm.c_str());
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void Menu::DrawMenu()
{
    if (!m_bInitialized || !m_bMenuVisible) return;

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::SetNextWindowSize(ImVec2(880, 600), ImGuiCond_Once);
    ImGui::SetNextWindowSizeConstraints(ImVec2(720, 400), ImVec2(FLT_MAX, FLT_MAX));
    ImGui::SetNextWindowPos(io.DisplaySize / 2, ImGuiCond_Once, ImVec2(0.5f, 0.5f));

    if (ImGui::Begin(PROJECT_NAME, &m_bMenuVisible))
    {
        ImGui::BeginChild("##log_sidebar", ImVec2(280, 0), ImGuiChildFlags_Borders);
        ImGui::Checkbox("Streamproof", &settings::menu::streamproof);
        ImGui::Separator();
        TabLog();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##main_content", ImVec2(0, 0));
        ImGui::PushItemWidth(-1.0f);
        TabFootballLocked();
        ImGui::PopItemWidth();
        ImGui::EndChild();
    }
    ImGui::End();
}

void Menu::Shutdown()
{
    if (!m_bInitialized) return;

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_bInitialized = false;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool Menu::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!m_bInitialized) return false;
    return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}
