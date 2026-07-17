#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <sdk/sdk.h>
#include <cache/cache.h>

namespace settings
{
	namespace aimbot
	{
		inline bool enabled{ false };
		inline int keybind{ 0 };
		inline int activation_mode{ 1 };

		inline int mode{ 1 };
		inline int aim_type{ 0 };

		inline int target_part{ 1 };
		inline bool air_part_enabled{ false };
		inline int air_part{ 1 };

		inline float fov{ 100.f };
		inline bool use_fov{ false };
		inline bool draw_fov{ false };
		inline float fov_circle_colour[4]{ 1.f, 1.f, 1.f, 1.f };
		inline float fov_outline_colour[4]{ 0.f, 0.f, 0.f, 1.f };

		inline bool smoothing{ false };
		inline float smoothingx{ 10.f };
		inline float smoothingy{ 10.f };

		inline bool enable_prediction{ false };
		inline float prediction_x{ 10.f };
		inline float prediction_y{ 10.f };

		inline bool air_prediction_enabled{ false };
		inline float air_prediction_x{ 10.f };
		inline float air_prediction_y{ 10.f };

		inline int smoothing_style{ 0 };

		inline bool teamcheck{ false };
		inline bool knock_check{ false };
		inline bool wallcheck{ false };
		inline bool grabbed_check{ false };
		inline bool forcefield_check{ false };
		inline bool npc_check{ false };
		inline bool reloading_check{ false };
		inline bool no_ammo_check{ false };
		inline bool sticky_aim{ false };
		inline bool anti_shake{ false };

		inline bool health_check_enabled{ false };
		inline float min_health{ 0.0f };

		inline int fov_shape{ 0 };

		namespace triggerbot
		{
			inline bool enabled{ false };
			inline int keybind{ 0 };
			inline int activation_mode{ 1 };
			inline int fire_mode{ 0 };
			inline float clicks_per_second{ 10.f };
			inline float hold_duration{ 0.15f };
			inline float reaction_ms{ 80.f };
			inline float max_distance{ 300.f };
			inline bool max_distance_enabled{ false };
			inline bool wallcheck{ false };
			inline bool headshot_only{ false };
			inline bool knife_check{ false };
			inline bool reloading_check{ false };
			inline bool no_ammo_check{ false };
			inline bool crew_check{ false };
			inline float gun_switch_delay{ 0.0f };
			inline float hitbox_allowance{ 0.0f };
			inline bool trigger_when_host_shoots{ false };
			inline float host_delay{ 0.0f };
			inline float host_duration{ 0.0f };
			inline int host_player_index{ -1 };
			inline int fov_perspective{ 0 };
		}

		inline bool offset_enabled{ false };
		inline float offset_x{ 0.0f };
		inline float offset_y{ 0.0f };
		inline float ai_silent_y_offset{ 0.5f };
	}

	namespace rage
	{
		inline bool hitsounds{ false };
		inline int hitsound_type{ 0 };
		inline int hitsound_method{ 0 };
		inline bool rapidfire{ false };
		inline bool noclip{ false };
		inline int noclip_keybind{ 0 };
		inline int noclip_activation_mode{ 1 };
		inline bool hit_tracers{ false };
		inline float hit_tracers_color[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
		inline float hit_tracers_duration{ 1.0f };

		namespace hipheight
		{
			inline bool enabled{ false };
			inline float height{ 2.0f };
		}

		namespace hitbox_expander
		{
			inline bool enabled{ false };
			inline bool expand_all_parts{ false };
			inline float size_x{ 2.2f };
			inline float size_y{ 2.2f };
			inline float size_z{ 1.2f };
			inline bool knock_check{ false };
			inline bool anti_collision{ true };
			inline float head_size_x{ 2.5f };
			inline float head_size_y{ 2.5f };
			inline float head_size_z{ 2.0f };
			inline float torso_size_x{ 2.2f };
			inline float torso_size_y{ 2.2f };
			inline float torso_size_z{ 1.2f };
			inline float limb_size_x{ 1.8f };
			inline float limb_size_y{ 1.8f };
			inline float limb_size_z{ 1.0f };
		}

		namespace spin360
		{
			inline bool enabled{ false };
			inline float speed{ 5.0f };
			inline int keybind{ 0 };
			inline int activation_mode{ 1 };
		}

		namespace playerlist
		{
			inline bool enabled{ false };
			inline int selected_index{ -1 };
			inline std::uint64_t selected_address{ 0 };
			inline std::string selected_name{};

			inline bool is_spectating{ false };
			inline std::string spectate_target_name{};
			inline std::uint64_t original_camera_subject{ 0 };

			inline math::vector3 saved_position{};
			inline bool has_saved_position{ false };

			inline std::unordered_set<std::string> whitelist{};

			inline std::string target_name{};
			inline std::uint64_t target_address{ 0 };
		}

		inline bool is_whitelisted(const std::string& name)
		{
			return playerlist::whitelist.count(name) > 0;
		}
	}

