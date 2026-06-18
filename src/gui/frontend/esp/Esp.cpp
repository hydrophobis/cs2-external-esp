#include "Esp.hpp"

#include <cmath>
#include <algorithm>
#include <chrono>
#include "gui/renderer/Renderer.hpp"
#include "assets/fonts/WeaponIcons.h"
#include "assets/fonts/Icons.h"
#include "core/engine/features/Misc.hpp"

bool Esp::Init() {
	return GetInstance().InitImpl();
}

void Esp::Render() {
	return GetInstance().RenderImpl();
}

bool Esp::InitImpl() {
	auto& io = ImGui::GetIO();

	ImFontConfig cfg{};
	cfg.FontDataOwnedByAtlas = false;

	this->font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 12.0f, &cfg);

	ImFontConfig icon_cfg{};
	icon_cfg.FontDataOwnedByAtlas = false;

	this->font_merged_icons = io.Fonts->AddFontFromMemoryTTF(
		weapon_icon_font,
		weapon_icon_font_len,
		16.0f,
		&icon_cfg
		);

	icon_cfg.MergeMode = true;

	static const ImWchar general_ranges[] = { 0xE100, 0xE108, 0 };
	io.Fonts->AddFontFromMemoryTTF(
		icons_font,
		icons_font_len,
		16.0f,
		&icon_cfg,
		general_ranges
	);

	return true;
}

// gradient
static ImU32 HealthColor(int health) {
	float t = std::clamp(health / 100.f, 0.f, 1.f);
	float r, g;
	if (t > 0.5f) {
		float s = (t - 0.5f) * 2.f; // 1=green, 0=yellow
		r = 1.f - s;
		g = 1.f;
	} else {
		float s = t * 2.f; // 0=red, 1=yellow
		r = 1.f;
		g = s;
	}
	return IM_COL32((int)(r * 255), (int)(g * 255), 0, 255);
}

void Esp::RenderImpl() {
	if (!cfg::enabled)
		return;

	auto snapshot = Cache::CopySnapshot();
	auto& game = snapshot.game;
	auto& bomb = snapshot.bomb;
	auto& local = snapshot.local;
	auto& globals = snapshot.globals;
	auto& players = snapshot.players;
	auto& worldEntities = snapshot.world_entities;

	ImGui::PushFont(this->font);

	this->io = ImGui::GetIO();
	this->d = ImGui::GetBackgroundDrawList();

	this->matrix = game.view_matrix;

	RenderBomb(local, bomb);

	for (auto& player : players) {
		if (!player.alive)
			continue;

		if (player.localplayer)
			continue;

		bool mate = player.team == local.team;

		if (!cfg::esp::team && mate)
			continue;

		if (cfg::esp::spotted && !player.spotted)
			continue;

		if (cfg::esp::visible_only && !player.visible)
			continue;

		// Are we spectating the player in first person? then dont render
		if (
			local.observer_services.target == player.pawn_controller_addr
			&& local.observer_services.mode == ObserverMode::First
			)
			continue;

		RenderPlayerTracers(local, player, mate);
		RenderPlayerVisionRay(local, player, mate);
		RenderPlayer(player, mate);
	}

	// World entities
	RenderDroppedWeapons(worldEntities, local);
	RenderGrenades(worldEntities, local);

	RenderCrosshair(local);
	RenderAimbotFOV();
	RenderTriggerbotFOV();
	RenderHitMarkers();
	ImGui::PopFont();
}

