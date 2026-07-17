#include "menu.h"
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_dx11.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/misc/imgui_freetype.h>
#include <settings.h>
#include <cache/cache.h>
#include <game/game.h>
#include <features/football/football.h>
#include <menu/keybind/keybind.h>
#include <cstdio>
#include <cstring>
#include <cmath>

#define PROJECT_NAME	external_config::cheat_name.c_str()

static const float WIN_W = 700.0f;
static const float WIN_H = 600.0f;
static const float CORNER_R = 7.0f;

static ImU32 GetAccent()
{
	ImVec4 ac;
	switch (settings::ui::accent_color)
	{
	case 0: ac = ImVec4(87 / 255.f, 190 / 255.f, 234 / 255.f, 1.f); break;
	case 1: ac = ImVec4(60 / 255.f, 140 / 255.f, 255 / 255.f, 1.f); break;
	case 2: ac = ImVec4(240 / 255.f, 70 / 255.f, 70 / 255.f, 1.f); break;
	case 3: ac = ImVec4(40 / 255.f, 200 / 255.f, 100 / 255.f, 1.f); break;
	case 4: ac = ImVec4(250 / 255.f, 200 / 255.f, 50 / 255.f, 1.f); break;
	case 5: ac = ImVec4(240 / 255.f, 110 / 255.f, 180 / 255.f, 1.f); break;
	case 6: ac = ImVec4(30 / 255.f, 190 / 255.f, 180 / 255.f, 1.f); break;
	case 7:
	{
		float* cv = settings::ui::custom_accent_color;
		ac = ImVec4(cv[0], cv[1], cv[2], cv[3]); break;
	}
	default: ac = ImVec4(87 / 255.f, 190 / 255.f, 234 / 255.f, 1.f);
	}
	return IM_COL32(int(ac.x * 255), int(ac.y * 255), int(ac.z * 255), int(ac.w * 255));
}

static ImVec4 GetAccentV4()
{
	ImU32 c = GetAccent();
	return ImVec4(((c >> 24) & 0xFF) / 255.0f, ((c >> 16) & 0xFF) / 255.0f, ((c >> 8) & 0xFF) / 255.0f, (c & 0xFF) / 255.0f);
}

