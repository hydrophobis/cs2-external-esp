#include "Aimbot.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"
#include <thread>
#include <cmath>
#include <cstdlib>
#include <algorithm>

void Aimbot::Init() {
	std::thread(Aimbot::Thread).detach();
}

void Aimbot::Thread() {
	Vec2_t oldPunch = { 0.f, 0.f };
	float rcsRemainderX = 0.f;
	float rcsRemainderY = 0.f;

	// simple LCG random for deterministicish output
	static unsigned int rng_state = 12345;
	auto rng_float = [&]() -> float {
		rng_state = rng_state * 1103515245 + 12345;
		return ((rng_state >> 16) & 0x7FFF) / 32767.f; // 0.0 - 1.0
	};

	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		if (!cfg::enabled) continue;

		auto p = Engine::GetProcess();
		auto client = Engine::GetClient();
		if (!p) continue;

		auto snapshot = Cache::CopySnapshot();
		if (!snapshot.local.alive) continue;

		if (!snapshot.local.pawn_addr) continue;

		bool aimKeyDown = (GetAsyncKeyState(cfg::aimbot::hotkey) & 0x8000) != 0;
		bool aimActive = cfg::aimbot::always_on ? !aimKeyDown : aimKeyDown;

		// Angle writing
		if (cfg::aimbot::enabled && cfg::aimbot::angle_write && aimActive) {
			uintptr_t inputPtr = p->read<uintptr_t>(client.base + offsets::csgoInput);
			if (!inputPtr) continue;

			Vec3_t currentAngles = p->read<Vec3_t>(inputPtr + offsets::input::viewAngles);

			// local eye pos
			Vec3_t viewOffset = p->read<Vec3_t>(snapshot.local.pawn_addr + offsets::pawn::m_vecViewOffset);
			Vec3_t localEye = snapshot.local.pos + viewOffset;

			// best target
			Player* bestTarget = nullptr;
			Vec3_t bestTargetPos = { 0, 0, 0 };
			float bestAngleDist = cfg::aimbot::fov;

			for (auto& player : snapshot.players) {
				if (!player.alive || player.localplayer) continue;
				if (player.team == snapshot.local.team) continue;
				if (cfg::aimbot::visible_only && !player.visible) continue;
				if (player.bone_list.empty()) continue;

				std::vector<int> bonesToCheck;
				if (cfg::aimbot::multi_bone) {
					bonesToCheck = cfg::aimbot::bone_priority;
				} else {
					bonesToCheck = { cfg::aimbot::bone };
				}

				for (int boneIdx : bonesToCheck) {
					if (boneIdx < 0 || boneIdx >= (int)player.bone_list.size()) continue;

					Vec3_t aimPos = player.bone_list[boneIdx].pos;

					if (cfg::aimbot::velocity_comp) {
						float scale = cfg::aimbot::velocity_comp_scale;
						aimPos.x += (player.vel.x - snapshot.local.vel.x) * scale;
						aimPos.y += (player.vel.y - snapshot.local.vel.y) * scale;
						aimPos.z += (player.vel.z - snapshot.local.vel.z) * scale;
					}

					Vec3_t delta = aimPos - localEye;
					Vec3_t targetAngle = delta.ToAngle();

					Vec3_t angleDelta = Vec3_t::AngleNormalize(targetAngle - currentAngles);
					float angleDist = sqrtf(angleDelta.x * angleDelta.x + angleDelta.y * angleDelta.y);

					if (angleDist < bestAngleDist) {
						bestAngleDist = angleDist;
						bestTarget = &player;
						bestTargetPos = aimPos;
					}
				}
			}

			if (bestTarget) {
				Vec3_t delta = bestTargetPos - localEye;
				Vec3_t targetAngle = delta.ToAngle();

				Vec3_t compensatedAngle = targetAngle;
				if (cfg::aimbot::rcs && snapshot.local.shotsFired > 0) {
					compensatedAngle.x -= snapshot.local.aimPunch.x * cfg::aimbot::rcs_x;
					compensatedAngle.y -= snapshot.local.aimPunch.y * cfg::aimbot::rcs_y;
				}

				Vec3_t angleDelta = Vec3_t::AngleNormalize(compensatedAngle - currentAngles);

				float smooth = cfg::aimbot::smooth > 0.1f ? cfg::aimbot::smooth : 1.0f;

				float effectiveSmooth = smooth;
				if (cfg::aimbot::humanize) {
					float variance = cfg::aimbot::smooth_variance;
					effectiveSmooth = smooth * (1.f - variance + rng_float() * variance * 2.f);
				}

				angleDelta.x /= effectiveSmooth;
				angleDelta.y /= effectiveSmooth;
				angleDelta.z = 0.f;

				if (cfg::aimbot::humanize && cfg::aimbot::jitter > 0.f) {
					float j = cfg::aimbot::jitter;
					angleDelta.x += (rng_float() - 0.5f) * j * 0.1f;
					angleDelta.y += (rng_float() - 0.5f) * j * 0.1f;
				}
				if (cfg::aimbot::soft_aim) {
					float cap = cfg::aimbot::soft_aim_max_move;
					angleDelta.x = std::clamp(angleDelta.x, -cap, cap);
					angleDelta.y = std::clamp(angleDelta.y, -cap, cap);
				}

				Vec3_t newAngle = Vec3_t::AngleNormalize(currentAngles + angleDelta);
				newAngle.x = std::clamp(newAngle.x, -89.f, 89.f);
				newAngle.z = 0.f;

				p->write<float>(inputPtr + offsets::input::viewAngles + 0, newAngle.x);
				p->write<float>(inputPtr + offsets::input::viewAngles + 4, newAngle.y);
				p->write<float>(inputPtr + offsets::input::viewAngles + 8, 0.f);
			}

			if (!bestTarget && cfg::aimbot::rcs && snapshot.local.shotsFired > 0) {
				Vec3_t punch = { snapshot.local.aimPunch.x, snapshot.local.aimPunch.y, 0.f };

				Vec3_t compensation = Vec3_t::AngleNormalize(currentAngles + Vec3_t(
					-(punch.x - oldPunch.x) * cfg::aimbot::rcs_x,
					-(punch.y - oldPunch.y) * cfg::aimbot::rcs_y,
					0.f
				));

				compensation.x = std::clamp(compensation.x, -89.f, 89.f);
				compensation.z = 0.f;

				p->write<float>(inputPtr + offsets::input::viewAngles + 0, compensation.x);
				p->write<float>(inputPtr + offsets::input::viewAngles + 4, compensation.y);
				p->write<float>(inputPtr + offsets::input::viewAngles + 8, 0.f);

				oldPunch = snapshot.local.aimPunch;
			} else if (snapshot.local.shotsFired == 0) {
				oldPunch = { 0.f, 0.f };
			}

			continue; 
		}

		// mouse aimbot
		float screenX = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)) / 2.f;
		float screenY = static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) / 2.f;
		Vec2_t screenCenter = { screenX, screenY };

		float totalMoveX = 0.f;
		float totalMoveY = 0.f;

		bool hasTarget = false;

		// Aimbot
		if (cfg::aimbot::enabled && !cfg::aimbot::angle_write && aimActive) {
			Player* bestTarget = nullptr;
			float bestDist = cfg::aimbot::fov * 10.f;
			Vec2_t bestTargetPos = { 0, 0 };

			for (auto& player : snapshot.players) {
				if (!player.alive || player.localplayer) continue;
				if (player.team == snapshot.local.team) continue;
				if (cfg::aimbot::visible_only && !player.visible) continue;
				if (player.bone_list.empty()) continue;

				std::vector<int> bonesToCheck;
				if (cfg::aimbot::multi_bone) {
					bonesToCheck = cfg::aimbot::bone_priority;
				} else {
					bonesToCheck = { cfg::aimbot::bone };
				}

				for (int boneIdx : bonesToCheck) {
					if (boneIdx < 0 || boneIdx >= (int)player.bone_list.size()) continue;
					auto target_bone = player.bone_list[boneIdx];

					Vec3_t aimPos = target_bone.pos;
					if (cfg::aimbot::velocity_comp) {
						float scale = cfg::aimbot::velocity_comp_scale;
						Vec3_t relVel = {
							player.vel.x - snapshot.local.vel.x,
							player.vel.y - snapshot.local.vel.y,
							player.vel.z - snapshot.local.vel.z
						};
						aimPos.x += relVel.x * scale;
						aimPos.y += relVel.y * scale;
						aimPos.z += relVel.z * scale;
					}

					Vec2_t bonePos;
					if (!snapshot.game.view_matrix.wts(aimPos, Vec2_t(screenX * 2, screenY * 2), bonePos, false)) continue;

					float dist = sqrt(pow(bonePos.x - screenCenter.x, 2) + pow(bonePos.y - screenCenter.y, 2));
					if (dist < bestDist) {
						bestDist = dist;
						bestTarget = &player;
						bestTargetPos = bonePos;
					}
				}
			}

			if (bestTarget) {
				hasTarget = true;
				float smooth = cfg::aimbot::smooth > 0.1f ? cfg::aimbot::smooth : 1.0f;

				// Humanize: randomize smooth and add jitter per tick
				float effectiveSmooth = smooth;
				if (cfg::aimbot::humanize) {
					float variance = cfg::aimbot::smooth_variance;
					effectiveSmooth = smooth * (1.f - variance + rng_float() * variance * 2.f);
				}

				float aimX = (bestTargetPos.x - screenCenter.x) / effectiveSmooth;
				float aimY = (bestTargetPos.y - screenCenter.y) / effectiveSmooth;

				if (cfg::aimbot::humanize && cfg::aimbot::jitter > 0.f) {
					float j = cfg::aimbot::jitter;
					aimX += (rng_float() - 0.5f) * j;
					aimY += (rng_float() - 0.5f) * j;
				}

				if (cfg::aimbot::soft_aim) {
					float cap = cfg::aimbot::soft_aim_max_move;
					aimX = std::max(-cap, std::min(cap, aimX));
					aimY = std::max(-cap, std::min(cap, aimY));
				}

				aimX += rcsRemainderX;
				aimY += rcsRemainderY;
				rcsRemainderX = 0.f;
				rcsRemainderY = 0.f;
				totalMoveX += aimX;
				totalMoveY += aimY;
			}
		}

		if (!hasTarget) {
			rcsRemainderX = 0.f;
			rcsRemainderY = 0.f;
		}

		if (cfg::aimbot::rcs && !cfg::aimbot::angle_write) {
			auto& local = snapshot.local;

			if (local.shotsFired == 0) {
				oldPunch = { 0.f, 0.f };
				rcsRemainderX = 0.f;
				rcsRemainderY = 0.f;
			} else if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
				Vec2_t punch = local.aimPunch;
				Vec2_t delta = { punch.x - oldPunch.x, punch.y - oldPunch.y };

				float sens = cfg::aimbot::sensitivity;
				float sensScale = 1.f / (sens * 0.011f);

				float rcsX = delta.y * cfg::aimbot::rcs_x * sensScale + rcsRemainderX;
				float rcsY = -delta.x * cfg::aimbot::rcs_y * sensScale + rcsRemainderY;
				rcsRemainderX = 0.f;
				rcsRemainderY = 0.f;
				totalMoveX += rcsX;
				totalMoveY += rcsY;

				oldPunch = local.aimPunch;
			} else {
				oldPunch = { 0.f, 0.f };
				rcsRemainderX = 0.f;
				rcsRemainderY = 0.f;
			}
		} else if (!cfg::aimbot::angle_write) {
			oldPunch = { 0.f, 0.f };
			rcsRemainderX = 0.f;
			rcsRemainderY = 0.f;
		}

		// Only do mouse moves when not in write mode
		if (!cfg::aimbot::angle_write) {
			int moveX = static_cast<int>(totalMoveX);
			int moveY = static_cast<int>(totalMoveY);

			float fracX = totalMoveX - static_cast<float>(moveX);
			float fracY = totalMoveY - static_cast<float>(moveY);

			if (hasTarget) {
				rcsRemainderX = fracX;
				rcsRemainderY = fracY; // reuse as aimbot remainder in non-angle mode
			}
			if (cfg::aimbot::rcs && snapshot.local.shotsFired > 0) {
				rcsRemainderX = fracX;
				rcsRemainderY = fracY;
			}

			if (moveX != 0 || moveY != 0) {
				mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(moveX), static_cast<DWORD>(moveY), 0, 0);
			}
		}
	}
}