void Esp::RenderPlayer(Player player, bool mate) {
	std::pair<Vec2_t, Vec2_t> bounds;
	if (!player.GetBounds(matrix, io.DisplaySize, bounds))
		return;

	// Causes hp bars across the screen when they respawn
	if (!player.alive)
		return;

	if (cfg::esp::box) {
		if (cfg::esp::box_3d) {
			Vec3_t foot = player.pos;
			Vec3_t headPos = player.bone_list.empty() ? (player.pos + Vec3_t(0, 0, 75.f)) : player.bone_list[bone_index::head].pos;
			headPos.z += 10.f; // extend above head bone

			float halfW = (bounds.second.x - bounds.first.x) * 0.5f;
			float height3d = headPos.z - foot.z;
			float width3d = height3d * 0.4f; // approximate width

			Vec3_t mins = foot - Vec3_t(width3d * 0.5f, width3d * 0.5f, 0);
			Vec3_t maxs = foot + Vec3_t(width3d * 0.5f, width3d * 0.5f, height3d);

			Vec3_t corners[8] = {
				Vec3_t(mins.x, mins.y, mins.z),
				Vec3_t(maxs.x, mins.y, mins.z),
				Vec3_t(maxs.x, maxs.y, mins.z),
				Vec3_t(mins.x, maxs.y, mins.z),
				Vec3_t(mins.x, mins.y, maxs.z),
				Vec3_t(maxs.x, mins.y, maxs.z),
				Vec3_t(maxs.x, maxs.y, maxs.z),
				Vec3_t(mins.x, maxs.y, maxs.z),
			};

			Vec2_t screen[8];
			bool allValid = true;
			for (int i = 0; i < 8; i++) {
				if (!matrix.wts(corners[i], io.DisplaySize, screen[i], false)) {
					allValid = false;
					break;
				}
			}

			if (allValid) {
				auto color = player.visible
					? cfg::esp::colors::box_visible
					: (mate ? cfg::esp::colors::box_team : cfg::esp::colors::box_enemy);

				// Bottom face: 0-1-2-3
				// Top face: 4-5-6-7
				// Verticals: 0-4, 1-5, 2-6, 3-7
				int edges[12][2] = {
					{0,1},{1,2},{2,3},{3,0}, // bottom
					{4,5},{5,6},{6,7},{7,4}, // top
					{0,4},{1,5},{2,6},{3,7}  // verticals
				};

				for (auto& edge : edges) {
					d->AddLine(screen[edge[0]], screen[edge[1]], ImColor(color), 1.5f);
				}
			}
		} else {
			// 2D box with visibility coloring
			auto color = player.visible
				? cfg::esp::colors::box_visible
				: (mate ? cfg::esp::colors::box_team : cfg::esp::colors::box_enemy);

			d->AddRect(
				bounds.first,
				bounds.second,
				ImColor(color)
			);
		}
	}

	if (cfg::esp::skeleton)
		RenderPlayerBones(player, mate);

	if (cfg::esp::head_tracker)
		RenderPlayerTracker(player, bounds, mate);

	RenderPlayerBars(player, bounds);
	RenderPlayerFlags(player, bounds, mate);
}

void Esp::RenderPlayerBones(Player player, bool mate) {
	auto color = player.visible
		? cfg::esp::colors::skeleton_visible
		: (mate ? cfg::esp::colors::skeleton_team : cfg::esp::colors::skeleton_enemy);

	auto bone_count = player.bone_list.size();
	for (const auto& bone : connections) {
		int first = bone[0], second = bone[1];

		if (bone_count <= first || bone_count <= second)
			continue;

		const auto& bone1 = player.bone_list[first];
		const auto& bone2 = player.bone_list[second];

		Vec2_t scb1;
		if (!matrix.wts(bone1.pos, io.DisplaySize, scb1))
			continue;

		Vec2_t scb2;
		if (!matrix.wts(bone2.pos, io.DisplaySize, scb2))
			continue;

		d->AddLine(
			scb1,
			scb2,
			ImColor(color),
			1.5f
		);
	}
}

void Esp::RenderPlayerTracker(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate) {
	if (player.bone_list.empty())
		return;

	auto head_bone = player.bone_list[bone_index::head];

	Vec2_t head;
	if (!matrix.wts(head_bone.pos, io.DisplaySize, head))
		return;

	auto width = bounds.second.x - bounds.first.x;
	auto color = mate ? cfg::esp::colors::tracker_team : cfg::esp::colors::tracker_enemy;

	d->AddCircle(
		head,
		width / 6,
		ImColor(color),
		15
	);
}

