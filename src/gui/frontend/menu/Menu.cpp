#include "Menu.hpp"
#include <windows.h>
#include <algorithm>

#include "config/Config.hpp"
#include "core/engine/cache/Cache.hpp"
#include "gui/renderer/Renderer.hpp"
#include "gui/renderer/window/Window.hpp"
#include "assets/fonts/Icons.h"


bool Menu::Init() {
	return GetInstance().InitImpl();
}

void Menu::Render() {
	return GetInstance().RenderImpl();
}

void Menu::RenderStartupHelp() {
	return GetInstance().RenderStartupHelpImpl();
}

ImVec2 Menu::GetPos() {
	return GetInstance().pos;
}

ImVec2 Menu::GetSize() {
	return GetInstance().size;
}

bool Menu::InitImpl() {
	SetupStyles();

	LOGF(INFO, "Successfully initialized menu...");
	return true;
}

void Menu::RenderImpl() {
	if (!isSetup)
		return;

	static auto io = ImGui::GetIO();
	static auto screen = io.DisplaySize;
	static auto color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None;

#ifdef _DEBUG
	static auto title = "github.com/IMXNOOBX/cs2-external-esp (recode) [DEV]";
#else
	static auto title = "cs2-external-esp | recode";
#endif

	ImGui::SetNextWindowSize(ImVec2(600, 370), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(screen.x / 2 - 300, screen.y / 2 - 150), ImGuiCond_FirstUseEver);

	ImGui::GetWindowPos();
	if (ImGui::Begin(title, nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
		this->pos = ImGui::GetWindowPos();
		this->size = ImGui::GetWindowSize();

		static int active_tab = 0;

		if (ImGui::BeginChild("##main_split"))
		{
			auto size = ImGui::GetContentRegionAvail();

			ImGui::BeginChild("##tab_buttons", ImVec2(160, size.y), true);
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.1f, 0.5f));
				for (const auto& tab : tabs)
				{
					bool is_active = (active_tab == tab.id);

					if (is_active)
					{
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
					}

					if (ImGui::Button((tab.icon + " " + tab.label).c_str(), ImVec2(-1, 28)))
						active_tab = tab.id;

					if (is_active) 
						ImGui::PopStyleColor(3);
				}
				ImGui::PopStyleVar(1);

				auto space = ImGui::GetContentRegionAvail().y;
				ImGui::SetCursorPosY(ImGui::GetCursorPos().y + space - 16.f * 3);

				ImGui::Dummy(ImVec2(11, 0)); ImGui::SameLine();
				ImGui::TextLinkOpenURL(Icons::DISCORD, "https://discord.gg/pRew8ZDkyp");
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(11, 0)); ImGui::SameLine();
				ImGui::TextLinkOpenURL(Icons::GITHUB, "https://github.com/IMXNOOBX/cs2-external-esp");
				ImGui::Separator();

				ImGui::Checkbox("Enable", &cfg::enabled);
			}
			ImGui::EndChild();

			ImGui::SameLine();


			ImGui::BeginDisabled(!cfg::enabled);

			ImGui::BeginChild("##tab_content", ImVec2(0, size.y), true);
			{
				if (active_tab == Tab::PLAYER)
				{
					ImGui::Text("Visuals");
					ImGui::Separator();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Box", &cfg::esp::box);
						ImGui::BeginDisabled(!cfg::esp::box);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team box color", cfg::esp::colors::box_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy box color", cfg::esp::colors::box_enemy.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Spotted box color", cfg::esp::colors::box_visible.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Skeleton", &cfg::esp::skeleton);
						ImGui::BeginDisabled(!cfg::esp::skeleton);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team skeleton color", cfg::esp::colors::skeleton_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy skeleton color", cfg::esp::colors::skeleton_enemy.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Spotted skeleton color", cfg::esp::colors::skeleton_visible.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Head Tracker", &cfg::esp::head_tracker);
						ImGui::BeginDisabled(!cfg::esp::head_tracker);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team head tracker color", cfg::esp::colors::tracker_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy head tracker color", cfg::esp::colors::tracker_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Tracers", &cfg::esp::tracers);
						ImGui::BeginDisabled(!cfg::esp::tracers);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team tracer color", cfg::esp::colors::tracer_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy tracer color", cfg::esp::colors::tracer_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Vision Ray", &cfg::esp::vision_ray);
						ImGui::SetItemTooltip("Draws a line from your position to each player to visualize line of sight");
					}
					ImGui::EndGroup();

					ImGui::SameLine();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Health", &cfg::esp::health);
						if (cfg::esp::health)
							ImGui::Checkbox("Health Number", &cfg::esp::health_number);
						ImGui::Checkbox("Armor", &cfg::esp::armor);

						ImGui::Checkbox("Spotted", &cfg::esp::spotted);
						ImGui::SetItemTooltip("Esp will only be visible if the player has been spotted by you");

						ImGui::Checkbox("Show Team", &cfg::esp::team);

						ImGui::Checkbox("3D Box", &cfg::esp::box_3d);
						ImGui::Checkbox("Visible Only", &cfg::esp::visible_only);
						ImGui::SetItemTooltip("Only show ESP for players currently visible to you");
					}
					ImGui::EndGroup();

					ImGui::Spacing();
					ImGui::Text("World ESP");
					ImGui::Separator();

					ImGui::Checkbox("Dropped Weapons", &cfg::esp::dropped_weapons);
					ImGui::BeginDisabled(!cfg::esp::dropped_weapons);
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("Dropped Weapon Color", cfg::esp::colors::dropped_weapon.data(), color_flags);
					}
					ImGui::EndDisabled();

					ImGui::Checkbox("Grenade ESP", &cfg::esp::grenade_esp);
					ImGui::BeginDisabled(!cfg::esp::grenade_esp);
					{
						ImGui::SameLine();
						ImGui::ColorEdit4("Grenade Color", cfg::esp::colors::grenade_color.data(), color_flags);
					}
					ImGui::EndDisabled();

					ImGui::Spacing();

					ImGui::Text("Flags");
					ImGui::Separator();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Flashed", &cfg::esp::flags::flashed);
						ImGui::BeginDisabled(!cfg::esp::flags::flashed);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team flashed color", cfg::esp::colors::flags::flashed_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy flashed color", cfg::esp::colors::flags::flashed_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Reloading", &cfg::esp::flags::reloading);
						ImGui::BeginDisabled(!cfg::esp::flags::reloading);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team reloading color", cfg::esp::colors::flags::reloading_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy reloading color", cfg::esp::colors::flags::reloading_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Defusing", &cfg::esp::flags::defusing);
						ImGui::BeginDisabled(!cfg::esp::flags::defusing);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team defusing color", cfg::esp::colors::flags::defusing_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy defusing color", cfg::esp::colors::flags::defusing_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();

						ImGui::Checkbox("Scoped", &cfg::esp::flags::scoped);
						ImGui::BeginDisabled(!cfg::esp::flags::scoped);
						{
							ImGui::SameLine();
							ImGui::ColorEdit4("Team scoped color", cfg::esp::colors::flags::scoped_team.data(), color_flags);
							ImGui::SameLine();
							ImGui::ColorEdit4("Enemy scoped color", cfg::esp::colors::flags::scoped_enemy.data(), color_flags);
						}
						ImGui::EndDisabled();
					}
					ImGui::EndGroup();

					ImGui::SameLine();

					ImGui::BeginGroup();
					{
						ImGui::Checkbox("Name", &cfg::esp::flags::name);
						ImGui::Checkbox("Money", &cfg::esp::flags::money);
						ImGui::Checkbox("Weapon", &cfg::esp::flags::weapon);
						ImGui::Checkbox("Ammo", &cfg::esp::flags::ammo);
						ImGui::Checkbox("Ping", &cfg::esp::flags::ping);
						ImGui::Checkbox("Distance", &cfg::esp::flags::distance);
					}
					ImGui::EndGroup();
				}
				else if (active_tab == Tab::AIMBOT)
				{
					ImGui::Text("Aimbot");
					ImGui::Separator();

					ImGui::Checkbox("Enable Aimbot", &cfg::aimbot::enabled);
					ImGui::BeginDisabled(!cfg::aimbot::enabled);
					{
						// ImGui::Checkbox("Angle Write", &cfg::aimbot::angle_write);
						// ImGui::SetItemTooltip("Writes view angles directly instead of mouse movement");
						ImGui::Checkbox("Visible Targets Only", &cfg::aimbot::visible_only);
						ImGui::SetItemTooltip("Only aim at targets that are currently visible to you");
						ImGui::Checkbox("Flash Check", &cfg::aimbot::flash_check);
						ImGui::SetItemTooltip("Pause aimbot while you are flashed");
						ImGui::Checkbox("Scope Check", &cfg::aimbot::scope_check);
						ImGui::SetItemTooltip("Only aim with scoped weapons when scoped in");
						ImGui::Checkbox("Stop Check", &cfg::aimbot::stop_check);
						ImGui::SetItemTooltip("Only aim when your movement is nearly stopped");
						ImGui::SliderInt("Start Bullet", &cfg::aimbot::start_bullet, 0, 5, "%d");
						ImGui::SetItemTooltip("Delay aimbot until after a few shots have been fired");

						ImGui::SliderFloat("FOV", &cfg::aimbot::fov, 1.0f, 180.0f, "%.1f");
						ImGui::Checkbox("Draw FOV", &cfg::aimbot::draw_fov);
						ImGui::ColorEdit4("FOV Color", cfg::aimbot::fov_color.data(), color_flags);
						ImGui::SliderFloat("Smoothing", &cfg::aimbot::smooth, 1.0f, 20.0f, "%.1f");

						const char* bone_names[] = { "Head", "Neck", "Chest", "Spine" };
						int bone_values[] = { 7, 6, 23, 4 };
						int bone_current = 0;
						for (int i = 0; i < 4; i++) { if (cfg::aimbot::bone == bone_values[i]) { bone_current = i; break; } }
						if (ImGui::Combo("Target Bone", &bone_current, bone_names, 4))
							cfg::aimbot::bone = bone_values[bone_current];

						static bool waiting_for_key = false;
						if (waiting_for_key) {
							ImGui::Button("Press any key...", ImVec2(-1, 0));
							for (int i = 1; i < 256; i++) {
								if (i == VK_LBUTTON || i == VK_RBUTTON || i == VK_MBUTTON) continue;
								if (GetAsyncKeyState(i) & 0x8000) {
									cfg::aimbot::hotkey = i;
									waiting_for_key = false;
									break;
								}
							}
						} else {
							char key_label[64];
							UINT scanCode = MapVirtualKey(cfg::aimbot::hotkey, MAPVK_VK_TO_VSC);
							LONG lParam = (scanCode << 16);
							if (GetKeyNameTextA(lParam, key_label, 64) == 0)
								sprintf_s(key_label, sizeof(key_label), "0x%X", cfg::aimbot::hotkey);
							char btn_label[80];
							sprintf_s(btn_label, sizeof(btn_label), "Hotkey: %s", key_label);
							if (ImGui::Button(btn_label, ImVec2(-1, 0)))
								waiting_for_key = true;
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("Click to rebind");
						}
						ImGui::Checkbox("Always On##aim", &cfg::aimbot::always_on);
						ImGui::SetItemTooltip("Aimbot is active by default; hold hotkey to disable");
					}
					ImGui::EndDisabled();

					ImGui::Spacing();
					ImGui::Text("Humanize");
					ImGui::Separator();

					ImGui::Checkbox("Enable Humanize", &cfg::aimbot::humanize);
					ImGui::BeginDisabled(!cfg::aimbot::humanize);
					{
						ImGui::SliderFloat("Smooth Variance", &cfg::aimbot::smooth_variance, 0.0f, 1.0f, "%.2f");
						ImGui::SliderFloat("Jitter", &cfg::aimbot::jitter, 0.0f, 3.0f, "%.2f");
					}
					ImGui::EndDisabled();

					ImGui::Spacing();
					ImGui::Text("Velocity Compensation");
					ImGui::Separator();
					ImGui::Checkbox("Enable Velocity Comp", &cfg::aimbot::velocity_comp);
					ImGui::BeginDisabled(!cfg::aimbot::velocity_comp);
					{
						ImGui::SliderFloat("Lead Time", &cfg::aimbot::velocity_comp_scale, 0.0f, 0.3f, "%.3f");
					}
					ImGui::EndDisabled();

					ImGui::Spacing();
					ImGui::Text("Recoil Control");
					ImGui::Separator();

					ImGui::Checkbox("Enable RCS", &cfg::aimbot::rcs);
					ImGui::BeginDisabled(!cfg::aimbot::rcs);
					{
						ImGui::SliderFloat("Sensitivity", &cfg::aimbot::sensitivity, 0.1f, 10.0f, "%.2f");
					}
					ImGui::EndDisabled();

					ImGui::Spacing();
					ImGui::Text("Soft Aim");
					ImGui::Separator();

					ImGui::Checkbox("Enable Soft Aim", &cfg::aimbot::soft_aim);
					ImGui::SetItemTooltip("Caps move per tick so the aim only nudges when already close to target");
					ImGui::BeginDisabled(!cfg::aimbot::soft_aim);
					{
						ImGui::SliderFloat("Max Move (px)", &cfg::aimbot::soft_aim_max_move, 0.5f, 20.0f, "%.1f");
					}
					ImGui::EndDisabled();

					ImGui::Spacing();
					ImGui::Text("Multi-Bone");
					ImGui::Separator();

					ImGui::Checkbox("Multi-Bone Aimbot", &cfg::aimbot::multi_bone);
					ImGui::SetItemTooltip("Iterate head/neck/chest and lock onto the bone closest to crosshair");
					ImGui::BeginDisabled(!cfg::aimbot::multi_bone);
					{
						static const char* bone_labels[] = { "Head", "Neck", "Chest" };
						static const int bone_vals[] = { 7, 6, 23 };
						for (int i = 0; i < 3; i++) {
							bool active = false;
							for (int v : cfg::aimbot::bone_priority)
								if (v == bone_vals[i]) { active = true; break; }
							if (ImGui::Checkbox(bone_labels[i], &active)) {
								if (active) {
									cfg::aimbot::bone_priority.push_back(bone_vals[i]);
								} else {
									cfg::aimbot::bone_priority.erase(
										std::remove(cfg::aimbot::bone_priority.begin(), cfg::aimbot::bone_priority.end(), bone_vals[i]),
										cfg::aimbot::bone_priority.end()
									);
								}
							}
							if (i < 2) ImGui::SameLine();
						}
					}
					ImGui::EndDisabled();

					ImGui::Spacing();
					ImGui::Text("Zoom");
					ImGui::Separator();
					ImGui::Checkbox("FOV Scales With Zoom", &cfg::aimbot::fov_zoom_scale);
					ImGui::SetItemTooltip("Shrinks the aimbot FOV circle when scoped to match actual visible FOV");
				}
				else if (active_tab == Tab::WORLD)
				{
					ImGui::Text("Bomb");
					
					ImGui::Separator();

					ImGui::Checkbox("Bomb Location", &cfg::world::bomb::location);
					ImGui::Checkbox("Bomb Timer", &cfg::world::bomb::timer);
					ImGui::Checkbox("Bomb Damage Calc", &cfg::world::bomb::damage_calc);
					ImGui::SetItemTooltip("Shows estimated damage you will take from the bomb explosion");

					ImGui::Spacing();

					ImGui::Text("Spectator list");
					ImGui::Separator();

					ImGui::Checkbox("Enable", &cfg::world::spectators::enabled);
					if (cfg::world::spectators::enabled) {
						ImGui::Checkbox("Detailed", &cfg::world::spectators::detailed);
						ImGui::Checkbox("Only Self", &cfg::world::spectators::self_only);
						ImGui::SetItemTooltip("Only display users spectating you");
					}

					ImGui::Spacing();

					ImGui::Text("Misc");
					ImGui::Separator();

					ImGui::Checkbox("Crosshair", &cfg::world::crosshair::enabled);
					ImGui::Checkbox("Velocity Graph", &cfg::world::velocity::enabled);
				#ifdef _DEBUG // Part of the velocity graph for developers
					if (cfg::world::velocity::enabled) {
						ImGui::SliderInt("Sample rate", &cfg::world::velocity::sample_rate, 1, 100);
						ImGui::SliderFloat("Sample length", &cfg::world::velocity::sample_length, 1, 20, "%.1f");
					}
				#endif
					ImGui::Spacing();

					ImGui::Text("Radar");
					ImGui::Separator();

					ImGui::Checkbox("Radar", &cfg::world::radar::enabled);
					ImGui::BeginDisabled(!cfg::world::radar::enabled);
					{
						ImGui::SameLine();
						ImGui::SliderFloat("Range", &cfg::world::radar::range, 100.f, 8000.f, "%.0f u");
						ImGui::Checkbox("Disable Rotation", &cfg::world::radar::no_rotate);
					}
					ImGui::EndDisabled();
				}
				else if (active_tab == Tab::MOVEMENT)
				{
					ImGui::Text("Movement");
					ImGui::Separator();

					ImGui::Checkbox("Bunny Hop", &cfg::misc::bhop);
					ImGui::SetItemTooltip("Auto-jumps on landing while space is held");

					ImGui::Checkbox("Strafe Helper", &cfg::misc::strafe::helper);
					ImGui::SetItemTooltip("Corrects A/D to match mouse turn direction mid-air");

					ImGui::Checkbox("Anti-AFK", &cfg::misc::anti_afk);
					ImGui::SetItemTooltip("Wiggles A/D periodically to prevent AFK kick");
					ImGui::BeginDisabled(!cfg::misc::anti_afk);
					{
						ImGui::SliderInt("AFK Interval (s)", &cfg::misc::anti_afk_interval_s, 10, 300, "%ds");
					}
					ImGui::EndDisabled();
				}
				else if (active_tab == Tab::COMBAT)
				{
					ImGui::Text("Visuals");
					ImGui::Separator();

					ImGui::Checkbox("Anti Flash", &cfg::misc::anti_flash);
					ImGui::SetItemTooltip("Zeroes flash alpha every tick - you never go blind");

					ImGui::Checkbox("Anti Smoke (WIP)", &cfg::misc::anti_smoke);
					ImGui::SetItemTooltip("Reduces smoke opacity for visibility through smokes");

					ImGui::Text("Weapons");
					ImGui::Separator();

					ImGui::Checkbox("Trigger Bot", &cfg::misc::triggerbot::enabled);
					ImGui::BeginDisabled(!cfg::misc::triggerbot::enabled);
					{
						ImGui::SliderFloat("Trigger FOV", &cfg::misc::triggerbot::fov, 0.5f, 20.0f, "%.1f px");
						ImGui::SliderInt("Trigger Delay", &cfg::misc::triggerbot::delay_ms, 0, 300, "%d ms");
						ImGui::Checkbox("Shot Delay", &cfg::misc::triggerbot::shot_delay);
						ImGui::SetItemTooltip("On: wait between shots. Off: hold the mouse down while a target stays in range.");
						ImGui::BeginDisabled(!cfg::misc::triggerbot::shot_delay);
						{
							ImGui::SliderInt("Shot Delay Time", &cfg::misc::triggerbot::shot_delay_ms, 0, 1000, "%d ms");
						}
						ImGui::EndDisabled();
						ImGui::Checkbox("Visible Targets Only", &cfg::misc::triggerbot::visible_only);
						ImGui::SetItemTooltip("Only trigger on targets that are currently visible to you");

						ImGui::Checkbox("Draw FOV##tbot", &cfg::misc::triggerbot::draw);
						ImGui::BeginDisabled(!cfg::misc::triggerbot::draw);
						{
							ImGui::ColorEdit4("FOV Color##tbot", cfg::misc::triggerbot::draw_color.data(), color_flags);
						}
						ImGui::EndDisabled();

						static bool waiting_for_trigger_key = false;
						if (waiting_for_trigger_key) {
							ImGui::Button("Press any key...", ImVec2(-1, 0));
							for (int i = 1; i < 256; i++) {
								if (GetAsyncKeyState(i) & 0x8000) {
									cfg::misc::triggerbot::hotkey = i;
									waiting_for_trigger_key = false;
									break;
								}
							}
						} else {
							char key_label[64];
							UINT sc = MapVirtualKey(cfg::misc::triggerbot::hotkey, MAPVK_VK_TO_VSC);
							if (GetKeyNameTextA((LONG)(sc << 16), key_label, 64) == 0)
								sprintf_s(key_label, sizeof(key_label), "0x%X", cfg::misc::triggerbot::hotkey);
							char btn_label[80];
							sprintf_s(btn_label, sizeof(btn_label), "Hotkey: %s", key_label);
							if (ImGui::Button(btn_label, ImVec2(-1, 0)))
								waiting_for_trigger_key = true;
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("Hold this key to enable triggerbot");
						}
						ImGui::Checkbox("Always On##trig", &cfg::misc::triggerbot::always_on);
						ImGui::SetItemTooltip("Triggerbot is active by default; hold hotkey to disable");
					}
					ImGui::EndDisabled();

					ImGui::Checkbox("Auto Zeus", &cfg::misc::auto_zeus::enabled);
					ImGui::SetItemTooltip("Switches to Zeus and fires when an enemy is in range");
					ImGui::BeginDisabled(!cfg::misc::auto_zeus::enabled);
					{
						ImGui::SliderFloat("Zeus Range", &cfg::misc::auto_zeus::range, 50.f, 300.f, "%.0f u");
						ImGui::Checkbox("Visible Targets Only##zeus", &cfg::misc::auto_zeus::visible_only);
						ImGui::SetItemTooltip("Only use Zeus on targets that are currently visible to you");
					}
					ImGui::EndDisabled();

					ImGui::Checkbox("Auto Knife", &cfg::misc::auto_knife::enabled);
					ImGui::SetItemTooltip("Switches to knife and swings when an enemy is in range");
					ImGui::BeginDisabled(!cfg::misc::auto_knife::enabled);
					{
						ImGui::SliderFloat("Knife Range", &cfg::misc::auto_knife::range, 30.f, 150.f, "%.0f u");
						ImGui::Checkbox("Visible Targets Only##knife", &cfg::misc::auto_knife::visible_only);
						ImGui::SetItemTooltip("Only use knife on targets that are currently visible to you");
					}
					ImGui::EndDisabled();

					ImGui::Text("Feedback");
					ImGui::Separator();

					ImGui::Checkbox("Kill Sound", &cfg::misc::kill_sound::enabled);
					ImGui::SetItemTooltip("Plays kill.wav on kill (place kill.wav next to the exe)");

					ImGui::Checkbox("Hit Marker", &cfg::misc::hit_marker::enabled);
					ImGui::BeginDisabled(!cfg::misc::hit_marker::enabled);
					{
						ImGui::ColorEdit4("Hit Marker Color", cfg::misc::hit_marker::color.data(), color_flags);
						ImGui::SliderFloat("Duration (ms)", &cfg::misc::hit_marker::duration_ms, 100.f, 1500.f, "%.0f ms");
					}
					ImGui::EndDisabled();

					ImGui::Checkbox("Session Stats", &cfg::misc::stats::enabled);
					ImGui::SetItemTooltip("Shows hit/kill/shot counter overlay");
				}
				else if (active_tab == Tab::UTILITY)
				{
					ImGui::Text("Clantag (WIP)");
					ImGui::Separator();

					ImGui::Checkbox("Clantag Changer", &cfg::misc::clantag::enabled);
					ImGui::SetItemTooltip("Writes a custom clan tag to the local player controller");
					ImGui::BeginDisabled(!cfg::misc::clantag::enabled);
					{
						ImGui::InputText("Clantag", cfg::misc::clantag::text, sizeof(cfg::misc::clantag::text));
					}
					ImGui::EndDisabled();

					ImGui::Text("Ping");
					ImGui::Separator();

					ImGui::Checkbox("Fake Ping", &cfg::misc::fake_ping::enabled);
					ImGui::SetItemTooltip("Spoofs your ping value in the scoreboard");
					ImGui::BeginDisabled(!cfg::misc::fake_ping::enabled);
					{
						ImGui::SliderInt("Ping (ms)", &cfg::misc::fake_ping::ping, 0, 999, "%d ms");
					}
					ImGui::EndDisabled();

					ImGui::Text("Ranks");
					ImGui::Separator();

					ImGui::Checkbox("Rank Revealer", &cfg::misc::rank_revealer::enabled);
					ImGui::SetItemTooltip("Forces rank display in the scoreboard");

					ImGui::Checkbox("Auto Queue", &cfg::misc::auto_queue::enabled);
					ImGui::SetItemTooltip("Clicks Play every 3s when not in a match");
					ImGui::BeginDisabled(!cfg::misc::auto_queue::enabled);
					{
						ImGui::Checkbox("Accept Match", &cfg::misc::auto_queue::accept_match);
						ImGui::SetItemTooltip("Also clicks the Accept button when a match is found");
					}
					ImGui::EndDisabled();

					ImGui::Text("Skin Changer (WIP)");
					ImGui::Separator();

					ImGui::Checkbox("Skin Changer", &cfg::misc::skin::enabled);
					ImGui::SetItemTooltip("Replaces active weapon skin client-side only");
					ImGui::BeginDisabled(!cfg::misc::skin::enabled);
					{
						ImGui::InputInt("Target Weapon ID", &cfg::misc::skin::weapon_id);
						ImGui::SetItemTooltip("Item def index of the weapon to reskin (e.g. 9 = AWP)");
						ImGui::InputInt("Replacement Skin ID", &cfg::misc::skin::skin_id);
						ImGui::SetItemTooltip("Item def index to write (e.g. 7 = AK-47 model on AWP slot)");
					}
					ImGui::EndDisabled();
				}
				else if (active_tab == Tab::SETTINGS)
				{
					ImGui::Text("Misc");
					ImGui::Separator();

					if (ImGui::Checkbox("Streamproof", &cfg::settings::streamproof))
					{
						Window::SetAffinity(
							Window::hwnd,
							cfg::settings::streamproof ? WindowAffinity::Invisible : WindowAffinity::Disabled
						);
					}

					ImGui::Checkbox("Watermark", &cfg::settings::watermark);
					ImGui::Checkbox("Keybinds Overlay", &cfg::settings::keybinds_overlay);
					ImGui::SetItemTooltip("Shows a small HUD with active features and their hotkeys");
					ImGui::Checkbox("Save on Exit", &cfg::settings::save_on_exit);
					ImGui::SetItemTooltip("Automatically saves config when closing the overlay");

					if (ImGui::Checkbox("VSync", &cfg::settings::vsync))
						Window::vsync = cfg::settings::vsync;

					ImGui::Checkbox("Free CPU", &cfg::settings::free_cpu);
					ImGui::SetItemTooltip("Let the CPU sleep to Free Resources\nNOTE: might cause performance issues in lower end computers!");

					ImGui::Spacing();
					ImGui::Text("Config Profiles");
					ImGui::Separator();

					{
						static char profile_name_buf[64] = "";
						static std::vector<std::string> profile_list;
						static int profile_selected = -1;
						static bool profiles_dirty = true;

						if (profiles_dirty) {
							profile_list = Config::ListProfiles();
							profile_selected = -1;
							for (int i = 0; i < (int)profile_list.size(); i++) {
								if (profile_list[i] == cfg::settings::current_profile) {
									profile_selected = i;
									break;
								}
							}
							profiles_dirty = false;
						}

						std::vector<const char*> profile_cstrs;
						for (auto& s : profile_list) profile_cstrs.push_back(s.c_str());

						ImGui::SetNextItemWidth(160.f);
						if (ImGui::Combo("##profiles", &profile_selected, profile_cstrs.data(), (int)profile_cstrs.size())) {
							if (profile_selected >= 0 && profile_selected < (int)profile_list.size()) {
								Config::ReadProfile(profile_list[profile_selected]);
								strncpy_s(profile_name_buf, profile_list[profile_selected].c_str(), sizeof(profile_name_buf) - 1);
							}
						}
						ImGui::SameLine();
						if (ImGui::Button("Refresh")) profiles_dirty = true;

						ImGui::SetNextItemWidth(160.f);
						ImGui::InputText("##profile_name", profile_name_buf, sizeof(profile_name_buf));
						ImGui::SameLine();
						if (ImGui::Button("Save As")) {
							if (profile_name_buf[0] != '\0') {
								Config::WriteProfile(profile_name_buf);
								profiles_dirty = true;
							}
						}
					}

					ImGui::Spacing();
					ImGui::Text("Toggle Keybind");
					ImGui::Separator();

					static bool waiting_for_toggle_key = false;
					if (waiting_for_toggle_key) {
						ImGui::Button("Press any key...", ImVec2(-1, 0));
						for (int i = 1; i < 256; i++) {
							if (i == VK_LBUTTON || i == VK_RBUTTON || i == VK_MBUTTON) continue;
							if (GetAsyncKeyState(i) & 0x8000) {
								cfg::settings::toggle_key = i;
								waiting_for_toggle_key = false;
								Config::Write();
								break;
							}
						}
					} else {
						char key_label[64];
						UINT scanCode = MapVirtualKey(cfg::settings::toggle_key, MAPVK_VK_TO_VSC);
						LONG lParam = (scanCode << 16);
						if (GetKeyNameTextA(lParam, key_label, 64) == 0)
							sprintf_s(key_label, sizeof(key_label), "0x%X", cfg::settings::toggle_key);
						char btn_label[80];
						sprintf_s(btn_label, sizeof(btn_label), "Enable/Disable: %s", key_label);
						if (ImGui::Button(btn_label, ImVec2(-1, 0)))
							waiting_for_toggle_key = true;
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Click to rebind the enable/disable toggle key");
					}

					static bool waiting_for_panic_key = false;
					if (waiting_for_panic_key) {
						ImGui::Button("Press any key...", ImVec2(-1, 0));
						for (int i = 1; i < 256; i++) {
							if (i == VK_LBUTTON || i == VK_RBUTTON || i == VK_MBUTTON) continue;
							if (GetAsyncKeyState(i) & 0x8000) {
								cfg::settings::panic_key = i;
								waiting_for_panic_key = false;
								break;
							}
						}
					} else {
						char panic_label[64];
						UINT sc = MapVirtualKey(cfg::settings::panic_key, MAPVK_VK_TO_VSC);
						if (GetKeyNameTextA((LONG)(sc << 16), panic_label, 64) == 0)
							sprintf_s(panic_label, sizeof(panic_label), "0x%X", cfg::settings::panic_key);
						char panic_btn[80];
						sprintf_s(panic_btn, sizeof(panic_btn), "Panic Key: %s", panic_label);
						if (ImGui::Button(panic_btn, ImVec2(-1, 0)))
							waiting_for_panic_key = true;
						if (ImGui::IsItemHovered())
							ImGui::SetItemTooltip("Press this key to instantly disable everything and close");
					}

					ImGui::Text("Notes");
					ImGui::Separator();
					ImGui::TextWrapped(
						"If you experience bad performance/lag try the following:\n"
						"\t- Disable ESP VSync: Look up > VSync: Un-Check\n"
						"\t- Disable VSync in game: ...Advanced Video > V-Sync: Disabled\n"
						"\t- Last Resort: Disable \"Free CPU\" option, it will inpact on your overall performace, but improve latency\n"
					);

#ifdef _DEBUG
					ImGui::Text("Dev");
					ImGui::Separator();

					if (ImGui::Checkbox("Console", &cfg::dev::console))
						if (!cfg::dev::console) LogHelper::Free();

					static int key_out;
					if (ImGui::Button("Open Menu Key"))
					{
						for (int i = ImGuiKey_NamedKey_BEGIN; i < ImGuiKey_NamedKey_END; i++)
						{
							if (ImGui::IsKeyPressed((ImGuiKey)i))
							{
								key_out = i;
								LOGF(VERBOSE, "Changed the open menu key to {}", key_out);
								break;
							}
						}
					}

					ImGui::SliderInt("Cache Refresh Rate", &cfg::dev::cache_refresh_rate, 0, 100, "%dms");
					ImGui::Checkbox("Force Show Flags", &cfg::dev::force_show_flags);
#endif
				}
			}
			ImGui::EndChild();

			ImGui::EndDisabled();


			ImGui::EndChild();
		}

	}

	ImGui::End();
}