	namespace infinite_ammo
	{
		inline bool enabled{ false };
		inline bool universal_mode{ true };
		inline int ammo_value{ 30 };
		inline bool preserve_mag{ true };
		inline bool no_reload{ false };
		inline int detection_method{ 0 };
	}

	namespace lua_executor
	{
		inline bool enabled{ false };
		inline bool auto_inject{ false };
		inline char script_buffer[65536]{ 0 };
		inline int execution_mode{ 0 };
		inline bool bypass_detection{ true };
		inline int inject_delay_ms{ 2000 };
		inline bool clear_on_execute{ false };
	}

	namespace desync
	{
		inline bool enabled{ false };
		inline int keybind{ 0 };
		inline int keybind_mode{ 1 };

		namespace visualizer
		{
			inline bool enabled{ false };
			inline float color[4]{ 0.0f, 0.8f, 1.0f, 1.0f };
			inline float thickness{ 2.0f };
			inline bool pulse{ true };
		}
	}

	namespace magicbullet
	{
		inline bool enabled{ false };
		inline int keybind{ 0 };
		inline int activation_mode{ 1 };
		inline int target_source{ 2 };
		inline float offset_distance{ 5.0f };
		inline int hold_ms{ 50 };
		inline int tp_iterations{ 8000 };
	}

	namespace custom_entities
	{
		struct custom_entity_t
		{
			rbx::instance_t instance;
			std::string name;
			std::string container_path;
			float distance = 0.f;
			std::unordered_map<std::string, cache::part_data_t> parts;
			cache::part_data_t root_part;
			cache::part_data_t head;
			bool enabled = true;
		};

		struct custom_container_t
		{
			std::string path;
			std::string name;
			bool enabled = true;
			std::vector<custom_entity_t> entities;
		};

		inline std::vector<custom_container_t> containers;
		inline std::string current_input = "Workspace.Bots";
		inline bool show_custom_entities = false;
		inline bool auto_refresh = false;
		inline float refresh_rate = 0.005f;
	}

	namespace silentaim
	{
		inline bool enabled{ false };
		inline int keybind{ 0 };
		inline int activation_mode{ 1 };

		inline int target_part{ 1 };
		inline float closest_point_scale{ 1.0f };

		inline float fov{ 100.f };
		inline bool use_fov{ false };
		inline bool draw_fov{ false };
		inline bool attach_fov_to_target{ false };
		inline float fov_circle_colour[4]{ 1.f, 1.f, 1.f, 1.f };
		inline float fov_outline_colour[4]{ 0.f, 0.f, 0.f, 1.f };
		inline int fov_shape{ 0 };

		inline bool enable_prediction{ false };
		inline float prediction_x{ 10.f };
		inline float prediction_y{ 10.f };

		inline bool sticky_aim{ false };
		inline bool auto_switch{ false };
		inline bool spoof_mouse{ true };
		inline int silent_mode{ 0 }; // 0=Camlock (viewport), 1=3rd Person (mouse)
		inline bool use_aimbot_target{ false };