void Esp::RenderPlayerBars(Player player, std::pair<Vec2_t, Vec2_t> bounds) {
	if (cfg::esp::health) {
		auto x_start = bounds.first.x - 4; // -4 is padding
		auto x_end = x_start - 2; // -2 is the inner space of the rect

		auto y_start = bounds.first.y;
		auto y_end = bounds.second.y;

		float height = y_end - y_start;
		float filled_height = height * (player.health / 100.0f);

		// Health gradient bar (green→yellow→red)
		d->AddRectFilled(
			ImVec2(x_start, y_end - filled_height),
			ImVec2(x_end, y_end),
			HealthColor(player.health)
		);

		d->AddRect(
			ImVec2(x_start, y_start),
			ImVec2(x_end, y_end),
			IM_COL32(0, 0, 0, 50)
		);

		if (cfg::esp::health_number && player.health < 100) {
			auto txt = std::to_string(player.health);
			auto sz = ImGui::CalcTextSize(txt.c_str());

			d->AddText(
				Vec2_t(
					(x_start + x_end) * 0.5f - sz.x * 0.5f,
					y_end - filled_height - sz.y * 0.5f
				),
				IM_COL32(255, 255, 255, 255),
				txt.c_str()
			);
		}
	}

	if (cfg::esp::armor) {
		auto y_start = bounds.second.y + 4; // 4 is padding
		auto y_end = y_start + 2; // 2 is the inner space of the rect

		auto x_start = bounds.first.x;
		auto x_end = bounds.second.x;

		float width = x_end - x_start;
		float filled_width = width * (player.armor / 100.0f);

		d->AddRectFilled(
			ImVec2(x_start, y_start),
			ImVec2(x_start + filled_width, y_end),
			IM_COL32(150, 150, 255, 255)
		);

		d->AddRect(
			ImVec2(x_start, y_start),
			ImVec2(x_end, y_end),
			IM_COL32(0, 0, 0, 50)
		);
	}
}

