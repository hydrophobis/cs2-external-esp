#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>

#include "classes/utils.h"
#include "memory/memory.hpp"
#include "classes/vector.hpp"
#include "hacks/reader.hpp"
//#include "classes/hack.hpp"
#include "classes/globals.hpp"
#include "classes/auto_updater.hpp"

bool finish = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        std::cout << "[overlay] Window created successfully" << std::endl;
        Beep(500, 100);
        break;

    case WM_SIZE:
        GetClientRect(hWnd, &g::gameBounds);
        break;

    case WM_PAINT:
        // Drawing logic removed
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void read_thread()
{
    while (!finish)
    {
        g_game.loop();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

int main() {
    utils.update_console_title();

    std::cout << "[info] Github Repository: https://github.com/IMXNOOBX/cs2-external-esp" << std::endl;
    std::cout << "[info] Unknowncheats thread: https://www.unknowncheats.me/forum/counter-strike-2-releases/600259-cs2-external-esp.html\n" << std::endl;

    std::cout << "[config] Reading configuration." << std::endl;
    if (config::read())
        std::cout << "[updater] Successfully read configuration file\n" << std::endl;
    else
        std::cout << "[updater] Error reading config file, resetting to the default state\n" << std::endl;

#ifndef _UC
    try {
        updater::check_and_update(config::automatic_update);
    }
    catch (std::exception& e) {
        std::cout << "An exception was caught while reading " << e.what() << std::endl;
    }
#endif

    std::cout << "[updater] Reading offsets from file offsets.json." << std::endl;
    if (updater::read())
        std::cout << "[updater] Successfully read offsets file\n" << std::endl;
    else
        std::cout << "[updater] Error reading offsets file, resetting to the default state\n" << std::endl;

    g_game.init();

    if (g_game.buildNumber != updater::build_number) {
        std::cout << "[cs2] Build number doesnt match, the game has been updated and this esp most likely wont work." << std::endl;
        std::cout << "[warn] If the esp doesnt work, consider updating offsets manually in the file offsets.json" << std::endl;
        std::cout << "[cs2] Press any key to continue" << std::endl;
        std::cin.get();
    }
    else {
        std::cout << "[cs2] Offsets seem to be up to date! have fun!" << std::endl;
    }

    std::cout << "[overlay] Waiting to focus game to create the overlay..." << std::endl;
    std::cout << "[overlay] Make sure your game is in \"Full Screen Windowed\"" << std::endl;
    while (GetForegroundWindow() != g_game.process->hwnd_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        g_game.process->UpdateHWND();
        ShowWindow(g_game.process->hwnd_, TRUE);
    }

    std::cout << "[overlay] Creating window overlay..." << std::endl;

    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongA(g_game.process->hwnd_, (-6)));
    wc.lpszMenuName = " ";
    wc.lpszClassName = " ";

    RegisterClassExA(&wc);

    GetClientRect(g_game.process->hwnd_, &g::gameBounds);

    HINSTANCE hInstance = NULL;
    HWND hWnd = CreateWindowExA(WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        " ", "cs2-external-esp", WS_POPUP,
        g::gameBounds.left, g::gameBounds.top,
        g::gameBounds.right - g::gameBounds.left,
        g::gameBounds.bottom + g::gameBounds.left,
        NULL, NULL, hInstance, NULL);

    if (hWnd == NULL)
        return 0;

    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ShowWindow(hWnd, TRUE);

    std::cout << "[overlay] Window initialized successfully (no drawing logic)\n";

    std::thread read(read_thread);

    std::cout << "\n[settings] In Game keybinds:\n\t[INSERT] Toggle GUI (disabled)\n\t[END] Unload\n" << std::endl;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && !finish)
    {
        if (GetAsyncKeyState(VK_END) & 0x8000) finish = true;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (read.joinable())
        read.join();

    std::cout << "[overlay] Destroying overlay window." << std::endl;

    DestroyWindow(hWnd);
    g_game.close();

#ifdef NDEBUG
    std::cout << "[cs2] Press any key to close" << std::endl;
    std::cin.get();
#endif

    return 1;
}
