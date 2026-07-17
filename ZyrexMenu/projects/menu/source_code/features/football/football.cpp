#include "football.h"
#include <settings.h>
#include <sdk/sdk.h>
#include <game/game.h>
#include <cache/cache.h>
#include <memory/memory.h>
#include <Offsets/Offsets.hpp>
#include <imgui/imgui.h>
#include <menu/keybind/keybind.h>
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <vector>
#include <deque>
#include <functional>

namespace football
{
    // ── Ball state (multi-mode: Script or Part) ──────────────────────────
    static std::uint64_t g_ball_model{ 0 };
    static std::uint64_t g_ball_pos_val{ 0 };
    static std::uint64_t g_ball_vel_val{ 0 };
    static std::uint64_t g_ball_speed_val{ 0 };
    static std::uint64_t g_ball_part_addr{ 0 };
    static bool g_ball_script_mode{ false };
    static math::vector3 g_prev_pos{};
    static bool g_have_prev{ false };
    static std::deque<math::vector3> g_pos_hist2{};
    static math::vector3 g_ball_pos{};
    static math::vector3 g_ball_vel{};
    static double g_now{ 0.0 };
    static double g_dive_cd{ 0.0 };
    static double g_auto_m2_cd{ 0.0 };
    static double g_mode_b_end{ 0.0 };
    static bool g_jvb_kb_active{ false };
    constexpr float JUMP_POWER_DEFAULT = 50.0f;

    // ── Tracking state ────────────────────────────────────────────────────
    static bool g_tracking{ false };
    static int g_missed{ 0 };
    static int g_track_frames{ 0 };
    static double g_track_start{ 0.0 };

    // ── Panel state ───────────────────────────────────────────────────────
    static math::vector3 g_panel_origin{};
    static math::vector3 g_panel_normal{};
    static math::vector3 g_panel_right{};
    static float g_panel_hw{ 45.4f };
    static float g_panel_hh{ 60.2f };

    // ── Post guard state ──────────────────────────────────────────────────
    static std::vector<std::pair<float, float>> g_goal_posts{}; // (leftX, rightX)

    // ── Path prediction state ─────────────────────────────────────────────
    static std::vector<math::vector3> g_path_points{};
    static std::vector<math::vector3> g_bounce_points{};
    static math::vector3 g_last_predicted_hit{};
    static double g_last_known_dist{ 0.0 };
    static double g_goal_display_until{ 0.0 };
    static std::string g_goal_actual_zone{};
    static std::string g_goal_pred_zone{};
    static math::vector3 g_goal_local_pos{};
    static bool g_mb2_held_by_user{ false };
    static bool g_mb2_prev_held{ false };

    // ── Physics constants ─────────────────────────────────────────────────
    static constexpr float LEFT_GOAL_SIZE_X = 60.53413009643555f;
    static constexpr float LEFT_GOAL_SIZE_Y = 40.148765563964844f;
    static constexpr float SIM_STEP = 1.0f / 200.0f;
    static constexpr float MAX_TIME = 5.0f;
    static constexpr int MAX_MISS = 30;
    static constexpr double TRACK_TIMEOUT = 2.0;

    // ── Helpers ───────────────────────────────────────────────────────────
    static float get_grav_y()
    {
        return -9.2f * settings::football::gravity_mult;
    }

    static float get_active_m2_offset()
    {
        return settings::football::mode_b_active ? settings::football::auto_m2_mode_b_dive_offset
                                                  : settings::football::auto_m2_dive_offset;
    }

    static float get_active_offset()
    {
        return settings::football::mode_b_active ? settings::football::mode_b_dive_offset
                                                  : settings::football::mode_a_dive_offset;
    }

    static float get_active_top_thresh()
    {
        return settings::football::mode_b_active ? settings::football::mode_b_top_threshold
                                                  : settings::football::mode_a_top_threshold;
    }

    static void update_panel_size()
    {
        float sx = settings::football::zone_scale_x;
        float sy = settings::football::zone_scale_y;
        g_panel_hw = (LEFT_GOAL_SIZE_X * sx) / 2.0f;
        g_panel_hh = (LEFT_GOAL_SIZE_Y * sy) / 2.0f;
    }

    // ── Zone layout helper (single source of truth for b1-b4 column bounds) ──
    struct ZoneLayout {
        float full_w, mid_w, hm, side_w, ms_w, outer_w;
        float b1, b2, b3, b4;
    };

    static ZoneLayout calc_zone_layout()
    {
        float mf = settings::football::mid_fraction;
        float msf = settings::football::mid_side_fraction;
        float full_w = g_panel_hw * 2.0f;
        float mid_w = full_w * mf;
        float hm = mid_w / 2.0f;
        float side_w = (full_w - mid_w) / 2.0f;
        float ms_w = side_w * msf;
        float outer_w = side_w - ms_w;
        float b1 = -(hm + ms_w), b2 = -hm, b3 = hm, b4 = hm + ms_w;
        return { full_w, mid_w, hm, side_w, ms_w, outer_w, b1, b2, b3, b4 };
    }

    static bool is_m2_held()
    {
        return g_mb2_held_by_user;
    }

    // ── Split zone name ───────────────────────────────────────────────────
    static bool split_zone(const std::string& zn, std::string& row, std::string& col)
    {
        size_t us = zn.find('_');
        if (us == std::string::npos) return false;
        row = zn.substr(0, us);
        col = zn.substr(us + 1);
        return true;
    }

    // ── Ball data struct ──────────────────────────────────────────────────
    struct BallData {
        std::uint64_t model_addr{ 0 };  // ballEntity model or Part address
        math::vector3 pos{};
        math::vector3 vel{};
    };