		inline bool teamcheck{ false };
		inline bool wallcheck{ false };
		inline bool grabbed_check{ false };
		inline bool forcefield_check{ false };
		inline bool guncheck{ false };
		inline bool knock_check{ false };
		inline bool npc_check{ false };

		inline int priorities{ 0 };
		inline bool health_check_enabled{ false };
		inline float min_health{ 0.0f };

		inline bool hitchance_enabled{ false };
		inline float hitchance{ 100.0f };

		inline bool draw_target_dot{ false };
		inline float target_dot_color[4]{ 1.f, 0.f, 0.f, 1.f };
		inline float target_dot_size{ 4.0f };

		inline bool draw_snap_line{ false };
		inline float snap_line_color[4]{ 1.f, 1.f, 1.f, 1.f };

		namespace triggerbot
		{
			inline bool enabled{ false };
			inline int keybind{ 0 };
			inline int activation_mode{ 1 };
			inline int fire_mode{ 0 };
			inline float clicks_per_second{ 10.f };
			inline float hold_duration{ 0.15f };
			inline float reaction_ms{ 80.f };
			inline float max_distance{ 300.f };
			inline bool max_distance_enabled{ false };
			inline bool wallcheck{ false };
			inline bool headshot_only{ false };
		}
	}

	namespace visuals
	{
		inline bool radar_enabled{ false };
		inline float radar_size{ 0.f };

		inline bool enable_enemies{ false };
		inline bool enable_client{ false };

		inline bool box{ false };
		inline int box_type{ 0 };
		inline float box_color[4]{ 1.f, 1.f, 1.f, 1.f };
		inline bool box_fill{ false };
		inline float box_fill_color[4]{ 0.2f, 0.2f, 0.2f, 0.3f };

		inline bool name{ false };
		inline int name_type{ 0 };
		inline int name_display_type{ 0 };
		inline float name_color[4]{ 1.f, 1.f, 1.f, 1.f };
		inline float name_color_blend_start[4]{ 1.f, 1.f, 1.f, 1.f };
		inline float name_color_blend_end[4]{ 0.f, 0.f, 1.f, 1.f };
		inline bool blend{ false };
		inline bool avatar{ false };

		inline bool healthbar{ false };
		inline float healthbar_color[4]{ 0.f, 1.f, 0.f, 1.f };
		inline bool health_based_healthbar{ false };
		inline bool gradient_healthbar{ false };
		inline float gradient_healthbar_color_start[4]{ 1.f, 1.f, 1.f, 1.f };
		inline float gradient_healthbar_color_end[4]{ 0.f, 1.f, 0.f, 1.f };
		inline bool health_percent{ false };
		inline float health_percent_color[4]{ 1.f, 1.f, 1.f, 1.f };

		inline bool armorbar{ false };
		inline float armorbar_color[4]{ 0.275f, 0.627f, 1.f, 1.f };

		inline bool distance{ false };
		inline int distance_measurement{ 0 };
		inline float distance_color[4]{ 1.f, 1.f, 1.f, 1.f };

		inline bool tool{ false };
		inline float tool_color[4]{ 1.f, 1.f, 1.f, 1.f };

		inline int esp_font{ 0 };
		inline bool local_player{ false };
		inline bool chams{ false };
		inline int chams_type{ 1 };
		inline float chams_fill_color[4]{ 1.f, 0.f, 0.f, 0.5f };
		inline float chams_outline_color[4]{ 1.f, 1.f, 1.f, 1.f };
		inline bool chams_fill_enabled{ true };
		inline bool chams_outline_enabled{ true };

		inline bool target_warning_icon{ false };
		inline float target_warning_icon_size{ 24.0f };

		inline bool flags{ false };
		inline float flags_state_colour[4]{ 1.f, 1.f, 1.f, 1.f };

		inline bool client_box{ false };
		inline float client_box_color[4]{ 1.f, 1.f, 1.f, 1.f };
		inline bool client_box_fill{ false };
		inline float client_box_fill_color[4]{ 0.2f, 0.2f, 0.2f, 0.3f };

		inline bool client_name{ false };
		inline float client_name_color[4]{ 1.f, 1.f, 1.f, 1.f };
		inline bool client_avatar{ false };