bool Menu::Initialize(HWND hWnd, ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;

	ImGui::StyleColorsDark();

	style.WindowRounding = CORNER_R;
	style.ChildRounding = 5;
	style.FrameRounding = 4;
	style.PopupRounding = 3;

	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.FrameBorderSize = 0;
	style.PopupBorderSize = 0;

	style.WindowPadding = ImVec2(16, 16);
	style.ChildPadding = ImVec2(14, 14);
	style.FramePadding = ImVec2(12, 12);
	style.ItemSpacing = ImVec2(16, 16);
	style.ItemInnerSpacing = ImVec2(8, 6);
	style.CellPadding = ImVec2(4, 2);

	style.ScrollbarSize = 6.0f;
	style.GrabMinSize = 14.0f;
	style.GrabRounding = style.GrabMinSize / 2.0f;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);

	ImVec4 ac = GetAccentV4();

	style.Colors[ImGuiCol_WindowBg]             = ImAdd::HexToColorVec4(0x0a0708);
	style.Colors[ImGuiCol_ChildBg]              = ImAdd::HexToColorVec4(0x120f11, 0.6f);
	style.Colors[ImGuiCol_PopupBg]              = ImAdd::HexToColorVec4(0x120f11);
	style.Colors[ImGuiCol_TitleBg]              = ImAdd::HexToColorVec4(0x120f11);
	style.Colors[ImGuiCol_TitleBgActive]        = style.Colors[ImGuiCol_TitleBg];
	style.Colors[ImGuiCol_TitleBgCollapsed]     = style.Colors[ImGuiCol_TitleBg];

	style.Colors[ImGuiCol_Text]                 = ImAdd::HexToColorVec4(0xdadada);
	style.Colors[ImGuiCol_TextDisabled]         = ImAdd::HexToColorVec4(0x555555, 0.7f);
	style.Colors[ImGuiCol_CheckMark]            = ImAdd::HexToColorVec4(0xdadada);

	style.Colors[ImGuiCol_Border]               = ImAdd::HexToColorVec4(0x221c1e, 0.75f);
	style.Colors[ImGuiCol_Separator]            = style.Colors[ImGuiCol_Border];

	style.Colors[ImGuiCol_Header]               = ImVec4(ac.x, ac.y, ac.z, ac.w);
	style.Colors[ImGuiCol_HeaderHovered]        = ImVec4(ac.x * 0.8f, ac.y * 0.8f, ac.z * 0.8f, ac.w);
	style.Colors[ImGuiCol_HeaderActive]         = ImVec4(ac.x * 0.7f, ac.y * 0.7f, ac.z * 0.7f, ac.w);
	style.Colors[ImGuiCol_SliderGrab]           = style.Colors[ImGuiCol_Header];
	style.Colors[ImGuiCol_SliderGrabActive]     = style.Colors[ImGuiCol_HeaderActive];
	style.Colors[ImGuiCol_TextSelectedBg]       = style.Colors[ImGuiCol_Header];
	style.Colors[ImGuiCol_TextSelectedBg].w     = 0.4f;

	style.Colors[ImGuiCol_FrameBg]              = ImAdd::HexToColorVec4(0x1a1617, 0.9f);
	style.Colors[ImGuiCol_FrameBgHovered]       = ImAdd::HexToColorVec4(0x221e1f, 0.9f);
	style.Colors[ImGuiCol_FrameBgActive]        = ImAdd::HexToColorVec4(0x151213, 0.9f);

	style.Colors[ImGuiCol_FrameBgShadow]        = ImVec4(0.00f, 0.00f, 0.00f, 0.45f);
	style.Colors[ImGuiCol_ButtonShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 0.45f);

	style.Colors[ImGuiCol_Button]               = style.Colors[ImGuiCol_FrameBg];
	style.Colors[ImGuiCol_ButtonHovered]        = style.Colors[ImGuiCol_FrameBgHovered];
	style.Colors[ImGuiCol_ButtonActive]         = style.Colors[ImGuiCol_FrameBgActive];

	style.Colors[ImGuiCol_Tab]                  = ImAdd::HexToColorVec4(0x0e0c0d);
	style.Colors[ImGuiCol_TabHovered]           = ImAdd::HexToColorVec4(0x1a1617);
	style.Colors[ImGuiCol_TabActive]            = ImAdd::HexToColorVec4(0x151213);

	style.Colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_ScrollbarGrab]        = ImAdd::HexToColorVec4(0x222222);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImAdd::HexToColorVec4(0x333333);
	style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImAdd::HexToColorVec4(0x444444);

	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(pDevice, pDeviceContext);

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
	ImGui::SliderFloat("M2 Dive Offset (Normal)", &settings::football::auto_m2_dive_offset, 0.0f, 2.0f, "%.2f");
	ImGui::SliderFloat("M2 Dive Offset (Mode B)", &settings::football::auto_m2_mode_b_dive_offset, 0.0f, 2.0f, "%.2f");
	ImGui::SliderFloat("M2 Cooldown", &settings::football::auto_m2_cooldown, 0.0f, 2.0f, "%.2f");
	ImGui::SliderFloat("M2 Jump Delay (TOP zones)", &settings::football::m2_jump_delay, 0.0f, 0.15f, "%.3f");
}