    // ── Collect ALL valid balls (Script mode) ──────────────────────────────
    static std::vector<BallData> collect_script_balls()
    {
        std::vector<BallData> result;
        rbx::instance_t bf = game::workspace.find_first_child("BallFolderServer");
        if (bf.address == 0) return result;
        auto kids = bf.get_children();
        for (auto& k : kids)
        {
            if (k.get_name() != "ballEntity" || k.get_class_name() != "Model") continue;
            rbx::instance_t p = k.find_first_child("Position");
            rbx::instance_t v = k.find_first_child("Velocity");
            rbx::instance_t s = k.find_first_child("BallSpeed");
            if (!p.address || !v.address || !s.address) continue;
            try {
                math::vector3 pos = memory->read<math::vector3>(p.address + Offsets::Misc::Value);
                math::vector3 vel = memory->read<math::vector3>(v.address + Offsets::Misc::Value);
                if (std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z)) continue;
                if (std::isnan(vel.x) || std::isnan(vel.y) || std::isnan(vel.z)) continue;
                result.push_back({ k.address, pos, vel });
            } catch (...) {}
        }
        return result;
    }

    // ── Collect ALL valid balls (Part mode, iterative stack) ──────────────
    // HEAVY: only called as fallback when script mode fails
    static std::vector<BallData> collect_part_balls()
    {
        // Disabled on purpose: exact Lua parity reads only
        // Workspace -> BallFolderServer -> ballEntity -> Position/Velocity/BallSpeed.Value.
        // Do not fall back to primitive Position or AssemblyLinearVelocity.
        return {};
    }

    // ── Pick the best ball (fastest speed → highest urgency) ──────────────
    static BallData pick_best_ball(const std::vector<BallData>& balls)
    {
        BallData best;
        float best_score = -1.0f;
        for (auto& b : balls)
        {
            float spd = b.vel.length();
            if (spd > best_score) { best_score = spd; best = b; }
        }
        return best;
    }

    // ── Find best ball: try Script mode first, fall back to Part mode ─────
    // Returns true if ball was found, sets g_ball_mode accordingly
    static BallData g_ball_data{};

    static bool refresh_best_ball()
    {
        g_ball_data = {};

        // Script-mode only: match Lua script exactly.
        // Workspace -> BallFolderServer -> ballEntity(Model) -> Position/Velocity/BallSpeed.Value
        auto script_balls = collect_script_balls();
        if (!script_balls.empty()) {
            g_ball_data = pick_best_ball(script_balls);
            if (g_ball_data.model_addr != 0) {
                g_ball_script_mode = true;
                return true;
            }
        }

        return false;
    }

    // ── Read ball data from best ball (g_ball_data) ───────────────────────
    static bool read_best_ball(math::vector3& pos, math::vector3& vel)
    {
        if (g_ball_data.model_addr == 0) return false;
        pos = g_ball_data.pos;
        vel = g_ball_data.vel;
        if (std::isnan(pos.x) || std::isnan(pos.y) || std::isnan(pos.z)) return false;
        if (std::isnan(vel.x) || std::isnan(vel.y) || std::isnan(vel.z)) return false;
        if (std::abs(pos.x) > 100000.0f || std::abs(pos.y) > 100000.0f || std::abs(pos.z) > 100000.0f) return false;
        if (std::abs(vel.x) > 100000.0f || std::abs(vel.y) > 100000.0f || std::abs(vel.z) > 100000.0f) return false;
        return true;
    }

    // ── Scan for GoalPost parts (for Post Guard) ─────────────────────────
    static void scan_goal_posts()
    {
        g_goal_posts.clear();
        if (game::workspace.address == 0) return;
        auto ds = game::workspace.get_children();
        for (auto& d : ds)
        {
            std::string n = d.get_name();
            std::string l = n;
            std::transform(l.begin(), l.end(), l.begin(), ::tolower);
            if (l.find("goalpost") != std::string::npos || l.find("goal_post") != std::string::npos || l.find("goal post") != std::string::npos)
            {
                if (d.get_class_name() != "Part" && d.get_class_name() != "MeshPart" && d.get_class_name() != "BasePart") continue;
                try {
                    rbx::part_t pt(d.address);
                    rbx::primitive_t prim = pt.get_primitive();
                    if (prim.address == 0) continue;
                    math::vector3 pos = prim.get_position();
                    math::vector3 sz = prim.get_size();
                    g_goal_posts.push_back({ pos.x - sz.x / 2.0f, pos.x + sz.x / 2.0f });
                } catch (...) {}
            }
        }
    }

    // ── Post Guard: redirect zone if near goal post ───────────────────────
    static std::string apply_post_guard(const std::string& zone)
    {
        if (!settings::football::post_guard_enabled) return zone;
        if (game::local_character.address == 0) return zone;

        auto hrp = game::local_character.find_first_child("HumanoidRootPart");
        if (hrp.address == 0) return zone;

        math::vector3 hrp_pos{};
        try {
            rbx::part_t p(hrp.address);
            rbx::primitive_t prim = p.get_primitive();
            if (prim.address == 0) return zone;
            hrp_pos = prim.get_position();
        } catch (...) { return zone; }

        std::string row, col;
        if (!split_zone(zone, row, col)) return zone;
        if (g_goal_posts.empty()) scan_goal_posts();

        float pd = settings::football::post_guard_distance;
        for (auto& post : g_goal_posts)
        {
            float post_center = (post.first + post.second) / 2.0f;
            float dist = std::abs(hrp_pos.x - post_center);
            if (dist <= pd)
            {
                if ((col == "LEFT" || col == "MID_LEFT") && hrp_pos.x < post_center)
                    return row + "_MIDDLE";
                if ((col == "RIGHT" || col == "MID_RIGHT") && hrp_pos.x > post_center)
                    return row + "_MIDDLE";
            }
        }
        return zone;
    }

    // ── Update camera-relative panel ──────────────────────────────────────
    static void update_panel()
    {
        if (game::local_character.address == 0) return;
        auto hrp = game::local_character.find_first_child("HumanoidRootPart");
        if (hrp.address == 0) return;

        math::vector3 hrp_pos{};
        try {
            rbx::part_t p(hrp.address);
            rbx::primitive_t prim = p.get_primitive();
            if (prim.address == 0) return;
            hrp_pos = prim.get_position();
        } catch (...) { return; }

        // Camera look from Camera instance rotation (reliable, not VP matrix)
        math::vector3 cam_look{ 0, 0, 1 };
        try {
            if (game::camera) {
                math::matrix3 rot = memory->read<math::matrix3>(game::camera + Offsets::Camera::Rotation);
                cam_look = rot.forward();
                float len = std::sqrtf(cam_look.x * cam_look.x + cam_look.y * cam_look.y + cam_look.z * cam_look.z);
                if (len > 0.001f) { float inv = 1.0f / len; cam_look = { cam_look.x * inv, cam_look.y * inv, cam_look.z * inv }; }
            }
        } catch (...) {}

        // Flatten
        math::vector3 flat = { cam_look.x, 0, cam_look.z };
        float flen = std::sqrtf(flat.x * flat.x + flat.z * flat.z);
        if (flen < 0.001f) flat = { 0, 0, 1 };
        else { float inv = 1.0f / flen; flat = { flat.x * inv, 0, flat.z * inv }; }

        float bd = settings::football::panel_behind_dist;
        float ha = settings::football::panel_height_adj;
        g_panel_origin = { hrp_pos.x + flat.x * bd, hrp_pos.y + ha, hrp_pos.z + flat.z * bd };

        // Match Lua exactly:
        // panelCF = CFrame.new(behindPos, behindPos + flatLook) * CFrame.Angles(0, pi, 0)
        // After the pi flip, LookVector becomes -flatLook.
        // CFrame:PointToObjectSpace uses the full local basis, so keep a persistent
        // panel_right basis instead of rebuilding it differently in prediction/render.
        g_panel_normal = { -flat.x, 0, -flat.z };
        g_panel_right  = { -flat.z, 0,  flat.x };

        float rlen = std::sqrtf(g_panel_right.x * g_panel_right.x + g_panel_right.z * g_panel_right.z);
        if (rlen > 0.001f) {
            float rinv = 1.0f / rlen;
            g_panel_right = { g_panel_right.x * rinv, 0, g_panel_right.z * rinv };
        }

        update_panel_size();
    }

    // ── Physics simulation + panel intersection ───────────────────────────
    static bool simulate_path(const math::vector3& pos, const math::vector3& vel,
                              std::vector<math::vector3>& out_pts,
                              std::vector<math::vector3>& out_bounces)
    {
        out_pts.clear(); out_bounces.clear();
        out_pts.push_back(pos);
        float gy = get_grav_y();
        float gr = settings::football::ground_y;
        float bvy = settings::football::bounce_vel_y;
        float bf = settings::football::bounce_friction;
        float rf = settings::football::rolling_friction;
        float sv = settings::football::stop_velocity;
        float mbv = settings::football::min_bounce_velocity;

        math::vector3 cp = pos, cv = vel;
        float t = 0.0f;
        bool rolling = false;
        int bounces = 0;

        while (t < MAX_TIME)
        {
            if (rolling)
            {
                cv = { cv.x * rf, cv.y * rf, cv.z * rf };
                if (cv.length() < sv) break;
                cp = { cp.x + cv.x * SIM_STEP, cp.y + cv.y * SIM_STEP, cp.z + cv.z * SIM_STEP };
            }
            else
            {
                cv.y += gy * SIM_STEP;
                cp = { cp.x + cv.x * SIM_STEP, cp.y + cv.y * SIM_STEP, cp.z + cv.z * SIM_STEP };
            }

            if (cp.y < gr)
            {
                cp.y = gr;
                bounces++;
                if (bounces <= 15) out_bounces.push_back(cp);
                if (std::abs(cv.y) * bvy < mbv)
                {
                    rolling = true;
                    cv = { cv.x * bf, 0, cv.z * bf };
                }
                else
                {
                    cv = { cv.x * bf, std::abs(cv.y) * bvy, cv.z * bf };
                }
            }
            out_pts.push_back(cp);
            t += SIM_STEP;
            if (out_pts.size() > 500) break;
        }
        return out_pts.size() > 1;
    }

    // ── Predict panel hit (returns live TTG) ──────────────────────────────
    static bool predict_hit(const math::vector3& pos, const math::vector3& vel,
                            float& out_ttg, math::vector3& out_hit, std::string& out_zone)
    {
        float gy = get_grav_y();
        float gr = settings::football::ground_y;
        float bvy = settings::football::bounce_vel_y;
        float bf = settings::football::bounce_friction;
        float rf = settings::football::rolling_friction;
        float sv = settings::football::stop_velocity;
        float mbv = settings::football::min_bounce_velocity;
        float top_th = get_active_top_thresh();

        math::vector3 cp = pos, cv = vel;
        float t = 0.0f;
        bool rolling = false;

        while (t < MAX_TIME)
        {
            math::vector3 prev = cp;

            if (rolling)
            {
                cv = { cv.x * rf, cv.y * rf, cv.z * rf };
                if (cv.length() < sv) break;
                cp = { cp.x + cv.x * SIM_STEP, cp.y + cv.y * SIM_STEP, cp.z + cv.z * SIM_STEP };
            }
            else
            {
                cv.y += gy * SIM_STEP;
                cp = { cp.x + cv.x * SIM_STEP, cp.y + cv.y * SIM_STEP, cp.z + cv.z * SIM_STEP };
            }

            if (cp.y < gr)
            {
                cp.y = gr;
                if (std::abs(cv.y) * bvy < mbv)
                {
                    rolling = true;
                    cv = { cv.x * bf, 0, cv.z * bf };
                }
                else
                {
                    cv = { cv.x * bf, std::abs(cv.y) * bvy, cv.z * bf };
                }
            }

            // Plane crossing
            float d_prev = (prev.x - g_panel_origin.x) * g_panel_normal.x +
                           (prev.y - g_panel_origin.y) * g_panel_normal.y +
                           (prev.z - g_panel_origin.z) * g_panel_normal.z;
            float d_curr = (cp.x - g_panel_origin.x) * g_panel_normal.x +
                           (cp.y - g_panel_origin.y) * g_panel_normal.y +
                           (cp.z - g_panel_origin.z) * g_panel_normal.z;

            if (d_prev * d_curr <= 0.0f && d_prev != d_curr)
            {
                float frac = d_prev / (d_prev - d_curr);
                float hx = prev.x + (cp.x - prev.x) * frac;
                float hy = prev.y + (cp.y - prev.y) * frac;
                float hz = prev.z + (cp.z - prev.z) * frac;

                // Convert world hit into the same panel-local space as Lua panelCF:PointToObjectSpace.
                // X = dot(hit - origin, panel right), Y = world up delta.
                if (g_panel_right.length() < 0.001f) continue;
                float dx = hx - g_panel_origin.x;
                float dy = hy - g_panel_origin.y;
                float dz = hz - g_panel_origin.z;
                float lx = dx * g_panel_right.x + dz * g_panel_right.z;
                float ly = dy;

                if (std::abs(lx) <= g_panel_hw && std::abs(ly) <= g_panel_hh)
                {
                    out_ttg = t + frac * SIM_STEP;
                    out_hit = { hx, hy, hz };

                    auto zl = calc_zone_layout();
                    const char* col;
                    if (lx < zl.b1) col = "LEFT";
                    else if (lx < zl.b2) col = "MID_LEFT";
                    else if (lx < zl.b3) col = "MIDDLE";
                    else if (lx < zl.b4) col = "MID_RIGHT";
                    else col = "RIGHT";

                    // Middle zone uses mid_top_threshold
                    float effective_top = top_th;
                    if (std::string(col) == "MIDDLE")
                        effective_top = settings::football::mid_top_threshold;

                    const char* row = (ly >= effective_top) ? "TOP" : "BOTTOM";
                    out_zone = std::string(row) + "_" + col;
                    return true;
                }
            }
            t += SIM_STEP;
        }
        return false;
    }

    // ── Smart M2 ──────────────────────────────────────────────────────────
    static void do_m2()
    {
        if (is_m2_held())
        {
            INPUT in = {}; in.type = INPUT_MOUSE; in.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            SendInput(1, &in, sizeof(INPUT));
        }
        else
        {
            INPUT in = {}; in.type = INPUT_MOUSE; in.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            SendInput(1, &in, sizeof(INPUT));
            Sleep(16);
            in.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            SendInput(1, &in, sizeof(INPUT));
        }
    }

    // ── Keyboard tap helper: send real key hold so Roblox reliably sees Space/C/Z ──
    static void tap_key(WORD vk)
    {
        WORD scan = (WORD)MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);

        INPUT down = {};
        down.type = INPUT_KEYBOARD;
        down.ki.wVk = 0;
        down.ki.wScan = scan;
        down.ki.dwFlags = KEYEVENTF_SCANCODE;
        SendInput(1, &down, sizeof(INPUT));

        Sleep(25);

        INPUT up = {};
        up.type = INPUT_KEYBOARD;
        up.ki.wVk = 0;
        up.ki.wScan = scan;
        up.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &up, sizeof(INPUT));
    }

    // ── Dive: configurable keys with random alternate support ──────
    static void do_dive(const std::string& zone, bool use_random_key = false)
    {
        if (zone.empty()) return;
        std::string row, col;
        if (!split_zone(zone, row, col)) return;

        // Collect keys that would be correct for this zone
        std::vector<WORD> zone_keys;
        if (row == "TOP")
            zone_keys.push_back((WORD)settings::football::dive_key_space);
        if (col == "LEFT" || col == "MID_LEFT")
            zone_keys.push_back((WORD)settings::football::dive_key_left);
        else if (col == "RIGHT" || col == "MID_RIGHT")
            zone_keys.push_back((WORD)settings::football::dive_key_right);
        else if (col == "MIDDLE" && settings::football::dive_key_middle != 0)
            zone_keys.push_back((WORD)settings::football::dive_key_middle);

        if (zone_keys.empty()) return;

        WORD key = zone_keys[0];

        // Random alternate key
        if (use_random_key && settings::football::random_dive_enabled) {
            std::vector<WORD> all_keys;
            if (settings::football::dive_key_space != 0) all_keys.push_back((WORD)settings::football::dive_key_space);
            if (settings::football::dive_key_left != 0) all_keys.push_back((WORD)settings::football::dive_key_left);
            if (settings::football::dive_key_right != 0) all_keys.push_back((WORD)settings::football::dive_key_right);
            if (settings::football::dive_key_middle != 0) all_keys.push_back((WORD)settings::football::dive_key_middle);
            if (all_keys.size() > 1) {
                size_t idx = rand() % all_keys.size();
                key = all_keys[idx];
            }
        }

        tap_key(key);
        g_dive_cd = g_now + (double)settings::football::dive_cooldown;
    }

    // ── Auto M2: Lua order — Jump → delay → M2 (TOP zones) ────────────────
    static void do_auto_m2(const std::string& zone)
    {
        if (zone.empty()) return;
        std::string row, col;
        if (!split_zone(zone, row, col)) return;

        // Lua order: Jump → delay → M2 (for TOP zones)
        if (row == "TOP") {
            tap_key((WORD)settings::football::dive_key_space);
            if (settings::football::m2_jump_delay > 0.0f)
                Sleep((DWORD)(settings::football::m2_jump_delay * 1000.0f));
        }

        // M2 always. If already held, release only. If not held, tap down/up.
        if (is_m2_held())
        {
            INPUT up = {};
            up.type = INPUT_MOUSE;
            up.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            SendInput(1, &up, sizeof(INPUT));
        }
        else
        {
            INPUT down = {};
            down.type = INPUT_MOUSE;
            down.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
            SendInput(1, &down, sizeof(INPUT));

            Sleep(16);

            INPUT up = {};
            up.type = INPUT_MOUSE;
            up.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
            SendInput(1, &up, sizeof(INPUT));
        }

        g_auto_m2_cd = g_now + (double)settings::football::auto_m2_cooldown;
    }

    // ── Mode B: check Workspace->Effects->jump ───────────────────────────
    static bool check_effect_trigger()
    {
        if (!settings::football::mode_b_effect_enabled) return false;
        if (game::workspace.address == 0) return false;

        // Find Workspace->Effects
        rbx::instance_t effects = game::workspace.find_first_child("Effects");
        settings::football::mode_b_effects_addr = effects.address;

        if (effects.address == 0) {
            settings::football::mode_b_effects_child_count = 0;
            return false;
        }

        auto kids = effects.get_children();
        settings::football::mode_b_effects_child_count = (int)kids.size();

        for (auto& k : kids) {
            if (!k.address) continue;
            std::string name;
            std::string cn;
            try { name = k.get_name(); } catch (...) { continue; }
            try { cn = k.get_class_name(); } catch (...) { continue; }

            settings::football::mode_b_last_effect_name = name;
            settings::football::mode_b_last_effect_addr = k.address;

            // Case-insensitive check for "jump"
            std::string lower = name;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            if (lower == "jump") {
                settings::football::mode_b_found_jump = true;
                settings::football::mode_b_last_effect_time = g_now;
                return true;
            }
        }

        settings::football::mode_b_found_jump = false;
        return false;
    }

    // ── Mode B: update active state ─────────────────────────────────────
    static void update_mode_b()
    {
        // Force override
        if (settings::football::force_mode_b_active) {
            settings::football::mode_b_active = true;
            settings::football::mode_b_trigger_source = "Force";
            settings::football::mode_b_time_left = 99.0;
            return;
        }

        // ── Check effect trigger (always, even if mode_b_enabled is off) ──
        bool effect_triggered = check_effect_trigger();
        if (effect_triggered && !settings::football::mode_b_active) {
            settings::football::mode_b_active = true;
            settings::football::mode_b_trigger_source = "Effect";
            g_mode_b_end = g_now + (double)settings::football::mode_b_duration;

        }

        // ── Expiry: deactivate when timer runs out ─────────────────────────
        if (settings::football::mode_b_active && g_now >= g_mode_b_end) {
            settings::football::mode_b_active = false;
            settings::football::mode_b_trigger_source = "";
        }

        // Update time left for debug
        settings::football::mode_b_time_left = settings::football::mode_b_active
            ? g_mode_b_end - g_now : 0.0;
    }

    // ── Process feature keybinds ──────────────────────────────────────────
    static void process_keybinds()
    {
        // Autodive
        {
            static keybind::keybind_t kb;
            kb.key = settings::football::autodive_key;
            kb.mode = (keybind::activation_mode)settings::football::autodive_key_mode;
            settings::football::autodive_enabled = keybind::is_active(kb);
        }
        // Auto M2
        {
            static keybind::keybind_t kb;
            kb.key = settings::football::auto_m2_key;
            kb.mode = keybind::activation_mode::hold;
            settings::football::auto_m2 = keybind::is_active(kb);
        }
        // Panel visibility (toggle only, edge-triggered)
        {
            static keybind::keybind_t kb;
            kb.key = settings::football::panel_vis_key;
            kb.mode = keybind::activation_mode::toggle;
            static bool prev_state = false;
            bool now_state = keybind::is_active(kb);
            if (now_state && !prev_state)
                settings::football::show_panel = !settings::football::show_panel;
            prev_state = now_state;
        }
        // Jump Velocity Boost — keybind state OR checkbox
        if (settings::movement::jump_velocity_boost::keybind != 0)
        {
            static keybind::keybind_t kb;
            kb.key = settings::movement::jump_velocity_boost::keybind;
            kb.mode = (keybind::activation_mode)settings::movement::jump_velocity_boost::activation_mode;
            g_jvb_kb_active = keybind::is_active(kb);
        }
        else
        {
            g_jvb_kb_active = false;
        }
    }

    // ── Main tick ─────────────────────────────────────────────────────────
    void tick()
    {
        if (memory->last_read_failed.load()) {
            memory->reset_error_flags();
            game::clear_state();
            settings::football::is_tracking = false;
            g_tracking = false; g_ball_model = 0; g_ball_data = {};
            return;
        }

        // No rate limit — runs every frame (RenderStepped equivalent)
        auto now = std::chrono::steady_clock::now();
        g_now = std::chrono::duration<double>(now.time_since_epoch()).count();

        // Process keybinds (updates autodive_enabled, auto_m2, show_panel)
        process_keybinds();

        // Update panel every frame regardless of feature state so keybind-only toggling works
        update_panel();

        // Guard: if core game state is dead, bail out
        if (game::workspace.address == 0 || game::local_character.address == 0) {
            settings::football::is_tracking = false;
            settings::football::status_text = "No game";
            g_tracking = false; g_ball_model = 0; g_ball_data = {};
            return;
        }

        // ── Jump Velocity Boost (absolute JumpPower, like Linoria script) ──
        {
            bool jvb_active = settings::movement::jump_velocity_boost::enabled || g_jvb_kb_active;
            auto humanoid = game::local_character.find_first_child_by_class("Humanoid");
            if (humanoid.address != 0) {
                if (jvb_active) {
                    // Enabled: set UseJumpPower=true and write the slider value
                    memory->write<uint8_t>(humanoid.address + Offsets::Humanoid::UseJumpPower, 1);
                    memory->write<float>(humanoid.address + Offsets::Humanoid::JumpPower, settings::movement::jump_velocity_boost::jp_boost);
                } else {
                    // Disabled: set UseJumpPower=false and restore default JumpPower
                    memory->write<uint8_t>(humanoid.address + Offsets::Humanoid::UseJumpPower, 0);
                    memory->write<float>(humanoid.address + Offsets::Humanoid::JumpPower, JUMP_POWER_DEFAULT);
                }
            }
        }

        bool en = settings::football::autodive_enabled || settings::football::auto_m2;
        if (!en && !settings::football::mode_b_enabled && !settings::football::force_mode_b_active) {
            settings::football::is_tracking = false;
            settings::football::status_text = "Disabled";
            g_tracking = false; g_ball_model = 0; g_ball_data = {}; return;
        }

        // Update Mode B
        update_mode_b();

        // Scan for goal posts every 5 seconds for post guard
        static double last_post_scan = 0.0;
        if (settings::football::post_guard_enabled && g_now - last_post_scan > 5.0) {
            scan_goal_posts();
            last_post_scan = g_now;
        }

        // Collect all balls
        auto balls = collect_script_balls();

        // Tracking timeout
        if (g_tracking && (g_now - g_track_start) >= TRACK_TIMEOUT) {
            g_tracking = false; g_track_frames = 0;
            settings::football::is_tracking = false;
            settings::football::status_text = "Tracking timed out";
        }

        // ── MB2 hold-state tracking ──
        {
            bool mb2_now = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            if (mb2_now && !g_mb2_prev_held) g_mb2_held_by_user = true;
            else if (!mb2_now && g_mb2_prev_held) g_mb2_held_by_user = false;
            g_mb2_prev_held = mb2_now;
        }

        // ── Loop all balls like Lua ──
        bool did_track = false;
        bool prediction_ok = false;

        for (auto& ball : balls)
        {
            float ttg = 0.0f;
            math::vector3 hit{};
            std::string zone;
            bool pred_ok = predict_hit(ball.pos, ball.vel, ttg, hit, zone);

            float max_t = settings::football::mode_b_active ? 3.0f : MAX_TIME;
            bool valid_pred = pred_ok && ttg > 0 && ttg < max_t;
            if (settings::football::mode_b_active && ball.vel.length() <= 0.01f)
                valid_pred = false;

            bool is_tracked = (g_tracking && ball.model_addr == g_ball_model);

            if (valid_pred || is_tracked)
            {
                did_track = true;

                if (!g_tracking || ball.model_addr != g_ball_model) {
                    g_tracking = true;
                    g_track_start = g_now;
                    g_missed = 0;
                    settings::football::is_tracking = true;
                    g_ball_model = ball.model_addr;
                }
                g_ball_pos = ball.pos;
                g_ball_vel = ball.vel;

                if (valid_pred)
                {
                    prediction_ok = true;
                    g_missed = 0;
                    settings::football::predicted_time = ttg;
                    settings::football::current_zone = zone;
                    g_last_predicted_hit = hit;
                    g_track_frames++;

                    char buf[128];
                    snprintf(buf, sizeof(buf), "%.2fs to panel | %s", ttg, zone.c_str());
                    if (settings::football::mode_b_active)
                        snprintf(buf, sizeof(buf), "MODE B %.2fs | %s", ttg, zone.c_str());
                    settings::football::status_text = buf;

                    // Post guard
                    std::string final_zone = apply_post_guard(zone);
                    float offset = get_active_offset();
                    if (final_zone != zone) {
                        offset += settings::football::post_guard_offset_bonus;
                        settings::football::current_zone = final_zone;
                    }

                    float m2_offset = get_active_m2_offset();
                    if (final_zone != zone) m2_offset += settings::football::post_guard_offset_bonus;

                    // Random dive
                    bool use_random_key = false;
                    float dive_offset = offset;
                    if (settings::football::random_dive_enabled) {
                        float r = (float)rand() / (float)RAND_MAX;
                        if (r < 0.5f) {
                            dive_offset *= (1.0f - settings::football::random_dive_offset_reduction);
                            use_random_key = true;
                        }
                    }

                    if (settings::football::autodive_enabled && ttg <= dive_offset && g_now >= g_dive_cd)
                        do_dive(final_zone, use_random_key);
                    if (settings::football::auto_m2 && ttg <= m2_offset && g_now >= g_auto_m2_cd)
                        do_auto_m2(final_zone);
                }
                else
                {
                    // Prediction failed but this is the tracked ball
                    g_missed++;
                    if (g_missed > MAX_MISS) {
                        g_tracking = false; g_track_frames = 0;
                        settings::football::is_tracking = false;
                        settings::football::status_text = "Prediction lost";
                    } else {
                        settings::football::status_text = "Tracking (Stable)";
                    }
                }
            }
        }

        if (!did_track)
        {
            if (g_tracking) {
                g_missed++;
                if (g_missed > MAX_MISS) {
                    g_tracking = false; g_track_frames = 0;
                    settings::football::is_tracking = false;
                    settings::football::status_text = "Prediction lost";
                } else {
                    settings::football::status_text = "Tracking (Stable)";
                }
            } else {
                char buf[64];
                snprintf(buf, sizeof(buf), "Watching %d ball(s)...", (int)balls.size());
                settings::football::status_text = buf;
            }
        }

        // ── Path simulation (full trajectory, independent of hit detection) ──
        if (settings::football::show_path && g_tracking && g_track_frames > 0)
        {
            simulate_path(g_ball_pos, g_ball_vel, g_path_points, g_bounce_points);
        }

        // ── Goal-scored detection ──
        if (g_tracking && settings::football::is_tracking && prediction_ok)
        {
            math::vector3 diff = {
                g_ball_pos.x - g_panel_origin.x,
                g_ball_pos.y - g_panel_origin.y,
                g_ball_pos.z - g_panel_origin.z
            };
            float dist = std::abs(diff.x * g_panel_normal.x + diff.y * g_panel_normal.y + diff.z * g_panel_normal.z);
            float lx = diff.x * g_panel_right.x + diff.z * g_panel_right.z;
            float ly = diff.y;
            bool in_bounds = std::abs(lx) <= g_panel_hw && std::abs(ly) <= g_panel_hh;

            bool scored = (dist < 2.0f && in_bounds) ||
                          (dist > g_last_known_dist && g_last_known_dist < 3.0f);

            if (scored && g_now > g_goal_display_until + 3.0)
            {
                auto zl = calc_zone_layout();
                const char* actual_col;
                if (lx < zl.b1) actual_col = "LEFT";
                else if (lx < zl.b2) actual_col = "MID_LEFT";
                else if (lx < zl.b3) actual_col = "MIDDLE";
                else if (lx < zl.b4) actual_col = "MID_RIGHT";
                else actual_col = "RIGHT";

                float th = get_active_top_thresh();
                float effective_top = th;
                if (std::string(actual_col) == "MIDDLE")
                    effective_top = settings::football::mid_top_threshold;
                const char* actual_row = (ly >= effective_top) ? "TOP" : "BOTTOM";

                g_goal_actual_zone = std::string(actual_row) + "_" + actual_col;
                g_goal_pred_zone = settings::football::current_zone;
                g_goal_local_pos = { lx, ly, 0 };
                g_goal_display_until = g_now + 3.0;

                settings::football::goal_scored = true;
                settings::football::goal_actual_zone = g_goal_actual_zone;
                settings::football::goal_predicted_zone = g_goal_pred_zone;
                settings::football::goal_time = g_now;


                g_tracking = false; g_track_frames = 0;
                g_ball_model = 0;
                settings::football::is_tracking = false;
            }

            g_last_known_dist = dist;
        }

        // ── Goal display expiry ──
        if (settings::football::goal_scored && g_now > g_goal_display_until)
        {
            settings::football::goal_scored = false;
            settings::football::goal_actual_zone.clear();
            settings::football::goal_predicted_zone.clear();
        }
    }

    // ── Project 3D point to screen ────────────────────────────────────────
    static bool w2s(const math::vector3& world, ImVec2& out)
    {
        if (!game::visengine.address) return false;
        try {
            math::vector2 dims = game::visengine.get_dimensions();
            math::matrix4 view = memory->read<math::matrix4>(game::visengine.address + Offsets::VisualEngine::ViewMatrix);
            math::vector2 screen;
            if (game::visengine.world_to_screen(world, screen, dims, view)) {
                out = { screen.x, screen.y };
                return true;
            }
        } catch (...) {}
        return false;
    }

    // ── Status overlay + zone panel visualization ─────────────────────────
    void render_status()
    {
        bool show_any = settings::football::show_prediction || settings::football::show_path || settings::football::show_panel;
        if (!show_any) return;

        ImDrawList* dl = ImGui::GetBackgroundDrawList();
        ImVec2 s = ImGui::GetIO().DisplaySize;

        // ── Draw zone panel: 3D projected + 2D fallback grid ─────────────
        if (settings::football::show_panel)
        {
            auto zl = calc_zone_layout();
            float outer_w = zl.outer_w;
            float full_w = zl.full_w;

            struct ColInfo { const char* name; float offset; float width; int cx; };
            ColInfo cols[5] = {
                { "LEFT", -(zl.hm + zl.ms_w + outer_w / 2), outer_w, 0 },
                { "MID_LEFT", -(zl.hm + zl.ms_w / 2), zl.ms_w, 0 },
                { "MIDDLE", 0, zl.mid_w, 0 },
                { "MID_RIGHT", zl.hm + zl.ms_w / 2, zl.ms_w, 0 },
                { "RIGHT", zl.hm + zl.ms_w + outer_w / 2, outer_w, 0 },
            };

            float th = get_active_top_thresh();
            float top_h = g_panel_hh - th; if (top_h < 1.0f) top_h = 1.0f;
            float bot_h = g_panel_hh + th; if (bot_h < 1.0f) bot_h = 1.0f;

            struct RowInfo { const char* name; float offset; float height; int ry; };
            RowInfo rows[2] = {
                { "TOP", th + top_h / 2, top_h, 0 },
                { "BOTTOM", th - bot_h / 2, bot_h, 0 },
            };

            std::string act_row, act_col;
            bool has_act = split_zone(settings::football::current_zone, act_row, act_col);

            // Try 3D projection first (only if visengine is ready)
            bool drew_3d = false;
            if (game::visengine.address && g_panel_normal.length() > 0.1f)
            {
                math::vector3 up = { 0, 1, 0 };
                math::vector3 right = g_panel_right;
                if (right.length() < 0.001f)
                    right = { 1, 0, 0 };

                bool any_visible = false;
                for (int ri = 0; ri < 2; ri++)
                {
                    for (int ci = 0; ci < 5; ci++)
                    {
                        float hw = cols[ci].width / 2.0f;
                        float hh = rows[ri].height / 2.0f;
                        float ox = cols[ci].offset;
                        float oy = rows[ri].offset;

                        math::vector3 c[4] = {
                            { g_panel_origin.x + right.x * (ox - hw) + up.x * (oy - hh),
                              g_panel_origin.y + right.y * (ox - hw) + up.y * (oy - hh),
                              g_panel_origin.z + right.z * (ox - hw) + up.z * (oy - hh) },
                            { g_panel_origin.x + right.x * (ox + hw) + up.x * (oy - hh),
                              g_panel_origin.y + right.y * (ox + hw) + up.y * (oy - hh),
                              g_panel_origin.z + right.z * (ox + hw) + up.z * (oy - hh) },
                            { g_panel_origin.x + right.x * (ox + hw) + up.x * (oy + hh),
                              g_panel_origin.y + right.y * (ox + hw) + up.y * (oy + hh),
                              g_panel_origin.z + right.z * (ox + hw) + up.z * (oy + hh) },
                            { g_panel_origin.x + right.x * (ox - hw) + up.x * (oy + hh),
                              g_panel_origin.y + right.y * (ox - hw) + up.y * (oy + hh),
                              g_panel_origin.z + right.z * (ox - hw) + up.z * (oy + hh) },
                        };
                        ImVec2 sc[4]; bool ok = true;
                        for (int ci2 = 0; ci2 < 4; ci2++)
                            if (!w2s(c[ci2], sc[ci2])) { ok = false; break; }
                        if (!ok) continue;
                        any_visible = true;

                        bool act = has_act && rows[ri].name == act_row && cols[ci].name == act_col;
                        ImU32 col = act ? IM_COL32(255, 40, 40, 200) : IM_COL32(0, 150, 255, 80);
                        ImU32 bdr = act ? IM_COL32(255, 80, 80, 255) : IM_COL32(0, 180, 255, 180);
                        dl->AddConvexPolyFilled(sc, 4, col);
                        dl->AddPolyline(sc, 4, bdr, ImDrawFlags_Closed, 2.0f);
                        if (act) {
                            ImVec2 cen = { (sc[0].x + sc[2].x) / 2, (sc[0].y + sc[2].y) / 2 };
                            char lb[32]; snprintf(lb, sizeof(lb), "%s %s", rows[ri].name, cols[ci].name);
                            ImVec2 tsz = ImGui::CalcTextSize(lb);
                            dl->AddText({ cen.x - tsz.x / 2, cen.y - tsz.y / 2 }, IM_COL32(255,255,255,255), lb);
                        }
                    }
                }
                if (any_visible) drew_3d = true;

                // Yellow threshold line
                float yt = g_panel_origin.y + th;
                ImVec2 sl, sr;
                if (w2s({ g_panel_origin.x - right.x * g_panel_hw, yt, g_panel_origin.z - right.z * g_panel_hw }, sl) &&
                    w2s({ g_panel_origin.x + right.x * g_panel_hw, yt, g_panel_origin.z + right.z * g_panel_hw }, sr)) {
                    dl->AddLine(sl, sr, IM_COL32(255, 255, 0, 220), 3.0f);
                }

                // Red hit dot
                if (settings::football::is_tracking) {
                    ImVec2 sh;
                    if (w2s(g_last_predicted_hit, sh)) {
                        dl->AddCircleFilled(sh, 8, IM_COL32(255, 0, 0, 240));
                        dl->AddCircle(sh, 12, IM_COL32(255, 255, 255, 200), 0, 2.5f);
                    }
                }
            }


        }

        // ── Path trace ──
        if (settings::football::show_path && !g_path_points.empty())
        {
            for (size_t i = 1; i < g_path_points.size(); i++)
            {
                ImVec2 a, b;
                if (w2s(g_path_points[i-1], a) && w2s(g_path_points[i], b))
                    dl->AddLine(a, b, IM_COL32(100, 255, 100, 160), 2.0f);
            }
            for (auto& bp : g_bounce_points)
            {
                ImVec2 sc;
                if (w2s(bp, sc))
                    dl->AddCircleFilled(sc, 5, IM_COL32(255, 100, 50, 220));
            }
        }

        // ── Goal scored display ──
        if (settings::football::goal_scored && g_now < g_goal_display_until)
        {
            char gbuf[128];
            bool correct = settings::football::goal_actual_zone == settings::football::goal_predicted_zone;
            snprintf(gbuf, sizeof(gbuf), "GOAL! Actual: %s | Pred: %s %s",
                settings::football::goal_actual_zone.c_str(),
                settings::football::goal_predicted_zone.c_str(),
                correct ? "✓" : "✗");
            ImVec2 tsz = ImGui::CalcTextSize(gbuf);
            float gx = s.x / 2 - tsz.x / 2;
            float gy = 100.0f;
            for (int g = 3; g >= 0; g--)
                dl->AddRectFilled({ gx - 12 - g * 2, gy - 6 - g * 2 },
                    { gx + tsz.x + 12 + g * 2, gy + tsz.y + 6 + g * 2 },
                    IM_COL32(0, 200, 0, 60 - g * 12), (float)(8 + g * 2));
            dl->AddRectFilled({ gx - 12, gy - 6 }, { gx + tsz.x + 12, gy + tsz.y + 6 },
                correct ? IM_COL32(0, 140, 0, 220) : IM_COL32(180, 80, 0, 220), 6);
            dl->AddText({ gx, gy }, IM_COL32(255, 255, 255, 255), gbuf);
        }

        // ── Status text ──
        if (settings::football::is_tracking)
        {
            char buf[256];
            snprintf(buf, sizeof(buf), "Auto-Dive: %s", settings::football::status_text.c_str());
            ImVec2 sz = ImGui::CalcTextSize(buf);
            ImVec2 pos(s.x / 2 - sz.x / 2, s.y - 60);
            dl->AddRectFilled({ pos.x - 8, pos.y - 8 }, { pos.x + sz.x + 8, pos.y + sz.y + 8 }, IM_COL32(0,0,0,180), 6);
            dl->AddText(pos, IM_COL32(0,200,255,255), buf);
        }

        if (settings::football::autodive_enabled && !settings::football::is_tracking)
        {
            const char* msg = settings::football::status_text.c_str();
            if (*msg) {
                ImVec2 sz = ImGui::CalcTextSize(msg);
                ImVec2 pos(10, s.y - 80);
                dl->AddRectFilled({ pos.x - 4, pos.y - 4 }, { pos.x + sz.x + 4, pos.y + sz.y + 4 }, IM_COL32(0,0,0,160), 4);
                dl->AddText(pos, IM_COL32(255,200,0,255), msg);
            }
        }
    }
} // namespace football