void Esp::RenderPlayerFlags(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate) {
	if (cfg::esp::flags::name) {
		auto sanitized_name = std::format("{}{}", player.name, (player.bot ? " (Bot)" : ""));
		auto name_size = ImGui::CalcTextSize(sanitized_name.data());

		d->AddText(
			Vec2_t(
				(bounds.first.x + bounds.second.x) / 2 - name_size.x / 2,
				bounds.first.y - 20
			),
			IM_COL32(255, 255, 255, 255),
			sanitized_name.data()
		);
	}

	if (cfg::esp::flags::ammo && player.ammo != -1) {
		auto txt = std::to_string(player.ammo);
		auto ammo_size = ImGui::CalcTextSize(txt.c_str());

		d->AddText(
			Vec2_t(
				(bounds.first.x + bounds.second.x) / 2 - ammo_size.x / 2,
				bounds.second.y + 20
			),
			IM_COL32(255, 255, 255, 255),
			txt.data()
		);
	}

	int offset = 0;
	static int offset_mult = 15;

	if (cfg::esp::flags::money && player.money) {
		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			IM_COL32(255, 255, 255, 255),
			std::format("{}$", player.money).c_str()
		);

		offset -= offset_mult;
	}

	if (cfg::esp::flags::ping && player.ping) {
		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			IM_COL32(255, 255, 255, 255),
			std::format("{}ms", player.ping).c_str()
		);

		offset -= offset_mult;
	}

	// Distance flag
	if (cfg::esp::flags::distance) {
		auto snapshot = Cache::CopySnapshot();
		float dist = player.pos.dist_to(snapshot.local.pos);
		auto dist_str = std::format("{:.0f}m", dist * 0.01905f); // CS2 units to meters (1 unit rough 0.01905m)
		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			IM_COL32(255, 255, 255, 255),
			dist_str.c_str()
		);
		offset -= offset_mult;
	}

	ImGui::PushFont(this->font_merged_icons);

	if (cfg::esp::flags::flashed && player.flashed || cfg::dev::force_show_flags) {
		auto color = mate ? cfg::esp::colors::flags::flashed_team : cfg::esp::colors::flags::flashed_enemy;

		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			ImColor(color),
			Icons::BLIND
		);

		offset -= offset_mult;
	}

	if (cfg::esp::flags::reloading && player.is_reloading || cfg::dev::force_show_flags) {
		auto color = mate ? cfg::esp::colors::flags::reloading_team : cfg::esp::colors::flags::reloading_enemy;

		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			ImColor(color),
			Icons::RELOAD
		);

		offset -= offset_mult;
	}

	if (cfg::esp::flags::defusing && player.defusing || cfg::dev::force_show_flags) {
		auto color = mate ? cfg::esp::colors::flags::defusing_team : cfg::esp::colors::flags::defusing_enemy;

		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			ImColor(color),
			WeaponIcons::CUTTERS
		);

		offset -= offset_mult;
	}

	if (cfg::esp::flags::scoped && player.scoped || cfg::dev::force_show_flags) {
		auto color = mate ? cfg::esp::colors::flags::scoped_team : cfg::esp::colors::flags::scoped_enemy;

		d->AddText(
			bounds.first - Vec2_t((bounds.first.x - bounds.second.x) - 10, offset),
			ImColor(color),
			WeaponIcons::SCOPE
		);

		offset -= offset_mult;
	}

	if (cfg::esp::flags::weapon) {
		auto weapon_size = ImGui::CalcTextSize(player.weapon.icon);

		d->AddText(
			Vec2_t(
				(bounds.first.x + bounds.second.x) / 2 - weapon_size.x / 2,
				bounds.second.y + 6
			),
			IM_COL32(255, 255, 255, 255),
			player.weapon.icon
		);
	}

	ImGui::PopFont();
}

void Esp::RenderPlayerVisionRay(Player local, Player player, bool mate) {
	if (!cfg::esp::vision_ray)
		return;

	// Draw a line from the player eye
	if (player.bone_list.empty())
		return;

	// Get head bone 
	Vec3_t eyePos = player.bone_list[bone_index::head].pos;

	Vec3_t dir = player.vel;
	if (dir.length_2d() < 1.f) {
		// Player is standing still, skip the ray
		return;
	}

	// Normalize the 2D direction and project forward
	float len = dir.length_2d();
	Vec3_t forward = dir / len;
	Vec3_t rayEnd = eyePos + forward * 200.f; // 200 units forward

	Vec2_t screenEye, screenEnd;
	if (!matrix.wts(eyePos, io.DisplaySize, screenEye, false))
		return;
	if (!matrix.wts(rayEnd, io.DisplaySize, screenEnd, false))
		return;

	auto color = mate ? cfg::esp::colors::skeleton_team : cfg::esp::colors::skeleton_enemy;

	d->AddLine(
		screenEye,
		screenEnd,
		ImColor(color),
		1.0f
	);
}

void Esp::RenderDroppedWeapons(const std::vector<WorldEntity>& worldEntities, Player local) {
	if (!cfg::esp::dropped_weapons)
		return;

	if (!local.alive)
		return;

	ImGui::PushFont(this->font_merged_icons);

	for (auto& we : worldEntities) {
		if (we.type != WorldEntity::Type::DroppedWeapon)
			continue;

		Vec2_t screenPos;
		if (!matrix.wts(we.pos, io.DisplaySize, screenPos))
			continue;

		auto color = cfg::esp::colors::dropped_weapon;

		// Draw weapon icon
		auto icon_size = ImGui::CalcTextSize(we.icon);
		d->AddText(
			Vec2_t(screenPos.x - icon_size.x * 0.5f, screenPos.y - icon_size.y * 0.5f),
			ImColor(color),
			we.icon
		);

		// Draw name below icon
		auto name_size = ImGui::CalcTextSize(we.name);
		d->AddText(
			Vec2_t(screenPos.x - name_size.x * 0.5f, screenPos.y + icon_size.y * 0.5f + 2),
			ImColor(color),
			we.name
		);

		// Draw distance
		float dist = we.pos.dist_to(local.pos);
		auto dist_str = std::format("{:.0f}m", dist * 0.01905f);
		auto dist_size = ImGui::CalcTextSize(dist_str.c_str());
		d->AddText(
			Vec2_t(screenPos.x - dist_size.x * 0.5f, screenPos.y + icon_size.y * 0.5f + name_size.y + 4),
			IM_COL32(200, 200, 200, 200),
			dist_str.c_str()
		);
	}

	ImGui::PopFont();
}

