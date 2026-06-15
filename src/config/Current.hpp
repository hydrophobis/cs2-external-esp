#pragma once
#include "core/engine/types/Weapons.hpp"

namespace cfg {
	inline bool enabled = true;

	namespace esp {
		inline bool team = true;

		inline bool box = true;
		inline bool armor = true;
		inline bool health = true;
		inline bool skeleton = true;
		inline bool head_tracker = true;
		inline bool health_number = false;

		inline bool spotted = false;

		inline bool tracers = false;

		namespace flags {
			inline bool name = true;
			inline bool ping = true;
			inline bool weapon = false;
			inline bool ammo = false;
			inline bool reloading = false;
			inline bool defusing = false;
			inline bool money = false;
			inline bool flashed = false;
			inline bool scoped = false;
		}

		namespace colors {
			inline color_t box_team{ 0.f, 1.f, 0.29f, 0.5f };
			inline color_t box_enemy{ 1.f, 0.f, 0.f, 0.5f };

			inline color_t skeleton_team{ 0.f, 1.f, 0.f, 0.5f };
			inline color_t skeleton_enemy{ 1.f, 0.f, 0.f, 0.5f };

			inline color_t tracker_team{ 1.f, 1.f, 1.f, 0.3f };
			inline color_t tracker_enemy{ 1.f, 1.f, 1.f, 0.3f };

			inline color_t tracer_team{ 0.f, 1.f, 0.f, 0.5f };
			inline color_t tracer_enemy{ 1.f, 0.f, 0.f, 0.5f };

			namespace flags {
				inline color_t flashed_team{ 1.f, 1.f, 1.f, 0.5f };
				inline color_t flashed_enemy{ 1.f, 1.f, 1.f, 0.8f };

				inline color_t reloading_team{ 1.f, 1.f, 1.f, 0.5f };
				inline color_t reloading_enemy{ 1.f, 1.f, 1.f, 0.8f };

				inline color_t defusing_team{ 1.f, 1.f, 1.f, 0.5f };
				inline color_t defusing_enemy{ 1.f, 1.f, 1.f, 0.8f };

				inline color_t scoped_team{ 1.f, 1.f, 1.f, 0.5f };
				inline color_t scoped_enemy{ 1.f, 1.f, 1.f, 0.8f };
			}
			
		}

	}

	namespace world {
		namespace spectators {
			inline bool enabled = false;

			inline bool detailed = false;
			inline bool self_only = true;

			inline Vec2_t pos{ 10.f, 100.f };
		}

		namespace bomb {
			inline bool location = true;
			inline bool timer = true;
		}

		namespace crosshair {
			inline bool enabled = false;
		}

		namespace radar {
			inline bool enabled = true;
			inline bool no_rotate = false;
			inline float range = 2000.f;
			inline Vec2_t pos{ 10.f, 10.f };
			inline Vec2_t size{ 200.f, 200.f };
		}

		namespace velocity {
			inline bool enabled = false;
			inline int sample_rate = 35;
			inline float sample_length = 5.f;

			inline Vec2_t size{ 400.f, 100.f };
			inline Vec2_t pos{ 10.f, 400.f };
		}
	}

	namespace aimbot {
		inline bool enabled = false;
		inline float fov = 5.0f;
		inline float smooth = 5.0f;
		inline int hotkey = 0x12; // VK_MENU (Alt)
		inline int bone = 7; // head
		inline bool rcs = false;
		inline float rcs_x = 2.0f;
		inline float rcs_y = 2.0f;

		inline bool humanize = false;
		inline float smooth_variance = 0.3f;
		inline float jitter = 0.5f;

		inline bool velocity_comp = false;
		inline float velocity_comp_scale = 0.1f;

		// Soft aim: only nudge when crosshair is already close; cap move per tick
		inline bool soft_aim = false;
		inline float soft_aim_max_move = 3.0f; // max pixels moved per tick
	}

	namespace misc {
		inline bool bhop = false;
		inline bool anti_flash = false;

		namespace triggerbot {
			inline bool enabled = false;
			inline int hotkey = VK_XBUTTON1; // Mouse4
			inline float fov = 2.0f;         // screen-space radius in pixels to trigger
			inline int delay_ms = 50;        // ms delay after target detected before clicking
		}

		namespace strafe {
			inline bool helper = false;  // corrects A/D key to the side that gains speed
			inline bool autostrafe = false; // full auto-strafe while in air
		}

		namespace skin {
			inline bool enabled = false;
			inline int weapon_id = weapon_awp; // weapon to reskin
			inline int skin_id = 0;            // item definition index to replace with
		}
	}

namespace settings {
		inline bool watermark = true;
		inline bool streamproof = false;
		inline bool vsync = false;
		inline bool free_cpu = true;
		inline int toggle_key = VK_F1; // hotkey to toggle cfg::enabled
	}


	// Not stored, just for testing
	namespace dev {
		inline bool console = true;
		inline int open_menu_key = false;
		inline int cache_refresh_rate = 5;
		inline bool force_show_flags = false;
	}
}