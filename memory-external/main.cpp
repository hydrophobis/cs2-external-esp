#include <iostream>
#include <thread>
#include <chrono>
#include <windows.h>
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_win32.h"
#include "imgui/backends/imgui_impl_dx11.h"

#include "classes/utils.h"
#include "memory/memory.hpp"
#include "classes/vector.hpp"
#include "hacks/reader.hpp"
#include "hacks/hack.hpp"
#include "classes/globals.hpp"
#include "classes/render.hpp"
#include "classes/auto_updater.hpp"

bool finish = false;

// Forward declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Forward declaration
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
<<<<<<< Updated upstream
<<<<<<< Updated upstream
	switch (message)
	{
	case WM_CREATE:
	{
		g::hdcBuffer = CreateCompatibleDC(NULL);
		g::hbmBuffer = CreateCompatibleBitmap(GetDC(hWnd), g::gameBounds.right, g::gameBounds.bottom);
		SelectObject(g::hdcBuffer, g::hbmBuffer);

		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
=======
=======
>>>>>>> Stashed changes
    // Let ImGui handle input when GUI is shown and has focus
    if (show_gui && gui_has_focus)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
            return true;
    }

    switch (message)
    {
    case WM_CREATE:
        overlay_hwnd = hWnd;
        std::cout << "[overlay] Window created successfully" << std::endl;
        Beep(500, 100);
        break;

    case WM_SIZE:
        if (renderer.m_device != nullptr && wParam != SIZE_MINIMIZED)
        {
            GetClientRect(hWnd, &g::gameBounds);
            // Notify ImGui about resize
            if (IsImGuiInitialized())
                OnWindowResize();
        }
        break;
>>>>>>> Stashed changes

		SetLayeredWindowAttributes(hWnd, RGB(255, 255, 255), 0, LWA_COLORKEY);

		std::cout << "[overlay] Window created successfully" << std::endl;
		Beep(500, 100);
		break;
	}
	case WM_ERASEBKGND: // We handle this message to avoid flickering
		return TRUE;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		//DOUBLE BUFFERING
		FillRect(g::hdcBuffer, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));

<<<<<<< Updated upstream
<<<<<<< Updated upstream

		if (GetForegroundWindow() == g_game.process->hwnd_) {
			//render::RenderText(g::hdcBuffer, 10, 10, "cs2 | ESP", RGB(75, 175, 175), 15);
			hack::loop();
		}
=======
=======
>>>>>>> Stashed changes
            // Render ESP overlay when game is in focus or GUI is not shown
            if (GetForegroundWindow() == g_game.process->hwnd_ || !show_gui)
                hack::loop(*render::g_renderer);

            // Render ImGui on top
            if (show_gui)
                RenderImGui();

            renderer.EndFrame();
            renderer.Present();
        }
>>>>>>> Stashed changes

		BitBlt(hdc, 0, 0, g::gameBounds.right, g::gameBounds.bottom, g::hdcBuffer, 0, 0, SRCCOPY);

<<<<<<< Updated upstream
		EndPaint(hWnd, &ps);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	}
	case WM_DESTROY:
		DeleteDC(g::hdcBuffer);
		DeleteObject(g::hbmBuffer);
=======
    case WM_DESTROY:
        CleanupImGui(); // Clean up ImGui first
        renderer.Shutdown();
        PostQuitMessage(0);
        break;
>>>>>>> Stashed changes

		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

<<<<<<< Updated upstream
void read_thread() {
	while (!finish) {
		g_game.loop();
		std::this_thread::sleep_for(std::chrono::milliseconds(2));
	}
=======
void read_thread()
{
    while (!finish)
    {
        g_game.loop();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
>>>>>>> Stashed changes
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
		std::cout << "An exceptio was caught while read " << e.what() << std::endl;
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

<<<<<<< Updated upstream
	WNDCLASSEXA wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEXA);
	wc.lpfnWndProc = WndProc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hbrBackground = WHITE_BRUSH;
	wc.hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongA(g_game.process->hwnd_, (-6))); // GWL_HINSTANCE));
	wc.lpszMenuName = " ";
	wc.lpszClassName = " ";
=======
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongA(g_game.process->hwnd_, (-6)));
    wc.lpszMenuName = " ";
    wc.lpszClassName = " ";
>>>>>>> Stashed changes

	RegisterClassExA(&wc);

	GetClientRect(g_game.process->hwnd_, &g::gameBounds);

	// Create the window
	HINSTANCE hInstance = NULL;
	HWND hWnd = CreateWindowExA(WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW, " ", "cs2-external-esp", WS_POPUP,
		g::gameBounds.left, g::gameBounds.top, g::gameBounds.right - g::gameBounds.left, g::gameBounds.bottom + g::gameBounds.left, NULL, NULL, hInstance, NULL); // NULL, NULL);

	if (hWnd == NULL)
		return 0;

	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	ShowWindow(hWnd, TRUE);
	//SetActiveWindow(hack::process->hwnd_);

<<<<<<< Updated upstream
	// Launch game memory reading thread
	std::thread read(read_thread);