static void TabAutoGK()
{
	ImGui::Checkbox("Enable Autodive", &settings::football::autodive_enabled);
	ImGui::Text("Keybind");
	ImGui::SameLine();
	keybind::keybind_selector("##autodive_key", &settings::football::autodive_key, &settings::football::autodive_key_mode);
	ImGui::Spacing();

	ImGui::SeparatorText("Offsets");
	ImGui::SliderFloat("Mode A Dive Offset", &settings::football::mode_a_dive_offset, 0.0f, 2.0f, "%.2f");
	ImGui::SliderFloat("Mode B Dive Offset", &settings::football::mode_b_dive_offset, 0.0f, 2.0f, "%.2f");
	ImGui::SliderFloat("Dive Cooldown", &settings::football::dive_cooldown, 0.0f, 2.0f, "%.2f");
	ImGui::Spacing();

	ImGui::SeparatorText("Mode B");
	ImGui::Checkbox("Effect Trigger", &settings::football::mode_b_enabled);
	if (settings::football::mode_b_enabled)
		ImGui::SliderFloat("Duration", &settings::football::mode_b_duration, 0.1f, 3.0f, "%.1fs");
	ImGui::Checkbox("Force Mode B Active", &settings::football::force_mode_b_active);
	ImGui::Text("Active: %s", settings::football::mode_b_active ? "true" : "false");
	ImGui::Text("Time Left: %.1fs", settings::football::mode_b_time_left > 0.0 ? settings::football::mode_b_time_left : 0.0);
	ImGui::Spacing();

	ImGui::SeparatorText("Thresholds");
	ImGui::SliderFloat("Mode A Top Y", &settings::football::mode_a_top_threshold, 0.0f, 60.0f, "%.1f");
	ImGui::SliderFloat("Mode B Top Y", &settings::football::mode_b_top_threshold, 0.0f, 60.0f, "%.1f");
	ImGui::SliderFloat("Middle Top Y", &settings::football::mid_top_threshold, 0.0f, 60.0f, "%.1f");
	ImGui::SliderFloat("Mid Iframe Top Y", &settings::football::mid_iframe_top_threshold, 0.0f, 60.0f, "%.1f");

	std::string mode_info = settings::football::mode_b_active ? "MODE B" : "MODE A";
	ImGui::Text("[%s] Ball: %s", mode_info.c_str(), settings::football::status_text.c_str());
	if (settings::football::is_tracking)
		ImGui::Text("Zone: %s | %.2fs", settings::football::current_zone.c_str(), settings::football::predicted_time);
}

static void TabDiveKeys()
{
	ImGui::SeparatorText("Dive Keys");
	keybind::keybind_selector("Space Key", &settings::football::dive_key_space, nullptr);
	keybind::keybind_selector("Left Dive Key", &settings::football::dive_key_left, nullptr);
	keybind::keybind_selector("Right Dive Key", &settings::football::dive_key_right, nullptr);
	ImGui::Spacing();

	ImGui::SeparatorText("Feature Keybinds");
	ImGui::Text("Panel Visibility");
	ImGui::SameLine();
	keybind::keybind_selector("##panel_vis_key", &settings::football::panel_vis_key);
}

static void TabZones()
{
	ImGui::SeparatorText("Zone Fractions");
	ImGui::SliderFloat("Middle Width", &settings::football::mid_fraction, 0.05f, 0.9f, "%.2f");
	ImGui::SliderFloat("Mid-Side Width", &settings::football::mid_side_fraction, 0.05f, 0.95f, "%.2f");
	ImGui::Spacing();

	ImGui::SeparatorText("Post Guard");
	ImGui::Checkbox("Enabled", &settings::football::post_guard_enabled);
	if (settings::football::post_guard_enabled)
	{
		ImGui::SliderFloat("Guard Distance", &settings::football::post_guard_distance, 1.0f, 30.0f, "%.1f");
		ImGui::SliderFloat("Offset Bonus", &settings::football::post_guard_offset_bonus, 0.0f, 0.3f, "%.2f");
	}
}

static void TabPhysics()
{
	ImGui::SliderFloat("Gravity Mult", &settings::football::gravity_mult, 1.0f, 20.0f, "%.1f");
	ImGui::SliderFloat("Bounce Friction", &settings::football::bounce_friction, 0.1f, 1.0f, "%.2f");
	ImGui::SliderFloat("Bounce Vel Y", &settings::football::bounce_vel_y, 0.1f, 1.0f, "%.2f");
	ImGui::SliderFloat("Rolling Friction", &settings::football::rolling_friction, 0.9f, 1.0f, "%.3f");
}

