#include "Config.hpp"

bool Config::Read() {
	return GetInstance().ReadImpl();
}

bool Config::Write() {
	return GetInstance().WriteImpl();
}

bool Config::ReadProfile(const std::string& name) {
	cfg::settings::current_profile = name;
	return GetInstance().ReadImpl();
}

bool Config::WriteProfile(const std::string& name) {
	cfg::settings::current_profile = name;
	return GetInstance().WriteImpl();
}

std::vector<std::string> Config::ListProfiles() {
	std::vector<std::string> profiles;
	try {
		for (auto& entry : std::filesystem::directory_iterator(".")) {
			if (entry.path().extension() == ".json")
				profiles.push_back(entry.path().stem().string());
		}
	} catch (...) {}
	return profiles;
}

bool Config::ReadImpl() {
	std::string filename = cfg::settings::current_profile + ".json";
	std::ifstream f(filename);

	if (!f.good()) {
		LOGF(FATAL, "Configuration file does not exist, creating a new one");
		WriteImpl();
		return false;
	}

	json data;
	try {
		data = json::parse(f);
	}
	catch (const std::exception& e) {
		LOGF(FATAL, "Failed to parse configuration file");
		WriteImpl();
		return false;
	}

	if (data.empty())
		return false;

	try {
		cfg::enabled = data.value("enabled", true);

		cfg::esp::box = data["esp"].value("box", true);
		cfg::esp::box_3d = data["esp"].value("box_3d", false);
		cfg::esp::team = data["esp"].value("team", true);
		cfg::esp::armor = data["esp"].value("armor", true);
		cfg::esp::health = data["esp"].value("health", true);
		cfg::esp::spotted = data["esp"].value("spotted", false);
		cfg::esp::skeleton = data["esp"].value("skeleton", true);
		cfg::esp::head_tracker = data["esp"].value("head_tracker", true);
		cfg::esp::health_number = data["esp"].value("health_number", false);
		cfg::esp::tracers = data["esp"].value("tracers", false);
		cfg::esp::dropped_weapons = data["esp"].value("dropped_weapons", false);
		cfg::esp::grenade_esp = data["esp"].value("grenade_esp", false);
		cfg::esp::vision_ray = data["esp"].value("vision_ray", false);
		cfg::esp::visible_only = data["esp"].value("visible_only", false);

		cfg::esp::flags::name = data["esp"]["flags"].value("name", true);
		cfg::esp::flags::ping = data["esp"]["flags"].value("ping", false);
		cfg::esp::flags::money = data["esp"]["flags"].value("money", false);
		cfg::esp::flags::weapon = data["esp"]["flags"].value("weapon", false);
		cfg::esp::flags::ammo = data["esp"]["flags"].value("ammo", false);
		cfg::esp::flags::reloading = data["esp"]["flags"].value("reloading", false);
		cfg::esp::flags::scoped = data["esp"]["flags"].value("scoped", false);
		cfg::esp::flags::defusing = data["esp"]["flags"].value("defusing", false);
		cfg::esp::flags::flashed = data["esp"]["flags"].value("flashed", false);
		cfg::esp::flags::distance = data["esp"]["flags"].value("distance", false);

		const auto& col = data["esp"]["colors"];
		cfg::esp::colors::box_team = JsonToColor(col, "box_team", { 0.f, 1.f, 0.29f, 0.5f });
		cfg::esp::colors::box_enemy = JsonToColor(col, "box_enemy", { 1.f, 0.f, 0.f, 0.5f });
		cfg::esp::colors::box_visible = JsonToColor(col, "box_visible", { 0.f, 1.f, 0.f, 0.8f });
		cfg::esp::colors::box_occluded = JsonToColor(col, "box_occluded", { 1.f, 0.f, 0.f, 0.8f });
		cfg::esp::colors::skeleton_team = JsonToColor(col, "skeleton_team", { 0.f, 1.f, 0.f, 0.5f });
		cfg::esp::colors::skeleton_enemy = JsonToColor(col, "skeleton_enemy", { 1.f, 0.f, 0.f, 0.5f });
		cfg::esp::colors::skeleton_visible = JsonToColor(col, "skeleton_visible", { 0.f, 1.f, 0.f, 0.8f });
		cfg::esp::colors::skeleton_occluded = JsonToColor(col, "skeleton_occluded", { 1.f, 0.f, 0.f, 0.8f });
		cfg::esp::colors::tracker_team = JsonToColor(col, "tracker_team", { 1.f, 1.f, 1.f, 0.3f });
		cfg::esp::colors::tracker_enemy = JsonToColor(col, "tracker_enemy", { 1.f, 1.f, 1.f, 0.3f });
		cfg::esp::colors::tracer_team = JsonToColor(col, "tracer_team", { 0.f, 1.f, 0.f, 0.5f });
		cfg::esp::colors::tracer_enemy = JsonToColor(col, "tracer_enemy", { 1.f, 0.f, 0.f, 0.5f });
		cfg::esp::colors::dropped_weapon = JsonToColor(col, "dropped_weapon", { 1.f, 1.f, 0.f, 0.7f });
		cfg::esp::colors::grenade_color = JsonToColor(col, "grenade_color", { 1.f, 0.5f, 0.f, 0.8f });

		const auto& fcol = data["esp"]["colors"]["flags"];
		cfg::esp::colors::flags::flashed_team = JsonToColor(fcol, "flashed_team", { 1.f, 1.f, 1.f, 0.5f });
		cfg::esp::colors::flags::flashed_enemy = JsonToColor(fcol, "flashed_enemy", { 1.f, 1.f, 1.f, 0.8f });
		cfg::esp::colors::flags::reloading_team = JsonToColor(fcol, "reloading_team", { 1.f, 1.f, 1.f, 0.5f });
		cfg::esp::colors::flags::reloading_enemy = JsonToColor(fcol, "reloading_enemy", { 1.f, 1.f, 1.f, 0.8f });
		cfg::esp::colors::flags::defusing_team = JsonToColor(fcol, "defusing_team", { 1.f, 1.f, 1.f, 0.5f });
		cfg::esp::colors::flags::defusing_enemy = JsonToColor(fcol, "defusing_enemy", { 1.f, 1.f, 1.f, 0.8f });
		cfg::esp::colors::flags::scoped_team = JsonToColor(fcol, "scoped_team", { 1.f, 1.f, 1.f, 0.5f });
		cfg::esp::colors::flags::scoped_enemy = JsonToColor(fcol, "scoped_enemy", { 1.f, 1.f, 1.f, 0.8f });

		cfg::world::spectators::enabled = data["world"]["spectators"].value("enabled", true);
		cfg::world::spectators::detailed = data["world"]["spectators"].value("detailed", false);
		cfg::world::spectators::self_only = data["world"]["spectators"].value("self_only", true);
		cfg::world::spectators::pos = JsonToVec2(data["world"]["spectators"], "pos", {10.f, 100.f});

		cfg::world::bomb::location = data["world"]["bomb"].value("bomb_location", true);
		cfg::world::bomb::timer = data["world"]["bomb"].value("bomb_timer", true);
		cfg::world::bomb::damage_calc = data["world"]["bomb"].value("damage_calc", false);

		cfg::world::crosshair::enabled = data["world"]["crosshair"].value("enabled", false);

		cfg::world::radar::enabled = data["world"]["radar"].value("enabled", true);
		cfg::world::radar::no_rotate = data["world"]["radar"].value("no_rotate", false);
		cfg::world::radar::range = data["world"]["radar"].value("range", 2000.f);
		cfg::world::radar::pos = JsonToVec2(data["world"]["radar"], "pos", { 10.f, 10.f });
		cfg::world::radar::size = JsonToVec2(data["world"]["radar"], "size", { 200.f, 200.f });

		cfg::world::velocity::enabled = data["world"]["velocity"].value("enabled", false);
		cfg::world::velocity::sample_rate = data["world"]["velocity"].value("sample_rate", 10);
		cfg::world::velocity::sample_length = data["world"]["velocity"].value("sample_length", 5.f);
		cfg::world::velocity::pos = JsonToVec2(data["world"]["velocity"], "pos", { 10.f, 400.f });
		cfg::world::velocity::size = JsonToVec2(data["world"]["velocity"], "size", { 400.f, 100.f });

		cfg::aimbot::enabled = data["aimbot"].value("enabled", false);
		cfg::aimbot::fov = data["aimbot"].value("fov", 5.0f);
		cfg::aimbot::smooth = data["aimbot"].value("smooth", 5.0f);
		cfg::aimbot::hotkey = data["aimbot"].value("hotkey", 0x12);
		cfg::aimbot::bone = data["aimbot"].value("bone", 7);
		cfg::aimbot::rcs = data["aimbot"].value("rcs", false);
		cfg::aimbot::rcs_x = data["aimbot"].value("rcs_x", 1.4f);
		cfg::aimbot::rcs_y = data["aimbot"].value("rcs_y", 1.4f);
		cfg::aimbot::sensitivity = data["aimbot"].value("sensitivity", 2.0f);
		cfg::aimbot::start_bullet = data["aimbot"].value("start_bullet", 0);
		cfg::aimbot::flash_check = data["aimbot"].value("flash_check", false);
		cfg::aimbot::scope_check = data["aimbot"].value("scope_check", false);
		cfg::aimbot::stop_check = data["aimbot"].value("stop_check", false);
		cfg::aimbot::draw_fov = data["aimbot"].value("draw_fov", false);
		cfg::aimbot::fov_color = JsonToColor(data["aimbot"], "fov_color", { 1.f, 1.f, 1.f, 0.5f });
		cfg::aimbot::visible_only = data["aimbot"].value("visible_only", false);
		cfg::aimbot::humanize = data["aimbot"].value("humanize", false);
		cfg::aimbot::smooth_variance = data["aimbot"].value("smooth_variance", 0.3f);
		cfg::aimbot::jitter = data["aimbot"].value("jitter", 0.5f);
		cfg::aimbot::velocity_comp = data["aimbot"].value("velocity_comp", false);
		cfg::aimbot::velocity_comp_scale = data["aimbot"].value("velocity_comp_scale", 0.1f);
		cfg::aimbot::soft_aim = data["aimbot"].value("soft_aim", false);
		cfg::aimbot::soft_aim_max_move = data["aimbot"].value("soft_aim_max_move", 3.0f);
		cfg::aimbot::multi_bone = data["aimbot"].value("multi_bone", false);
		cfg::aimbot::fov_zoom_scale = data["aimbot"].value("fov_zoom_scale", true);
		cfg::aimbot::always_on = data["aimbot"].value("always_on", false);
		cfg::aimbot::angle_write = data["aimbot"].value("angle_write", false);
		if (data["aimbot"].contains("bone_priority") && data["aimbot"]["bone_priority"].is_array()) {
			cfg::aimbot::bone_priority.clear();
			for (auto& b : data["aimbot"]["bone_priority"])
				cfg::aimbot::bone_priority.push_back(b.get<int>());
		}

		cfg::misc::bhop = data["misc"].value("bhop", false);
		cfg::misc::bhop_hotkey = data["misc"].value("bhop_hotkey", (int)VK_SPACE);
		cfg::misc::anti_flash = data["misc"].value("anti_flash", false);
		cfg::misc::anti_smoke = data["misc"].value("anti_smoke", false);
		cfg::misc::anti_afk = data["misc"].value("anti_afk", false);
		cfg::misc::anti_afk_interval_s = data["misc"].value("anti_afk_interval_s", 60);
		cfg::misc::triggerbot::enabled = data["misc"]["triggerbot"].value("enabled", false);
		cfg::misc::triggerbot::hotkey = data["misc"]["triggerbot"].value("hotkey", VK_XBUTTON1);
		cfg::misc::triggerbot::fov = data["misc"]["triggerbot"].value("fov", 2.0f);
		cfg::misc::triggerbot::delay_ms = data["misc"]["triggerbot"].value("delay_ms", 50);
		cfg::misc::triggerbot::shot_delay = data["misc"]["triggerbot"].value("shot_delay", false);
		cfg::misc::triggerbot::shot_delay_ms = data["misc"]["triggerbot"].value("shot_delay_ms", 150);
		cfg::misc::triggerbot::draw = data["misc"]["triggerbot"].value("draw", false);
		cfg::misc::triggerbot::draw_color = JsonToColor(data["misc"]["triggerbot"], "draw_color", { 1.f, 1.f, 0.f, 0.5f });
		cfg::misc::triggerbot::always_on = data["misc"]["triggerbot"].value("always_on", false);
		cfg::misc::triggerbot::visible_only = data["misc"]["triggerbot"].value("visible_only", false);
		cfg::misc::auto_zeus::visible_only = data["misc"]["auto_zeus"].value("visible_only", false);
		cfg::misc::auto_knife::visible_only = data["misc"]["auto_knife"].value("visible_only", false);
		cfg::misc::strafe::helper = data["misc"]["strafe"].value("helper", false);
		cfg::misc::skin::enabled = data["misc"]["skin"].value("enabled", false);
		cfg::misc::skin::weapon_id = data["misc"]["skin"].value("weapon_id", (int)weapon_awp);
		cfg::misc::skin::skin_id = data["misc"]["skin"].value("skin_id", 0);
		cfg::misc::kill_sound::enabled = data["misc"]["kill_sound"].value("enabled", false);
		cfg::misc::hit_marker::enabled = data["misc"]["hit_marker"].value("enabled", true);
		cfg::misc::hit_marker::duration_ms = data["misc"]["hit_marker"].value("duration_ms", 500.f);
		cfg::misc::hit_marker::color = JsonToColor(data["misc"]["hit_marker"], "color", { 1.f, 0.2f, 0.2f, 1.f });
		cfg::misc::auto_zeus::enabled = data["misc"]["auto_zeus"].value("enabled", false);
		cfg::misc::auto_zeus::range = data["misc"]["auto_zeus"].value("range", 180.f);
		cfg::misc::auto_knife::enabled = data["misc"]["auto_knife"].value("enabled", false);
		cfg::misc::auto_knife::range = data["misc"]["auto_knife"].value("range", 80.f);
		cfg::misc::stats::enabled = data["misc"]["stats"].value("enabled", false);
		cfg::misc::auto_queue::enabled = data["misc"]["auto_queue"].value("enabled", false);
		cfg::misc::auto_queue::accept_match = data["misc"]["auto_queue"].value("accept_match", true);
		cfg::misc::clantag::enabled = data["misc"]["clantag"].value("enabled", false);
		if (data["misc"]["clantag"].contains("text") && data["misc"]["clantag"]["text"].is_string()) {
			std::string txt = data["misc"]["clantag"]["text"].get<std::string>();
			strncpy_s(cfg::misc::clantag::text, txt.c_str(), sizeof(cfg::misc::clantag::text) - 1);
		}
		cfg::misc::fake_ping::enabled = data["misc"]["fake_ping"].value("enabled", false);
		cfg::misc::fake_ping::ping = data["misc"]["fake_ping"].value("ping", 0);
		cfg::misc::rank_revealer::enabled = data["misc"]["rank_revealer"].value("enabled", false);

		cfg::settings::watermark = data["utils"].value("watermark", true);
		cfg::settings::streamproof = data["utils"].value("streamproof", false);
		cfg::settings::vsync = data["utils"].value("vsync", true);
		cfg::settings::free_cpu = data["utils"].value("free_cpu", true);
		cfg::settings::toggle_key = data["utils"].value("toggle_key", VK_F1);
		cfg::settings::panic_key = data["utils"].value("panic_key", VK_END);
		cfg::settings::keybinds_overlay = data["utils"].value("keybinds_overlay", true);
		cfg::settings::save_on_exit = data["utils"].value("save_on_exit", true);
	}
	catch (const std::exception& e) {
		LOGF(FATAL, "Failed to parse configuration");
		WriteImpl();
		return false;
	}

	LOGF(INFO, "Successfully parsed configuration");
	return true;
}

