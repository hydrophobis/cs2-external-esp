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

        if (cfg::aimbot::enabled) {
            float screenX = static_cast<float>(GetSystemMetrics(SM_CXSCREEN)) / 2.f;
            float screenY = static_cast<float>(GetSystemMetrics(SM_CYSCREEN)) / 2.f;
            Vec2_t screenCenter = { screenX, screenY };

            if ((GetAsyncKeyState(cfg::aimbot::hotkey) & 0x8000) != 0) {
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
                    float smooth = cfg::aimbot::smooth > 0.1f ? cfg::aimbot::smooth : 1.0f;

                    float diffX = (bestTargetPos.x - screenCenter.x) / smooth + aimbotRemainderX;
                    float diffY = (bestTargetPos.y - screenCenter.y) / smooth + aimbotRemainderY;

                    int moveX = static_cast<int>(diffX);
                    int moveY = static_cast<int>(diffY);

                    aimbotRemainderX = diffX - moveX;
                    aimbotRemainderY = diffY - moveY;

                    if (moveX != 0 || moveY != 0) {
                        mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(moveX), static_cast<DWORD>(moveY), 0, 0);
                    }
                }
            } else {
                aimbotRemainderX = 0.f;
                aimbotRemainderY = 0.f;
            }
        }

        if (cfg::aimbot::rcs) {
            auto& local = snapshot.local;

            // m_predictableBaseAngle lingers after firing stops,
            // so reset when not actively shooting
            if (local.shotsFired == 0) {
                oldPunch = { 0.f, 0.f };
                rcsRemainderX = 0.f;
                rcsRemainderY = 0.f;
                continue;
            }

            Vec2_t punch = local.aimPunch;
            Vec2_t delta = { punch.x - oldPunch.x, punch.y - oldPunch.y };

            float rcsScale = 2.0f;
            float pixelScale = 15.0f;

            float dy = -(delta.x) * rcsScale * pixelScale + rcsRemainderY;
            float dx = (delta.y) * rcsScale * pixelScale + rcsRemainderX;

            int moveX = static_cast<int>(dx);
            int moveY = static_cast<int>(dy);

            rcsRemainderX = dx - moveX;
            rcsRemainderY = dy - moveY;

            if (moveX != 0 || moveY != 0) {
                mouse_event(MOUSEEVENTF_MOVE, static_cast<DWORD>(moveX), static_cast<DWORD>(moveY), 0, 0);
            }

            oldPunch = local.aimPunch;
        } else {
            oldPunch = { 0.f, 0.f };
            rcsRemainderX = 0.f;
            rcsRemainderY = 0.f;
        }
    }
}
