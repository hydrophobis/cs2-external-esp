#include "Bhop.hpp"
#include "core/engine/Engine.hpp"
#include "core/offsets/Offsets.hpp"
#include <thread>

void Bhop::Init() {
    std::thread(Bhop::Thread).detach();
}

void Bhop::Thread() {
    bool wasOnGround = false;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        if (!cfg::enabled || !cfg::misc::bhop) continue;

        auto snapshot = Cache::CopySnapshot();
        if (!snapshot.local.alive) continue;

        uintptr_t pawn = snapshot.local.pawn_addr;
        if (!pawn) continue;

        auto p = Engine::GetProcess();
        if (!p) continue;

        uint32_t flags = p->read<uint32_t>(pawn + offsets::pawn::m_fFlags);
        bool onGround = (flags & (1 << 0)) != 0; // FL_ONGROUND

        // Only hold spacebar down on the exact tick we land.
        // Release it immediately so the next frame we're airborne again.
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            if (onGround && !wasOnGround) {
                // Just touched ground — send a jump
                keybd_event(VK_SPACE, 0, 0, 0);
                keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
            } else if (!onGround) {
                // In the air — release space so the game doesn't queue another jump
                keybd_event(VK_SPACE, 0, KEYEVENTF_KEYUP, 0);
            }
        }

        wasOnGround = onGround;
    }
}
