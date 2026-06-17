#pragma once
#include "core/engine/cache/Cache.hpp"
#include <mutex>
#include <vector>

class Misc {
public:
    struct HitMarker {
        Vec2_t screen_pos;
        float created_at;
    };

    static std::mutex hit_marker_mutex;
    static std::vector<HitMarker> hit_markers;

    static void Init();
private:
    static void Thread();
};