void Menu::SetupStyles() {
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

	style.Colors[ImGuiCol_WindowBg] = ImColor(10, 10, 10);
	style.Colors[ImGuiCol_ChildBg] = ImColor(10, 10, 10);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_Border] = ImColor(50, 50, 50);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	style.Colors[ImGuiCol_FrameBg] = ImColor(75, 75, 75);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);

	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);

	style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.08f, 0.50f, 0.72f, 1.00f);

	style.Colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.67f, 0.67f, 0.67f, 0.39f);
	style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.41f, 0.42f, 0.44f, 1.00f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);

	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.29f, 0.30f, 0.31f, 0.67f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.29f, 0.30f, 0.31f, 0.95f);

	style.Colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.09f, 0.83f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.33f, 0.34f, 0.36f, 0.83f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.23f, 0.23f, 0.24f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.08f, 0.08f, 0.09f, 1.00f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.13f, 0.14f, 0.15f, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.11f, 0.64f, 0.92f, 1.00f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	style.FrameBorderSize = 1.0f;

	// Window & Frame
	style.WindowRounding = 12.f;
	style.ChildRounding = 10.f;

	style.FrameRounding = 5.f;
	style.PopupRounding = 5.f;

	style.GrabRounding = 3.f;

	auto& io = ImGui::GetIO();

	io.Fonts->Clear();
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 16.0f);

	ImFontConfig merge_icon_cfg{};
	merge_icon_cfg.FontDataOwnedByAtlas = false;
	merge_icon_cfg.MergeMode = true;
	merge_icon_cfg.GlyphOffset = Vec2_t(0, 3.5f);

	// the icons will use the size specified when getting added so it ignores the base size
	static const ImWchar icon_ranges[] = { 0xE100, 0xE108, 0 };
	io.Fonts->AddFontFromMemoryTTF(icons_font, icons_font_len, 20.f, &merge_icon_cfg, icon_ranges);
}

void Menu::RenderStartupHelpImpl() {
	static bool has_opened_menu = false;

	if (has_opened_menu)
		return;

	auto& io = ImGui::GetIO();
	auto screen = io.DisplaySize;
	auto d = ImGui::GetBackgroundDrawList();

	if (Renderer::IsOpen())
		has_opened_menu = true;

	auto help = "To OPEN the menu, Use Insert or Right Shift keys"
		"\n\t\t\t\tTo CLOSE, press the End key";
	auto size = ImGui::CalcTextSize(help);

	d->AddText(
		ImVec2(screen.x / 2 - size.x / 2, 80),
		IM_COL32(255, 255, 255, 255),
		help
	);
}

