#pragma once

#include "core/engine/cache/Cache.hpp"
#include <chrono>
#include <mutex>

class Esp {
public:
    ~Esp() = default;
    Esp(const Esp&) = delete;
    Esp(Esp&&) = delete;
    Esp& operator=(const Esp&) = delete;
    Esp& operator=(Esp&&) = delete;

    static bool Init();
    static void Render();

private:
    ImGuiIO io;
    ImFont* font;
    ImFont* font_merged_icons;
    ImDrawList* d;

    view_matrix_t matrix;
private:
    Esp() {};

    static Esp& GetInstance()
    {
        static Esp i{};
        return i;
    }

    bool InitImpl();
    void RenderImpl();

    void RenderPlayer(Player player, bool mate = false);
    void RenderPlayerBones(Player player, bool mate = false);
    void RenderPlayerBars(Player player, std::pair<Vec2_t, Vec2_t> bounds);
    void RenderPlayerFlags(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate = false);
    void RenderPlayerTracker(Player player, std::pair<Vec2_t, Vec2_t> bounds, bool mate = false);
    void RenderPlayerTracers(Player source, Player player, bool mate = false);
    void RenderPlayerVisionRay(Player local, Player player, bool mate = false);
    void RenderDroppedWeapons(const std::vector<WorldEntity>& worldEntities, Player local);
    void RenderGrenades(const std::vector<WorldEntity>& worldEntities, Player local);

	void RenderCrosshair(Player local);
    void RenderAimbotFOV();
    void RenderTriggerbotFOV();
    void RenderHitMarkers();
    void RenderBomb(Player local, Bomb bomb);
};