void Esp::RenderGrenades(const std::vector<WorldEntity>& worldEntities, Player local) {
	if (!cfg::esp::grenade_esp)
		return;

	if (!local.alive)
		return;

	ImGui::PushFont(this->font_merged_icons);

	for (auto& we : worldEntities) {
		if (we.type != WorldEntity::Type::GrenadeProjectile)
			continue;

		Vec2_t screenPos;
		if (!matrix.wts(we.pos, io.DisplaySize, screenPos))
			continue;

		auto color = cfg::esp::colors::grenade_color;

		// Draw grenade icon
		auto icon_size = ImGui::CalcTextSize(we.icon);
		d->AddText(
			Vec2_t(screenPos.x - icon_size.x * 0.5f, screenPos.y - icon_size.y * 0.5f),
			ImColor(color),
			we.icon
		);

		// Draw name
		auto name_size = ImGui::CalcTextSize(we.name);
		d->AddText(
			Vec2_t(screenPos.x - name_size.x * 0.5f, screenPos.y + icon_size.y * 0.5f + 2),
			ImColor(color),
			we.name
		);

		// Draw distance
		float dist = we.pos.dist_to(local.pos);
		auto dist_str = std::format("{:.0f}m", dist * 0.01905f);
		auto dist_size = ImGui::CalcTextSize(dist_str.c_str());
		d->AddText(
			Vec2_t(screenPos.x - dist_size.x * 0.5f, screenPos.y + icon_size.y * 0.5f + name_size.y + 4),
			IM_COL32(200, 200, 200, 200),
			dist_str.c_str()
		);

		// Draw timer circle for live grenades
		if (we.timer > 0.f) {
			d->AddText(
				Vec2_t(screenPos.x - 5, screenPos.y - icon_size.y * 0.5f - 14),
				IM_COL32(255, 255, 255, 255),
				std::format("{:.0f}s", we.timer).c_str()
			);
		}
	}

	ImGui::PopFont();
}

void Esp::RenderCrosshair(Player local)
{
	if (!cfg::world::crosshair::enabled)
		return;

	if (local.scoped)
		return;

	auto weapon = local.weapon;

	if (weapon.item_index == -1)
		return;

	static std::vector<WeaponIds> valid_weapons = { weapon_ssg08, weapon_awp, weapon_g3sg1, weapon_scar20 };

	if (std::find(valid_weapons.begin(), valid_weapons.end(), weapon.item_index) == valid_weapons.end())
		return;

	ImVec2 center(
		floorf(io.DisplaySize.x * 0.5f),
		floorf(io.DisplaySize.y * 0.5f));

	constexpr float size = 6.f;
	constexpr float thickness = 1.0f;

	d->AddLine(
		ImVec2(center.x - size, center.y),
		ImVec2(center.x + size + 1, center.y),
		IM_COL32(255, 255, 255, 255),

		thickness);
	d->AddLine(
		ImVec2(center.x, center.y - size),
		ImVec2(center.x, center.y + size + 1),
		IM_COL32(255, 255, 255, 255),
		thickness);
}

