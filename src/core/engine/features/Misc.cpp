#include "Misc.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"
#include <thread>
#include <cmath>
#include <chrono>

void Misc::Init() {
    std::thread(Misc::Thread).detach();
}

void Misc::Thread() {
    // Triggerbot state
    bool triggerHeld = false;
    bool triggerScheduled = false;
    std::chrono::steady_clock::time_point triggerFireAt{};

    // Auto-strafe / strafe helper state
    float prevYaw = 0.f;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!cfg::enabled) continue;

        auto p = Engine::GetProcess();
        if (!p) continue;

        auto client = Engine::GetClient();
        auto snapshot = Cache::CopySnapshot();

        if (cfg::misc::anti_flash && snapshot.local.alive && snapshot.local.pawn_addr) {
            p->write<float>(snapshot.local.pawn_addr + offsets::pawn::m_flFlashDuration, 0.f);
            p->write<float>(snapshot.local.pawn_addr + offsets::pawn::m_flFlashMaxAlpha, 0.f);
            p->write<float>(snapshot.local.pawn_addr + offsets::pawn::m_flFlashOverlayAlpha, 0.f);
        }

        if (cfg::misc::skin::enabled && snapshot.local.alive && snapshot.local.weapon_ptr) {
            short current = p->read<short>(
                snapshot.local.weapon_ptr + offsets::pawn::m_AttributeManager
                + offsets::pawn::m_Item + offsets::pawn::m_iItemDefinitionIndex);

            if (current == static_cast<short>(cfg::misc::skin::weapon_id)) {
                p->write<short>(
                    snapshot.local.weapon_ptr + offsets::pawn::m_AttributeManager
                    + offsets::pawn::m_Item + offsets::pawn::m_iItemDefinitionIndex,
                    static_cast<short>(cfg::misc::skin::skin_id));
            }
        }

        if (cfg::misc::triggerbot::enabled) {
            bool keyDown = (GetAsyncKeyState(cfg::misc::triggerbot::hotkey) & 0x8000) != 0;

            if (keyDown && snapshot.local.alive) {
                float screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
                float screenH = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
                float cx = screenW / 2.f;
                float cy = screenH / 2.f;
                float fov = cfg::misc::triggerbot::fov;

                bool onTarget = false;
                for (auto& player : snapshot.players) {
                    if (!player.alive || player.localplayer) continue;
                    if (player.team == snapshot.local.team) continue;
                    if (player.bone_list.empty()) continue;

                    // Check all bones for any that land within fov of crosshair
                    for (auto& bone : player.bone_list) {
                        Vec2_t screen;
                        if (!snapshot.game.view_matrix.wts(bone.pos, Vec2_t(screenW, screenH), screen, false))
                            continue;
                        float dx = screen.x - cx;
                        float dy = screen.y - cy;
                        if (dx * dx + dy * dy <= fov * fov) {
                            onTarget = true;
                            break;
                        }
                    }
                    if (onTarget) break;
                }

                if (onTarget && !triggerScheduled) {
                    triggerScheduled = true;
                    triggerFireAt = std::chrono::steady_clock::now()
                        + std::chrono::milliseconds(cfg::misc::triggerbot::delay_ms);
                }
            } else {
                triggerScheduled = false;
            }

            if (triggerScheduled && std::chrono::steady_clock::now() >= triggerFireAt) {
                triggerScheduled = false;
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }
        }

        if ((cfg::misc::strafe::helper || cfg::misc::strafe::autostrafe) && snapshot.local.alive) {
            uintptr_t pawn = snapshot.local.pawn_addr;
            if (pawn) {
                uint32_t flags = p->read<uint32_t>(pawn + offsets::pawn::m_fFlags);
                bool inAir = (flags & 1) == 0; // not FL_ONGROUND

                // Read current view yaw
                uintptr_t inputPtr = p->read<uintptr_t>(client.base + offsets::csgoInput);
                float curYaw = 0.f;
                if (inputPtr) {
                    Vec3_t angles = p->read<Vec3_t>(inputPtr + offsets::input::viewAngles);
                    curYaw = angles.y;
                }

                float yawDelta = curYaw - prevYaw;
                // Normalize delta to [-180, 180]
                while (yawDelta > 180.f)  yawDelta -= 360.f;
                while (yawDelta < -180.f) yawDelta += 360.f;

                if (inAir) {
                    if (cfg::misc::strafe::helper) {
                        // Strafe helper: only corrects if player is pressing the wrong key
                        bool aDown = (GetAsyncKeyState('A') & 0x8000) != 0;
                        bool dDown = (GetAsyncKeyState('D') & 0x8000) != 0;

                        // If turning right (yaw decreasing in CS2) but pressing A, fix to D
                        if (yawDelta < -0.5f && aDown && !dDown) {
                            keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
                            keybd_event('D', 0, 0, 0);
                        }
                        // If turning left (yaw increasing) but pressing D, fix to A
                        else if (yawDelta > 0.5f && dDown && !aDown) {
                            keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
                            keybd_event('A', 0, 0, 0);
                        }
                    }
                } else {
                    // On ground
                }

                prevYaw = curYaw;
            }
        }
    }
}