		inline bool client_healthbar{ false };
		inline float client_healthbar_color[4]{ 0.f, 1.f, 0.f, 1.f };
		inline bool client_health_percent{ false };
		inline float client_health_percent_color[4]{ 1.f, 1.f, 1.f, 1.f };

		inline bool client_armorbar{ false };
		inline float client_armorbar_color[4]{ 0.275f, 0.627f, 1.f, 1.f };

		inline bool client_distance{ false };
		inline float client_distance_color[4]{ 1.f, 1.f, 1.f, 1.f };

		inline bool client_tool{ false };
		inline float client_tool_color[4]{ 1.f, 1.f, 1.f, 1.f };

		inline bool client_chams{ false };
		inline float client_chams_fill_color[4]{ 1.f, 0.f, 0.f, 0.5f };
		inline float client_chams_outline_color[4]{ 1.f, 1.f, 1.f, 1.f };

		inline bool client_flags{ false };
		inline float client_flags_state_colour[4]{ 1.f, 1.f, 1.f, 1.f };

		inline bool client_headless{ false };
		inline bool client_korblox{ false };

		inline bool debug_wallcheck{ false };

		inline bool view_hitbox{ false };
		inline float view_hitbox_color[4]{ 1.f, 0.f, 0.f, 1.f };

		inline float fade_in_speed{ 5.0f };
		inline float fade_out_speed{ 5.0f };

		inline bool knock_check{ false };
		inline bool teamcheck{ false };
		inline bool use_team_color{ false };
		inline bool ignore_whitelisted{ false };

		inline bool max_distance_enabled{ false };
		inline float max_distance{ 500.f };

		inline int esp_keybind{ 0 };
		inline int esp_keybind_mode{ 1 };

		inline bool mm2_esp{ false };

		inline bool hit_tracers_enabled{ false };
		inline int hit_tracers_method{ 0 };
		inline float hit_tracers_color[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
		inline float hit_tracers_duration{ 1.0f };

		inline bool skeleton{ false };
		inline float skeleton_color[4]{ 1.f, 1.f, 1.f, 1.f };

		inline bool custom_preview_show_esp{ true };
		inline bool custom_preview_rotate{ true };
		inline float custom_preview_rotation_speed{ 1.0f };
		inline bool character_preview{ true };
		inline float preview_size{ 200.0f };
	}

	namespace movement
	{
		namespace speedhack
		{
			inline bool enabled{ false };
			inline int mode{ 0 };
			inline float speed{ 50.0f };
			inline int keybind{ 0 };
			inline int activation_mode{ 1 };
		}

		namespace jumphack
		{
			inline bool enabled{ false };
			inline float value{ 50.0f };
			inline int keybind{ 0 };
			inline int activation_mode{ 1 };
		}

		namespace jump_velocity_boost
		{
			inline bool enabled{ false };
			inline float jp_boost{ 50.0f }; // absolute JumpPower value (50 = default)
			inline int keybind{ 0 };
			inline int activation_mode{ 1 }; // 0=hold, 1=toggle
		}

		namespace nojumpcooldown
		{
			inline bool enabled{ false };
		}

		namespace flyhack
		{
			inline bool enabled{ false };
			inline int mode{ 0 };
			inline float speed{ 50.0f };
			inline int keybind{ 0 };
			inline int activation_mode{ 1 };
		}

		namespace tickrate
		{
			inline bool enabled{ false };
			inline float value{ 240.0f };
		}

		namespace orbit
		{
			inline bool enabled{ false };
			inline int orbit_type{ 0 };
			inline float speed{ 30.0f };
			inline float radius{ 10.0f };
			inline float height_offset{ 10.0f };
			inline bool spectate_target{ false };
			inline bool randomize{ false };
			inline float randomize_x{ 5.0f };
			inline float randomize_y{ 5.0f };
		}

		namespace gravity
		{
			inline bool enabled{ false };
			inline float value{ 196.2f };
		}
	}

	namespace ui
	{
		inline bool watermark{ true };
		inline bool keybinds{ true };

