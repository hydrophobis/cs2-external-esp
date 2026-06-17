#include "Aimbot.hpp"
#include "core/engine/Engine.hpp"
#include <thread>
#include <cmath>
#include <cstdlib>

void Aimbot::Init() {
    std::thread(Aimbot::Thread).detach();
}

void Aimbot::Thread() {
    Vec2_t oldPunch = { 0.f, 0.f };
    float rcsRemainderX = 0.f;
    float rcsRemainderY = 0.f;
    float aimbotRemainderX = 0.f;
    float aimbotRemainderY = 0.f;

    // simple LCG random for deterministicish output
    static unsigned int rng_state = 12345;
    auto rng_float = [&]() -> float {
        rng_state = rng_state * 1103515245 + 12345;
        return ((rng_state >> 16) & 0x7FFF) / 32767.f; // 0.0 - 1.0
    };

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!cfg::enabled) continue;

        auto snapshot = Cache::CopySnapshot();
        if (!snapshot.local.alive) continue;

        float screenX = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)) / 2.f;
        float screenY = static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) / 2.f;
        Vec2_t screenCenter = { screenX, screenY };

        float totalMoveX = 0.f;
        float totalMoveY = 0.f;

        bool hasTarget = false;

        // Aimbot
        bool aimKeyDown = (GetAsyncKeyState(cfg::aimbot::hotkey) & 0x8000) != 0;
        if (cfg::aimbot::enabled && aimKeyDown) {
            Player* bestTarget = nullptr;
            float bestDist = cfg::aimbot::fov * 10.f;
            Vec2_t bestTargetPos = { 0, 0 };

            for (auto& player : snapshot.players) {
                if (!player.alive || player.localplayer) continue;
                if (player.team == snapshot.local.team) continue;
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
                // Offset target by current punch (DragonBurn-style sensitivity scaling)
                if (cfg::aimbot::rcs && snapshot.local.shotsFired > 0) {
                    Vec2_t punch = snapshot.local.aimPunch;
                    float sens = cfg::aimbot::sensitivity;
                    float sensScale = 1.f / (sens * 0.011f);
                    bestTargetPos.x -= punch.y * cfg::aimbot::rcs_x * sensScale;
                    bestTargetPos.y += punch.x * cfg::aimbot::rcs_y * sensScale;
                }

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

                aimX += aimbotRemainderX;
                aimY += aimbotRemainderY;
                aimbotRemainderX = 0.f;
                aimbotRemainderY = 0.f;
                totalMoveX += aimX;
                totalMoveY += aimY;
            }
        }

        if (!hasTarget) {
            aimbotRemainderX = 0.f;
            aimbotRemainderY = 0.f;
        }

        if (cfg::aimbot::rcs && !hasTarget) {
            auto& local = snapshot.local;

            if (local.shotsFired == 0) {
                oldPunch = { 0.f, 0.f };
                rcsRemainderX = 0.f;
                rcsRemainderY = 0.f;
            } else {
                // DragonBurn-style: only apply RCS while actively firing (LMB held)
                if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
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
            }
        } else if (!cfg::aimbot::rcs) {
            oldPunch = { 0.f, 0.f };
            rcsRemainderX = 0.f;
            rcsRemainderY = 0.f;
        }

        int moveX = static_cast<int>(totalMoveX);
        int moveY = static_cast<int>(totalMoveY);

        float fracX = totalMoveX - static_cast<float>(moveX);
        float fracY = totalMoveY - static_cast<float>(moveY);
        if (hasTarget) {
            aimbotRemainderX = fracX;
            aimbotRemainderY = fracY;
        } else if (cfg::aimbot::rcs && snapshot.local.shotsFired > 0) {
            rcsRemainderX = fracX;
            rcsRemainderY = fracY;
        }

        if (moveX != 0 || moveY != 0) {
            mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(moveX), static_cast<DWORD>(moveY), 0, 0);
        }
    }
}