#ifndef _UC
	std::cout << "\n[settings] In Game keybinds:\n\t[F4] enable/disable Box ESP\n\t[F5] enable/disable Team ESP\n\t[F6] enable/disable automatic updates\n\t[F7] enable/disable extra flags\n\t[F8] enable/disable skeleton esp\n\t[F9] enable/disable head tracker\n\t[end] Unload esp.\n" << std::endl;
#else
	std::cout << "\n[settings] In Game keybinds:\n\t[F4] enable/disable Box ESP\n\t[F5] enable/disable Team ESP\n\t[F7] enable/disable extra flags\n\t[F8] enable/disable skeleton esp\n\t[F9] enable/disable head tracker\n\t[end] Unload esp.\n" << std::endl;
=======
    // Set the global renderer pointer
    render::g_renderer = &renderer;

    // Initialize ImGui with the DirectX context from renderer
    if (!InitImGui(hWnd, renderer.m_device.Get(), renderer.GetContext())) {
        std::cout << "[gui] Failed to initialize ImGui" << std::endl;
        DestroyWindow(hWnd);
        return 0;
    }

<<<<<<< Updated upstream
=======
    // Set the global renderer pointer
    render::g_renderer = &renderer;

    // Initialize ImGui with the DirectX context from renderer
    if (!InitImGui(hWnd, renderer.m_device.Get(), renderer.GetContext())) {
        std::cout << "[gui] Failed to initialize ImGui" << std::endl;
        DestroyWindow(hWnd);
        return 0;
    }

>>>>>>> Stashed changes
    // Make window transparent for overlay effect
    SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    ShowWindow(hWnd, TRUE);

    std::cout << "[overlay] DirectX 11 renderer initialized successfully" << std::endl;

    // Remove the separate GUI thread - we'll render everything in the main loop
    std::thread read(read_thread);

#ifndef _UC
    std::cout << "\n[settings] In Game keybinds:\n\t[INSERT] Toggle GUI\n\t[F4] Box ESP\n\t[F5] Team ESP\n\t[F6] Auto update\n\t[F7] Panic\n\t[F8] Skeleton ESP\n\t[F9] Head tracker\n\t[END] Unload esp.\n" << std::endl;
#else
    std::cout << "\n[settings] In Game keybinds:\n\t[INSERT] Toggle GUI\n\t[F4] Box ESP\n\t[F5] Team ESP\n\t[F7] Panic\n\t[F8] Skeleton ESP\n\t[F9] Head tracker\n\t[END] Unload esp.\n" << std::endl;
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
#endif
	std::cout << "[settings] Make sure you check the config for additional settings!" << std::endl;

	// Message loop
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) && !finish)
	{
		if (GetAsyncKeyState(VK_END) & 0x8000) finish = true;

<<<<<<< Updated upstream
		if (GetAsyncKeyState(VK_F4) & 0x8000) { config::show_box_esp = !config::show_box_esp; config::save(); Beep(700, 100); };
=======
        if (GetAsyncKeyState(VK_INSERT) & 0x8000) {
            show_gui = !show_gui;
            gui_has_focus = show_gui;

            // Change window transparency based on GUI state
            if (show_gui) {
                // Make window less transparent when GUI is shown
                SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 200, LWA_ALPHA | LWA_COLORKEY);
            }
            else {
                // Make window fully transparent when only ESP is shown
                SetLayeredWindowAttributes(hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
>>>>>>> Stashed changes

		if (GetAsyncKeyState(VK_F5) & 0x8000) { config::team_esp = !config::team_esp; config::save(); Beep(700, 100); };
#ifndef _UC
		if (GetAsyncKeyState(VK_F6) & 0x8000) { config::automatic_update = !config::automatic_update; config::save(); Beep(700, 100); }
#endif
		if (GetAsyncKeyState(VK_F7) & 0x8000) { config::show_extra_flags = !config::show_extra_flags; config::save(); Beep(700, 100); };

		if (GetAsyncKeyState(VK_F8) & 0x8000) { config::show_skeleton_esp = !config::show_skeleton_esp; config::save(); Beep(700, 100); };

		if (GetAsyncKeyState(VK_F9) & 0x8000) { config::show_head_tracker = !config::show_head_tracker; config::save(); Beep(700, 100); };

<<<<<<< Updated upstream
<<<<<<< Updated upstream
		TranslateMessage(&msg);
		DispatchMessage(&msg);
=======
=======
>>>>>>> Stashed changes
    if (read.joinable())
        read.join();
>>>>>>> Stashed changes

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	read.detach();

<<<<<<< Updated upstream
<<<<<<< Updated upstream
	Beep(700, 100); Beep(700, 100);
=======
=======
>>>>>>> Stashed changes
    // Clean up ImGui and renderer
    CleanupImGui();
    render::g_renderer = nullptr;
    renderer.Shutdown();
>>>>>>> Stashed changes

	std::cout << "[overlay] Destroying overlay window." << std::endl;
	DeleteDC(g::hdcBuffer);
	DeleteObject(g::hbmBuffer);

<<<<<<< Updated upstream
<<<<<<< Updated upstream
	DestroyWindow(hWnd);

	g_game.close();

=======
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
#ifdef NDEBUG
	std::cout << "[cs2] Press any key to close" << std::endl;
	std::cin.get();
#endif

	return 1;
}