		inline int accent_color{ 0 };
		inline float custom_accent_color[4]{ 0.33f, 0.26f, 0.39f, 1.0f };
		inline float menu_opacity{ 1.0f };
		inline bool rounded_corners{ true };
		inline bool compact_mode{ false };
		inline bool show_icons{ true };
		inline bool anim_transitions{ true };
		inline float animation_speed{ 1.0f };
	}

	namespace watermark
	{
		inline bool show_cheat_name{ true };
		inline bool show_display_name{ false };
		inline bool show_username{ false };
		inline bool show_fps{ true };
		inline bool show_server_ip{ false };
		inline bool show_ping{ false };

		inline int separator_type{ 0 };
		inline float text_color[4]{ 1.f, 1.f, 1.f, 1.f };
		inline bool rainbow{ false };
		inline float rainbow_speed{ 1.0f };
		inline float pos_x{ 20.0f };
		inline float pos_y{ 100.0f };
		inline bool pos_initialized{ false };
	}

	namespace menu
	{
		inline int menu_keybind{ VK_DELETE };
		inline bool watermark{ false };
		inline bool streamproof{ true };
		inline bool cursor_streamproof{ false };
		inline bool auto_inject{ false };
		inline bool unload{ false };
		inline bool vsync{ false };
		inline int frame_limiter_ms{ 6 };
		inline bool performance_mode{ false };
	}

	namespace lighting
	{
		namespace fog
		{
			inline bool enabled{ false };
			inline float fog_start{ 0.0f };
			inline float fog_end{ 500.0f };
			inline float fog_r{ 0.75f };
			inline float fog_g{ 0.75f };
			inline float fog_b{ 0.75f };
		}

		namespace shadows
		{
			inline bool disable{ false };
		}

		namespace clocktime
		{
			inline bool enabled{ false };
			inline float clock_time{ 12.0f };
		}

		namespace skybox
		{
			inline bool enabled{ false };
			inline int preset_index{ 0 };
		}

		namespace exposure
		{
			inline bool enabled{ false };
			inline float exposure{ 0.f };
		}
	}

	namespace exploits
	{
		namespace antiafk
		{
			inline bool enabled{ false };
		}

		namespace freezeplayer
		{
			inline bool enabled{ false };
			inline int keybind{ 0 };
			inline int activation_mode{ 1 };
		}
	}

	namespace client
	{
		namespace fpscaps
		{
			inline bool enabled{ false };
		}
	}

	namespace extras
	{
		inline bool crosshair{ false };
		inline float crosshair_size{ 8.0f };
		inline float crosshair_gap{ 4.0f };
		inline float crosshair_thickness{ 2.0f };
		inline float crosshair_color[4]{ 0.0f, 1.0f, 0.0f, 1.0f };
		inline bool bhop{ false };
		inline bool esp_tracers{ false };
		inline float esp_tracers_color[4]{ 1.0f, 1.0f, 1.0f, 1.0f };
		inline bool notifications{ true };
		inline bool keybind_list{ true };
		inline bool thirdperson{ false };
		inline float custom_fov{ 70.0f };
		inline bool custom_fov_enabled{ false };
		inline bool instant_respawn{ false };
		inline bool ragetp{ false };
		inline int ragetp_key{ 0 };
		inline float ragetp_range{ 200.0f };
		inline bool ragetp_autoshoot{ false };
		inline bool ragetp_magicbullet{ true };
		inline bool autofarm{ false };
		inline float autofarm_range{ 30.0f };
		inline bool anti_lock{ false };
		inline bool no_register{ false };
		inline bool macro_glitch{ false };
		inline int macro_glitch_key{ 0 };
		inline float macro_normal_speed{ 16.0f };
		inline float macro_boost_speed{ 120.0f };
		inline float macro_interval{ 0.05f };
	}

namespace football
{
	inline bool autodive_enabled{ false };

	inline bool auto_m2{ false };
	inline bool auto_m2_from_key{ false };

		inline bool show_prediction{ false };
	inline bool show_zones{ true };