void Esp::RenderPlayerTracers(Player source, Player player, bool mate) {
	if (!cfg::esp::tracers)
		return;

	Vec2_t screenPos;
	bool projected = matrix.wts(player.pos, io.DisplaySize, screenPos, false);

	if (!projected)
	{
		Vec3_t camPos = source.pos;
		Vec3_t dir = player.pos - camPos;

		// projection for off screen players
		Vec3_t viewDir;
		viewDir.x = matrix[0][0] * dir.x + matrix[0][1] * dir.y + matrix[0][2] * dir.z;
		viewDir.y = matrix[1][0] * dir.x + matrix[1][1] * dir.y + matrix[1][2] * dir.z;
		viewDir.z = matrix[2][0] * dir.x + matrix[2][1] * dir.y + matrix[2][2] * dir.z;

		if (viewDir.z > 0.0f)
		{
			viewDir.x = -viewDir.x;
			viewDir.y = -viewDir.y;
		}

		// normalize
		float len = sqrt(viewDir.x * viewDir.x + viewDir.y * viewDir.y);
		if (len > 0.001f)
		{
			viewDir.x /= len;
			viewDir.y /= len;
		}

		screenPos.x = io.DisplaySize.x * 0.5f + viewDir.x * io.DisplaySize.x * 0.5f;
		screenPos.y = io.DisplaySize.y * 0.5f - viewDir.y * io.DisplaySize.y * 0.5f;

		float margin = 10.f;
		screenPos.x = std::clamp(screenPos.x, margin, io.DisplaySize.x - margin);
		screenPos.y = std::clamp(screenPos.y, margin, io.DisplaySize.y - margin);
	}

	auto color = mate ? cfg::esp::colors::tracer_team : cfg::esp::colors::tracer_enemy;

	d->AddLine(
		Vec2_t(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
		screenPos,
		ImColor(color),
		1.0f
	);
}

void Esp::RenderBomb(Player local, Bomb bomb) {
	if (!cfg::world::bomb::location && !cfg::world::bomb::timer && !cfg::world::bomb::damage_calc)
		return;

	if (!bomb.is_planted)
		return;

	if (!bomb.pos.length())
		return;

	if (!local.alive)
		return;

	auto marker = bomb.pos + Vec3_t(0, 0, 20);

	Vec2_t pos;
	if (!matrix.wts(bomb.pos, io.DisplaySize, pos))
		return;

	auto distance = bomb.pos.dist_to(local.pos);

	float width = 20.f;
	float height = 20.f;
	float rounding = 10.f;

	static int margin = 10;
	static int padding = 10;

	auto duration_str = std::format("{}s", bomb.time_left);
	auto bombsite_str = std::string(bomb.site == BombSite::A ? "A" : "B");

	std::string bomb_string = "";

	if (cfg::world::bomb::location) {
		bomb_string += bombsite_str;
	}

	if (cfg::world::bomb::timer) {
		if (cfg::world::bomb::location)
			bomb_string += " - ";
		else
			bomb_string += " ";

		bomb_string += duration_str;
	}

	// Bomb damage calculator overlay
	if (cfg::world::bomb::damage_calc) {
		float dist3d = bomb.pos.dist_to_3d(local.pos);
		// C4 damage formula (approximate): damage = 500 * (1 - dist/1750), armor reduces it
		float rawDamage = 500.f * std::max(0.f, 1.f - (dist3d / 1750.f));
		float damage = rawDamage;
		if (local.armor > 0 && rawDamage > 0) {
			// Armor absorbs 50% of damage, reducing armor by damage/2
			float armorDamage = rawDamage * 0.5f;
			if (local.armor * 2 < rawDamage) {
				// Armor exhausted, remaining damage goes to health
				damage = rawDamage - (float)local.armor;
			} else {
				damage = armorDamage;
			}
		}
		bool willDie = damage >= local.health;
		float survivalDist = 1750.f * (1.f - local.health / 500.f); // distance where damage = health

		bomb_string += std::format(" | {}{}", willDie ? "FATAL" : "Safe", willDie ? "" : std::format(" -{}hp", (int)damage));
	}

	auto text_size = ImGui::CalcTextSize(bomb_string.data());
	width = text_size.x; height = text_size.y;

	d->AddRectFilled(
		ImVec2(pos.x + margin - padding, pos.y - height - 20 - margin - padding),
		ImVec2(pos.x + width + margin + padding, pos.y - 20 - margin + padding),
		IM_COL32(0, 0, 0, 200),
		rounding
	);

	d->AddRect(
		ImVec2(pos.x + margin - padding, pos.y - height - 20 - margin - padding),
		ImVec2(pos.x + width + margin + padding, pos.y - 20 - margin + padding),
		IM_COL32(100, 100, 100, 200),
		rounding
	);

	d->AddText(
		ImVec2(pos.x + margin, pos.y - height - 20 - margin),
		IM_COL32(255, 255, 255, 255),
		bomb_string.data()
	);
}

void Esp::RenderAimbotFOV() {
	if (!cfg::aimbot::draw_fov || !cfg::aimbot::enabled)
		return;

	constexpr float DEG_TO_RAD = 3.14159265f / 180.f;

	auto snapshot = Cache::CopySnapshot();
	float gameFov = 90.f;
	if (cfg::aimbot::fov_zoom_scale && snapshot.local.scoped && snapshot.local.zoom_level > 0) {
		static const float zoom_fov[] = { 90.f, 40.f, 15.f };
		int zl = snapshot.local.zoom_level;
		if (zl < 0) zl = 0;
		if (zl > 2) zl = 2;
		gameFov = zoom_fov[zl];
	}

	ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
	float halfWindowWidth = io.DisplaySize.x * 0.5f;

	float aimFovTan = tanf(cfg::aimbot::fov * DEG_TO_RAD * 0.5f);
	float staticFovTan = tanf(gameFov * DEG_TO_RAD * 0.5f);
	float radius = (aimFovTan / staticFovTan) * halfWindowWidth;

	d->AddCircle(center, radius, ImColor(cfg::aimbot::fov_color), 64, 1.5f);
}

void Esp::RenderTriggerbotFOV() {
	if (!cfg::misc::triggerbot::enabled || !cfg::misc::triggerbot::draw)
		return;

	ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
	d->AddCircle(center, cfg::misc::triggerbot::fov, ImColor(cfg::misc::triggerbot::draw_color), 32, 1.0f);
}

void Esp::RenderHitMarkers() {
	if (!cfg::misc::hit_marker::enabled)
		return;

	float now = std::chrono::duration<float>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
	float duration_s = cfg::misc::hit_marker::duration_ms / 1000.f;

	std::lock_guard<std::mutex> lock(Misc::hit_marker_mutex);

	Misc::hit_markers.erase(
		std::remove_if(Misc::hit_markers.begin(), Misc::hit_markers.end(),
			[&](const Misc::HitMarker& hm) { return (now - hm.created_at) >= duration_s; }),
		Misc::hit_markers.end()
	);

	for (auto& hm : Misc::hit_markers) {
		float t = (now - hm.created_at) / duration_s;
		float alpha = 1.f - t;
		auto& col = cfg::misc::hit_marker::color;
		ImU32 c = IM_COL32(
			(int)(col.r * 255),
			(int)(col.g * 255),
			(int)(col.b * 255),
			(int)(col.a * alpha * 255)
		);
		float x = hm.screen_pos.x;
		float y = hm.screen_pos.y;
		constexpr float sz = 8.f;
		constexpr float gap = 3.f;
		d->AddLine(ImVec2(x - sz, y - sz), ImVec2(x - gap, y - gap), c, 1.5f);
		d->AddLine(ImVec2(x + gap, y - gap), ImVec2(x + sz, y - sz), c, 1.5f);
		d->AddLine(ImVec2(x - sz, y + sz), ImVec2(x - gap, y + gap), c, 1.5f);
		d->AddLine(ImVec2(x + gap, y + gap), ImVec2(x + sz, y + sz), c, 1.5f);
	}
}