static void TabDisplay()
{
	ImGui::Checkbox("Show Status", &settings::football::show_prediction);
	ImGui::Checkbox("Show Path", &settings::football::show_path);
	ImGui::Checkbox("Show Panel", &settings::football::show_panel);
	ImGui::Spacing();

	ImGui::SeparatorText("Panel Position");
	ImGui::SliderFloat("Behind Dist", &settings::football::panel_behind_dist, -5.0f, 20.0f, "%.1f");
	ImGui::SliderFloat("Height Adj", &settings::football::panel_height_adj, -5.0f, 5.0f, "%.1f");
	ImGui::SliderFloat("Zone Scale X", &settings::football::zone_scale_x, 0.1f, 5.0f, "%.1f");
	ImGui::SliderFloat("Zone Scale Y", &settings::football::zone_scale_y, 0.1f, 5.0f, "%.1f");
}

static void TabKeybinds()
{
	ImGui::SeparatorText("Menu");
	ImGui::Text("Menu Toggle");
	ImGui::SameLine();
	keybind::keybind_selector("##menu_keybind", &settings::menu::menu_keybind);
	ImGui::Spacing();

	ImGui::SeparatorText("Feature Toggles");
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
		if (ImGui::BeginTabItem("Auto GK"))
		{
			TabAutoGK();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Dive Keys"))
		{
			TabDiveKeys();
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

void Menu::DrawWatermark()
{
	if (!settings::menu::watermark) return;
	ImU32 ac = GetAccent();
	std::string wm;
	auto app = [&](const std::string& s) { if (!s.empty()) { if (!wm.empty()) wm += " | "; wm += s; } };
	if (settings::watermark::show_cheat_name) app(external_config::cheat_name);
	if (settings::watermark::show_fps) app(std::to_string(int(ImGui::GetIO().Framerate)) + " fps");
	if (wm.empty()) wm = external_config::cheat_name;
	ImU32 tc = ImGui::ColorConvertFloat4ToU32({ settings::watermark::text_color[0], settings::watermark::text_color[1], settings::watermark::text_color[2], settings::watermark::text_color[3] });
	float tw = ImGui::CalcTextSize(wm.c_str()).x;
	float ww = tw + 40.0f, wh = 34.0f;
	ImGuiWindowFlags wfl = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoNav;
	if (!m_bMenuVisible) wfl |= ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs;
	ImGui::SetNextWindowSize(ImVec2(ww, wh), ImGuiCond_Always);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	bool o = ImGui::Begin("##WM", (bool*)0, wfl);
	ImGui::PopStyleVar(2);
	if (o)
	{
		ImRect wb(ImGui::GetCurrentWindow()->Rect());
		ImGuiWindow* w = ImGui::GetCurrentWindow();
		for (int i = 3; i >= 1; i--) { float e = float(i) * 2.5f; w->DrawList->AddRectFilled(ImVec2(wb.Min.x - e, wb.Min.y - e), ImVec2(wb.Max.x + e, wb.Max.y + e), IM_COL32(0, 0, 0, 10 / i), 18.0f + e); }
		w->DrawList->AddRectFilled(wb.Min, wb.Max, IM_COL32(16, 16, 20, 240), 18.0f);
		w->DrawList->AddRect(wb.Min, wb.Max, IM_COL32(255, 255, 255, 6), 18.0f);
		w->DrawList->AddRectFilled(ImVec2(wb.Min.x + 10, wb.Min.y + 2), ImVec2(wb.Max.x - 10, wb.Min.y + 4), ac, 1.0f);
		w->DrawList->AddText(ImVec2(wb.Min.x + 18, wb.Min.y + 9), tc, wm.c_str());
	}
	ImGui::End();
}

void Menu::DrawMenu()
{
	if (!m_bInitialized) return;

	DrawWatermark();

	if (!m_bMenuVisible) return;

	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();

	ImGui::SetNextWindowSize(ImVec2(WIN_W, WIN_H), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSizeConstraints(ImVec2(650, 500), ImVec2(FLT_MAX, FLT_MAX));
	ImGui::SetNextWindowPos(io.DisplaySize / 2, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

	if (ImGui::Begin("VISION", &m_bMenuVisible))
	{
		ImGui::PushItemWidth(-1.0f);
		TabFootballLocked();
		ImGui::PopItemWidth();
	}
	ImGui::End();
}

void Menu::Shutdown()
{
	if (m_bInitialized)
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		m_bInitialized = false;
	}
}