	// Mode A (normal) settings
	inline float mode_a_dive_offset{ 0.45f };
	inline float mode_a_top_threshold{ 4.8f };

	// Mode B (sound-triggered) settings
	inline bool mode_b_enabled{ false };
	inline bool mode_b_active{ false };
	inline float mode_b_dive_offset{ 0.60f };
	inline float mode_b_top_threshold{ 4.8f };
	inline float mode_b_duration{ 2.0f };

	// Mode B debug / control
	inline bool force_mode_b_active{ false };
	inline double mode_b_time_left{ 0.0 };

	// Effect trigger debug
	inline bool mode_b_effect_enabled{ true };
	inline uint64_t mode_b_effects_addr{ 0 };
	inline int mode_b_effects_child_count{ 0 };
	inline bool mode_b_found_jump{ false };
	inline std::string mode_b_last_effect_name{};
	inline uint64_t mode_b_last_effect_addr{ 0 };
	inline double mode_b_last_effect_time{ 0.0 };
	inline std::string mode_b_trigger_source{};

	// Zone thresholds
	inline float mid_top_threshold{ 4.8f };
	inline float mid_iframe_top_threshold{ 4.8f };

	// Zone sizing (matches script's leftGoalSize * zoneScale)
	inline float zone_scale_x{ 1.5f };
	inline float zone_scale_y{ 3.0f };

	// Panel config (camera-relative)
	inline float panel_behind_dist{ -0.6f };
	inline float panel_height_adj{ 0.0f };

	// Dive keys
	inline int dive_key_space{ VK_SPACE };
	inline int dive_key_left{ 0x43 }; // C
	inline int dive_key_right{ 0x5A }; // Z
	inline int dive_key_middle{ 0 };   // unbound by default
	inline int dive_key_m2{ 0 };       // unbound by default

	// Random dive
	inline bool random_dive_enabled{ false };
	inline float random_dive_offset_reduction{ 0.50f }; // 50%

	// Dive
	inline float dive_cooldown{ 0.75f };

	// Feature keybinds
	inline int autodive_key{ 0 };
	inline int autodive_key_mode{ 0 }; // 0 = hold, 1 = toggle
	inline int auto_m2_key{ 0xDB };
	inline int auto_m2_key_mode{ 1 };
	inline int panel_vis_key{ 0x4B }; // K (moved here, was below)

	// Auto M2 separate settings
	inline float auto_m2_dive_offset{ 0.49f };
	inline float auto_m2_mode_b_dive_offset{ 0.60f };
	inline float auto_m2_cooldown{ 0.75f };
	inline float m2_jump_delay{ 0.030f }; // delay between jump key and M2 on TOP zones (Lua: m2JumpDelay)

	// Post Guard
	inline bool post_guard_enabled{ false };
	inline float post_guard_distance{ 18.0f };
	inline float post_guard_offset_bonus{ 0.05f };

	// Physics constants
	inline float gravity_mult{ 6.0f };
	inline float ground_y{ 0.758f };
	inline float bounce_vel_y{ 0.4f };
	inline float bounce_friction{ 0.70f };
	inline float rolling_friction{ 0.995f };
	inline float stop_velocity{ 0.1f };
	inline float min_bounce_velocity{ 1.0f };

	// Zone fractions
	inline float mid_fraction{ 0.32f };
	inline float mid_side_fraction{ 0.27f };

	// Ball name fallback
	inline std::string ball_name{ "Ball" };

	// Prediction visualization
	inline bool show_path{ false };
	inline bool show_panel{ false };
	// (panel_vis_key moved to feature keybinds section above)

	// Goal-scored detection
	inline bool goal_scored{ false };
	inline std::string goal_actual_zone{};
	inline std::string goal_predicted_zone{};
	inline double goal_time{ 0.0 };

	// State (updated by football::tick)
	inline bool is_tracking{ false };
	inline std::string current_zone{ "---" };
	inline float predicted_time{ 0.0f };
	inline std::string status_text{ "Idle" };
}

namespace globals
{
	inline bool is_game_active{ true };
	inline std::string offset_validation_result{};
}
}
