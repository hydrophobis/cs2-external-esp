#include "Aimbot.hpp"
#include "core/engine/Engine.hpp"
#include <thread>
#include <cmath>

void Aimbot::Init() {
    std::thread(Aimbot::Thread).detach();
}

void Aimbot::Thread() {
    Vec2_t oldPunch = { 0.f, 0.f };
    float rcsRemainderX = 0.f;
    float rcsRemainderY = 0.f;
    float aimbotRemainderX = 0.f;
    float aimbotRemainderY = 0.f;

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

                Vec2_t headPos;
                auto head_bone = player.bone_list[bone_index::head];

                if (!snapshot.game.view_matrix.wts(head_bone.pos, Vec2_t(screenX * 2, screenY * 2), headPos, false)) continue;

                float dist = sqrt(pow(headPos.x - screenCenter.x, 2) + pow(headPos.y - screenCenter.y, 2));
                if (dist < bestDist) {
                    bestDist = dist;
                    bestTarget = &player;
                    bestTargetPos = headPos;
                }
            }

            if (bestTarget) {
                hasTarget = true;
                float smooth = cfg::aimbot::smooth > 0.1f ? cfg::aimbot::smooth : 1.0f;
                // Offset target by current punch
                if (cfg::aimbot::rcs && snapshot.local.shotsFired > 0) {
                    Vec2_t punch = snapshot.local.aimPunch;
                    float rcsScale = -1.f; // no idea why this needs to be negative but eh
                    float pixelScale = 15.0f;
                    bestTargetPos.x -= punch.y * rcsScale * pixelScale;
                    bestTargetPos.y += punch.x * rcsScale * pixelScale;
                }

                float aimX = (bestTargetPos.x - screenCenter.x) / smooth + aimbotRemainderX;
                float aimY = (bestTargetPos.y - screenCenter.y) / smooth + aimbotRemainderY;
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
                Vec2_t punch = local.aimPunch;
                Vec2_t delta = { punch.x - oldPunch.x, punch.y - oldPunch.y };

                float rcsScale = 2.0f;
                float pixelScale = 15.0f;

                float rcsX = (delta.y) * rcsScale * pixelScale + rcsRemainderX;
                float rcsY = -(delta.x) * rcsScale * pixelScale + rcsRemainderY;
                rcsRemainderX = 0.f;
                rcsRemainderY = 0.f;
                totalMoveX += rcsX;
                totalMoveY += rcsY;

                oldPunch = local.aimPunch;
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