bool Config::WriteImpl() {
	std::string filename = cfg::settings::current_profile + ".json";
	std::ofstream f(filename);

	json data;

	data["enabled"] = cfg::enabled;

	data["esp"]["box"] = cfg::esp::box;
	data["esp"]["box_3d"] = cfg::esp::box_3d;
	data["esp"]["team"] = cfg::esp::team;
	data["esp"]["armor"] = cfg::esp::armor;
	data["esp"]["health"] = cfg::esp::health;
	data["esp"]["health_number"] = cfg::esp::health_number;
	data["esp"]["skeleton"] = cfg::esp::skeleton;
	data["esp"]["head_tracker"] = cfg::esp::head_tracker;
	data["esp"]["spotted"] = cfg::esp::spotted;
	data["esp"]["tracers"] = cfg::esp::tracers;
	data["esp"]["dropped_weapons"] = cfg::esp::dropped_weapons;
	data["esp"]["grenade_esp"] = cfg::esp::grenade_esp;
	data["esp"]["vision_ray"] = cfg::esp::vision_ray;
	data["esp"]["visible_only"] = cfg::esp::visible_only;

	data["esp"]["flags"]["name"] = cfg::esp::flags::name;
	data["esp"]["flags"]["ping"] = cfg::esp::flags::ping;
	data["esp"]["flags"]["money"] = cfg::esp::flags::money;
	data["esp"]["flags"]["scoped"] = cfg::esp::flags::scoped;
	data["esp"]["flags"]["weapon"] = cfg::esp::flags::weapon;
	data["esp"]["flags"]["ammo"] = cfg::esp::flags::ammo;
	data["esp"]["flags"]["reloading"] = cfg::esp::flags::reloading;
	data["esp"]["flags"]["flashed"] = cfg::esp::flags::flashed;
	data["esp"]["flags"]["defusing"] = cfg::esp::flags::defusing;
	data["esp"]["flags"]["distance"] = cfg::esp::flags::distance;

	data["world"]["spectators"]["enabled"] = cfg::world::spectators::enabled;
	data["world"]["spectators"]["detailed"] = cfg::world::spectators::detailed;
	data["world"]["spectators"]["self_only"] = cfg::world::spectators::self_only;
	Vec2ToJson(data["world"]["spectators"], "pos", cfg::world::spectators::pos);

	data["world"]["bomb"]["bomb_location"] = cfg::world::bomb::location;
	data["world"]["bomb"]["bomb_timer"] = cfg::world::bomb::timer;
	data["world"]["bomb"]["damage_calc"] = cfg::world::bomb::damage_calc;

	data["world"]["crosshair"]["enabled"] = cfg::world::crosshair::enabled;

	data["world"]["radar"]["enabled"] = cfg::world::radar::enabled;
	data["world"]["radar"]["no_rotate"] = cfg::world::radar::no_rotate;
	data["world"]["radar"]["range"] = cfg::world::radar::range;
	Vec2ToJson(data["world"]["radar"], "pos", cfg::world::radar::pos);
	Vec2ToJson(data["world"]["radar"], "size", cfg::world::radar::size);

	data["world"]["velocity"]["enabled"] = cfg::world::velocity::enabled;
	data["world"]["velocity"]["sample_rate"] = cfg::world::velocity::sample_rate;
	data["world"]["velocity"]["sample_length"] = cfg::world::velocity::sample_length;
	Vec2ToJson(data["world"]["velocity"], "pos", cfg::world::velocity::pos);
	Vec2ToJson(data["world"]["velocity"], "size", cfg::world::velocity::size);

	auto& col = data["esp"]["colors"];
	ColorToJson(col, "box_team", cfg::esp::colors::box_team);
	ColorToJson(col, "box_enemy", cfg::esp::colors::box_enemy);
	ColorToJson(col, "box_visible", cfg::esp::colors::box_visible);
	ColorToJson(col, "box_occluded", cfg::esp::colors::box_occluded);
	ColorToJson(col, "skeleton_team", cfg::esp::colors::skeleton_team);
	ColorToJson(col, "skeleton_enemy", cfg::esp::colors::skeleton_enemy);
	ColorToJson(col, "skeleton_visible", cfg::esp::colors::skeleton_visible);
	ColorToJson(col, "skeleton_occluded", cfg::esp::colors::skeleton_occluded);
	ColorToJson(col, "tracker_team", cfg::esp::colors::tracker_team);
	ColorToJson(col, "tracker_enemy", cfg::esp::colors::tracker_enemy);
	ColorToJson(col, "tracer_team", cfg::esp::colors::tracer_team);
	ColorToJson(col, "tracer_enemy", cfg::esp::colors::tracer_enemy);
	ColorToJson(col, "dropped_weapon", cfg::esp::colors::dropped_weapon);
	ColorToJson(col, "grenade_color", cfg::esp::colors::grenade_color);

	auto& fcol = col["flags"];
	ColorToJson(fcol, "flashed_team", cfg::esp::colors::flags::flashed_team);
	ColorToJson(fcol, "flashed_enemy", cfg::esp::colors::flags::flashed_enemy);
	ColorToJson(fcol, "reloading_team", cfg::esp::colors::flags::reloading_team);
	ColorToJson(fcol, "reloading_enemy", cfg::esp::colors::flags::reloading_enemy);
	ColorToJson(fcol, "defusing_team", cfg::esp::colors::flags::defusing_team);
	ColorToJson(fcol, "defusing_enemy", cfg::esp::colors::flags::defusing_enemy);
	ColorToJson(fcol, "scoped_team", cfg::esp::colors::flags::scoped_team);
	ColorToJson(fcol, "scoped_enemy", cfg::esp::colors::flags::scoped_enemy);

	data["aimbot"]["enabled"] = cfg::aimbot::enabled;
	data["aimbot"]["fov"] = cfg::aimbot::fov;
	data["aimbot"]["smooth"] = cfg::aimbot::smooth;
	data["aimbot"]["hotkey"] = cfg::aimbot::hotkey;
	data["aimbot"]["bone"] = cfg::aimbot::bone;
	data["aimbot"]["rcs"] = cfg::aimbot::rcs;
	data["aimbot"]["rcs_x"] = cfg::aimbot::rcs_x;
	data["aimbot"]["rcs_y"] = cfg::aimbot::rcs_y;
	data["aimbot"]["sensitivity"] = cfg::aimbot::sensitivity;
	data["aimbot"]["start_bullet"] = cfg::aimbot::start_bullet;
	data["aimbot"]["flash_check"] = cfg::aimbot::flash_check;
	data["aimbot"]["scope_check"] = cfg::aimbot::scope_check;
	data["aimbot"]["stop_check"] = cfg::aimbot::stop_check;
	data["aimbot"]["draw_fov"] = cfg::aimbot::draw_fov;
	ColorToJson(data["aimbot"], "fov_color", cfg::aimbot::fov_color);
	data["aimbot"]["visible_only"] = cfg::aimbot::visible_only;
	data["aimbot"]["humanize"] = cfg::aimbot::humanize;
	data["aimbot"]["smooth_variance"] = cfg::aimbot::smooth_variance;
	data["aimbot"]["jitter"] = cfg::aimbot::jitter;
	data["aimbot"]["velocity_comp"] = cfg::aimbot::velocity_comp;
	data["aimbot"]["velocity_comp_scale"] = cfg::aimbot::velocity_comp_scale;
	data["aimbot"]["soft_aim"] = cfg::aimbot::soft_aim;
	data["aimbot"]["soft_aim_max_move"] = cfg::aimbot::soft_aim_max_move;
	data["aimbot"]["multi_bone"] = cfg::aimbot::multi_bone;
	data["aimbot"]["fov_zoom_scale"] = cfg::aimbot::fov_zoom_scale;
	data["aimbot"]["always_on"] = cfg::aimbot::always_on;
	data["aimbot"]["angle_write"] = cfg::aimbot::angle_write;
	data["aimbot"]["bone_priority"] = cfg::aimbot::bone_priority;

	data["misc"]["bhop"] = cfg::misc::bhop;
	data["misc"]["bhop_hotkey"] = cfg::misc::bhop_hotkey;
	data["misc"]["anti_flash"] = cfg::misc::anti_flash;
	data["misc"]["anti_smoke"] = cfg::misc::anti_smoke;
	data["misc"]["anti_afk"] = cfg::misc::anti_afk;
	data["misc"]["anti_afk_interval_s"] = cfg::misc::anti_afk_interval_s;
	data["misc"]["triggerbot"]["enabled"] = cfg::misc::triggerbot::enabled;
	data["misc"]["triggerbot"]["hotkey"] = cfg::misc::triggerbot::hotkey;
	data["misc"]["triggerbot"]["fov"] = cfg::misc::triggerbot::fov;
	data["misc"]["triggerbot"]["delay_ms"] = cfg::misc::triggerbot::delay_ms;
	data["misc"]["triggerbot"]["shot_delay"] = cfg::misc::triggerbot::shot_delay;
	data["misc"]["triggerbot"]["shot_delay_ms"] = cfg::misc::triggerbot::shot_delay_ms;
	data["misc"]["triggerbot"]["draw"] = cfg::misc::triggerbot::draw;
	ColorToJson(data["misc"]["triggerbot"], "draw_color", cfg::misc::triggerbot::draw_color);
	data["misc"]["triggerbot"]["always_on"] = cfg::misc::triggerbot::always_on;
	data["misc"]["triggerbot"]["visible_only"] = cfg::misc::triggerbot::visible_only;
	data["misc"]["auto_zeus"]["visible_only"] = cfg::misc::auto_zeus::visible_only;
	data["misc"]["auto_knife"]["visible_only"] = cfg::misc::auto_knife::visible_only;
	data["misc"]["strafe"]["helper"] = cfg::misc::strafe::helper;
	data["misc"]["skin"]["enabled"] = cfg::misc::skin::enabled;
	data["misc"]["skin"]["weapon_id"] = cfg::misc::skin::weapon_id;
	data["misc"]["skin"]["skin_id"] = cfg::misc::skin::skin_id;
	data["misc"]["kill_sound"]["enabled"] = cfg::misc::kill_sound::enabled;
	data["misc"]["hit_marker"]["enabled"] = cfg::misc::hit_marker::enabled;
	data["misc"]["hit_marker"]["duration_ms"] = cfg::misc::hit_marker::duration_ms;
	ColorToJson(data["misc"]["hit_marker"], "color", cfg::misc::hit_marker::color);
	data["misc"]["auto_zeus"]["enabled"] = cfg::misc::auto_zeus::enabled;
	data["misc"]["auto_zeus"]["range"] = cfg::misc::auto_zeus::range;
	data["misc"]["auto_knife"]["enabled"] = cfg::misc::auto_knife::enabled;
	data["misc"]["auto_knife"]["range"] = cfg::misc::auto_knife::range;
	data["misc"]["stats"]["enabled"] = cfg::misc::stats::enabled;
	data["misc"]["auto_queue"]["enabled"] = cfg::misc::auto_queue::enabled;
	data["misc"]["auto_queue"]["accept_match"] = cfg::misc::auto_queue::accept_match;
	data["misc"]["clantag"]["enabled"] = cfg::misc::clantag::enabled;
	data["misc"]["clantag"]["text"] = std::string(cfg::misc::clantag::text);
	data["misc"]["fake_ping"]["enabled"] = cfg::misc::fake_ping::enabled;
	data["misc"]["fake_ping"]["ping"] = cfg::misc::fake_ping::ping;
	data["misc"]["rank_revealer"]["enabled"] = cfg::misc::rank_revealer::enabled;

	data["utils"]["watermark"] = cfg::settings::watermark;
	data["utils"]["streamproof"] = cfg::settings::streamproof;
	data["utils"]["vsync"] = cfg::settings::vsync;
	data["utils"]["free_cpu"] = cfg::settings::free_cpu;
	data["utils"]["toggle_key"] = cfg::settings::toggle_key;
	data["utils"]["panic_key"] = cfg::settings::panic_key;
	data["utils"]["keybinds_overlay"] = cfg::settings::keybinds_overlay;
	data["utils"]["save_on_exit"] = cfg::settings::save_on_exit;

	f << std::setw(4) << data << std::endl;
	f.close();

	LOGF(VERBOSE, "Writing configuration to file");
	return true;
}

color_t Config::JsonToColor(const json& parent, const std::string& key, const color_t& def) {
	if (!parent.contains(key) || !parent[key].is_array() || parent[key].size() != 4)
		return def;
	return color_t(
		parent[key][0].get<float>(),
		parent[key][1].get<float>(),
		parent[key][2].get<float>(),
		parent[key][3].get<float>()
	);
}

void Config::ColorToJson(json& parent, const std::string& key, const color_t& color) {
	parent[key] = { color.r, color.g, color.b, color.a };
}

Vec2_t Config::JsonToVec2(const json& parent, const std::string& key, const Vec2_t& def)
{
	if (!parent.contains(key) || !parent[key].is_array() || parent[key].size() != 2)
		return def;
	return Vec2_t{
		parent[key][0].get<float>(),
		parent[key][1].get<float>()
	};
}

void Config::Vec2ToJson(json& parent, const std::string& key, const Vec2_t& vec)
{
	parent[key] = { vec.x, vec.y };
}
