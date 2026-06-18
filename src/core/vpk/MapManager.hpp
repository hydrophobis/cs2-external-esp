#pragma once
#include <string>
#include <vector>
#include <memory>
#include "VpkParser.hpp"

class MapManager {
public:
    static MapManager& Get() {
        static MapManager instance;
        return instance;
    }

    void Update(const std::string& mapName);
    bool IsMapLoaded() const { return m_mapLoaded; }
    const std::string& GetCurrentMapName() const { return m_currentMapName; }
    const VpkParser& GetVpkParser() const { return m_vpkParser; }

private:
    MapManager() = default;
    ~MapManager() = default;

    std::string m_currentMapName;
    VpkParser m_vpkParser;
    bool m_mapLoaded = false;
};
