#include "Misc.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"
#include <thread>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

std::mutex Misc::hit_marker_mutex;
std::vector<Misc::HitMarker> Misc::hit_markers;

void Misc::Init() {
    std::thread(Misc::Thread).detach();
}

void Misc::Thread() {
    bool triggerScheduled = false;
    std::chrono::steady_clock::time_point triggerFireAt{};

    float prevYaw = 0.f;

    std::unordered_map<int, int> prev_health;
    std::unordered_map<int, bool> prev_alive;
    int prev_shots_fired = 0;

    auto last_afk_move = std::chrono::steady_clock::now();

    bool zeus_on_cooldown = false;
    std::chrono::steady_clock::time_point zeus_cooldown_end{};
    bool knife_on_cooldown = false;
    std::chrono::steady_clock::time_point knife_cooldown_end{};

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!cfg::enabled) continue;

        auto p = Engine::GetProcess();
        if (!p) continue;

        auto client = Engine::GetClient();
        auto snapshot = Cache::CopySnapshot();
        auto now = std::chrono::steady_clock::now();

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
            bool trigActive = cfg::misc::triggerbot::always_on ? !keyDown : keyDown;

            if (trigActive && snapshot.local.alive) {
                float screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
                float screenH = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));
                float cx = screenW * 0.5f;
                float cy = screenH * 0.5f;
                float fov = cfg::misc::triggerbot::fov;

                bool onTarget = false;
                for (auto& player : snapshot.players) {
                    if (!player.alive || player.localplayer) continue;
                    if (player.team == snapshot.local.team) continue;
                    if (player.bone_list.empty()) continue;

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
                    triggerFireAt = now + std::chrono::milliseconds(cfg::misc::triggerbot::delay_ms);
                }
            } else {
                triggerScheduled = false;
            }

            if (triggerScheduled && now >= triggerFireAt) {
                triggerScheduled = false;
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }
        }

        if ((cfg::misc::strafe::helper || cfg::misc::strafe::autostrafe) && snapshot.local.alive) {
            uintptr_t pawn = snapshot.local.pawn_addr;
            if (pawn) {
                uint32_t flags = p->read<uint32_t>(pawn + offsets::pawn::m_fFlags);
                bool inAir = (flags & 1) == 0;

                uintptr_t inputPtr = p->read<uintptr_t>(client.base + offsets::csgoInput);
                float curYaw = 0.f;
                if (inputPtr) {
                    Vec3_t angles = p->read<Vec3_t>(inputPtr + offsets::input::viewAngles);
                    curYaw = angles.y;
                }

                float yawDelta = curYaw - prevYaw;
                while (yawDelta > 180.f)  yawDelta -= 360.f;
                while (yawDelta < -180.f) yawDelta += 360.f;

                if (inAir && cfg::misc::strafe::helper) {
                    bool aDown = (GetAsyncKeyState('A') & 0x8000) != 0;
                    bool dDown = (GetAsyncKeyState('D') & 0x8000) != 0;

                    if (yawDelta < -0.5f && aDown && !dDown) {
                        keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event('D', 0, 0, 0);
                    } else if (yawDelta > 0.5f && dDown && !aDown) {
                        keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
                        keybd_event('A', 0, 0, 0);
                    }
                }

                prevYaw = curYaw;
            }
        }

        if (cfg::misc::anti_afk && snapshot.local.alive) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_afk_move).count();
            if (elapsed >= cfg::misc::anti_afk_interval_s) {
                keybd_event('A', 0, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
                keybd_event('A', 0, KEYEVENTF_KEYUP, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(40));
                keybd_event('D', 0, 0, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
                keybd_event('D', 0, KEYEVENTF_KEYUP, 0);
                last_afk_move = now;
            }
        }

        if (snapshot.local.alive) {
            if (zeus_on_cooldown && now >= zeus_cooldown_end)
                zeus_on_cooldown = false;
            if (knife_on_cooldown && now >= knife_cooldown_end)
                knife_on_cooldown = false;

            for (auto& player : snapshot.players) {
                if (!player.alive || player.localplayer) continue;
                if (player.team == snapshot.local.team) continue;

                float dist = player.pos.dist_to_3d(snapshot.local.pos);

                if (cfg::misc::auto_zeus::enabled && !zeus_on_cooldown && dist <= cfg::misc::auto_zeus::range) {
                    keybd_event('5', 0, 0, 0);
                    keybd_event('5', 0, KEYEVENTF_KEYUP, 0);
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    zeus_on_cooldown = true;
                    zeus_cooldown_end = now + std::chrono::milliseconds(800);
                    break;
                }

                if (cfg::misc::auto_knife::enabled && !knife_on_cooldown && dist <= cfg::misc::auto_knife::range) {
                    keybd_event('3', 0, 0, 0);
                    keybd_event('3', 0, KEYEVENTF_KEYUP, 0);
                    mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                    mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
                    knife_on_cooldown = true;
                    knife_cooldown_end = now + std::chrono::milliseconds(500);
                    break;
                }
            }
        }

        if (cfg::misc::auto_queue::enabled) {
            static auto last_queue_action = std::chrono::steady_clock::now() - std::chrono::seconds(30);

            auto queue_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_queue_action).count();

            if (queue_elapsed >= 3000) {
                HWND cs2_hwnd = p->hwnd_;

                if (cs2_hwnd) {
                    RECT client_rect{};
                    GetClientRect(cs2_hwnd, &client_rect);
                    int cw = client_rect.right;
                    int ch = client_rect.bottom;

                    if (cw > 0 && ch > 0) {
                        if (!snapshot.globals.in_match) {
                            LPARAM play_pos = MAKELPARAM(cw / 2, (int)(ch * 0.92f));
                            PostMessage(cs2_hwnd, WM_LBUTTONDOWN, MK_LBUTTON, play_pos);
                            PostMessage(cs2_hwnd, WM_LBUTTONUP, 0, play_pos);
                        }

                        if (cfg::misc::auto_queue::accept_match) {
                            LPARAM accept_pos = MAKELPARAM(cw / 2, (int)(ch * 0.58f));
                            PostMessage(cs2_hwnd, WM_LBUTTONDOWN, MK_LBUTTON, accept_pos);
                            PostMessage(cs2_hwnd, WM_LBUTTONUP, 0, accept_pos);
                        }

                        last_queue_action = now;
                    }
                }
            }
        }

        bool do_hit_kill = cfg::misc::hit_marker::enabled || cfg::misc::kill_sound::enabled
            || cfg::misc::stats::enabled;

        if (do_hit_kill && snapshot.local.alive) {
            float screenW = static_cast<float>(GetSystemMetrics(SM_CXSCREEN));
            float screenH = static_cast<float>(GetSystemMetrics(SM_CYSCREEN));

            for (auto& player : snapshot.players) {
                if (player.localplayer) continue;
                if (player.team == snapshot.local.team) continue;

                int idx = (int)(uint8_t)player.index;
                int cur_hp = player.health;
                bool cur_alive = player.alive;

                auto it_hp = prev_health.find(idx);
                auto it_alive = prev_alive.find(idx);

                bool had_prev = it_hp != prev_health.end();
                int old_hp = had_prev ? it_hp->second : cur_hp;
                bool was_alive = it_alive != prev_alive.end() ? it_alive->second : cur_alive;

                if (had_prev && was_alive && cur_alive && cur_hp < old_hp) {
                    if (cfg::misc::stats::enabled)
                        cfg::misc::stats::hits++;

                    if (cfg::misc::hit_marker::enabled && !player.bone_list.empty()) {
                        Vec2_t screen;
                        if (snapshot.game.view_matrix.wts(player.bone_list[7].pos, Vec2_t(screenW, screenH), screen, false)) {
                            HitMarker hm;
                            hm.screen_pos = screen;
                            hm.created_at = std::chrono::duration<float>(
                                std::chrono::steady_clock::now().time_since_epoch()).count();
                            std::lock_guard<std::mutex> lock(hit_marker_mutex);
                            hit_markers.push_back(hm);
                        }
                    }
                }

                if (had_prev && was_alive && !cur_alive) {
                    if (cfg::misc::stats::enabled)
                        cfg::misc::stats::kills++;

                    if (cfg::misc::kill_sound::enabled) {
                        PlaySoundA("kill.wav", NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
                    }
                }

                prev_health[idx] = cur_hp;
                prev_alive[idx] = cur_alive;
            }

            int cur_shots = snapshot.local.shotsFired;
            if (cfg::misc::stats::enabled && cur_shots > prev_shots_fired)
                cfg::misc::stats::shots_fired += (cur_shots - prev_shots_fired);
            prev_shots_fired = cur_shots;
        }
    }
}
