#include "MapManager.hpp"
#include "core/engine/Engine.hpp"
#include "core/logger/LogHelper.hpp"
#include <filesystem>
#include <algorithm>

// Returns true if the map name looks like a real CS2 map name
// Rejects garbage strings read from uninitialized memory
static bool IsValidMapName(const std::string& name) {
    if (name.empty())
        return false;

    // Must have at least one non-whitespace character
    bool hasContent = false;
    for (char c : name) {
        // Windows forbidden path chars + control chars indicate garbage memory
        if (c == '<' || c == '>' || c == '"' || c == '|' ||
            c == '?' || c == '*' || (unsigned char)c < 0x20)
            return false;
        if (c != ' ' && c != '/')
            hasContent = true;
    }

    if (!hasContent)
        return false;

    for (char c : name)
        if (std::isalpha((unsigned char)c))
            return true;

    return false;
}

void MapManager::Update(const std::string& mapName) {
    if (mapName == m_currentMapName)
        return;

    m_currentMapName = mapName;
    m_mapLoaded = false;

    if (!IsValidMapName(mapName)) {
        return;
    }

    LOGF(INFO, "MapManager: Map change detected to: {}", mapName);

    try {
        auto process = Engine::GetProcess();
        if (!process) {
            LOGF(WARNING, "MapManager: No game process handle, cannot load VPK");
            return;
        }

        std::string procPath = process->GetProcessPath();
        if (procPath.empty()) {
            LOGF(WARNING, "MapManager: Failed to get process path for cs2.exe");
            return;
        }

        // Walk up from game/bin/win64/cs2.exe to the game root.
        std::filesystem::path gamePath(procPath);
        for (int i = 0; i < 3; ++i) {
            if (gamePath.has_parent_path())
                gamePath = gamePath.parent_path();
        }

        std::filesystem::path csgoPath = gamePath / "csgo";

        // Normalise the map name (forward slashes, trim trailing .vpk)
        std::string cleanMapName = mapName;
        std::replace(cleanMapName.begin(), cleanMapName.end(), '\\', '/');
        if (cleanMapName.size() > 4 &&
            cleanMapName.substr(cleanMapName.size() - 4) == ".vpk")
            cleanMapName = cleanMapName.substr(0, cleanMapName.size() - 4);

        // Strip leading "maps/" if present
        if (cleanMapName.rfind("maps/", 0) == 0)
            cleanMapName = cleanMapName.substr(5);

        // baseName is the bare map name, e.g. "de_dust2"
        size_t lastSlash = cleanMapName.rfind('/');
        std::string baseName = (lastSlash == std::string::npos)
                               ? cleanMapName
                               : cleanMapName.substr(lastSlash + 1);

        // Candidate VPK paths (most likely first)
        std::vector<std::filesystem::path> candidates = {
            csgoPath / "maps" / (baseName + ".vpk"),
            csgoPath / (cleanMapName + ".vpk"),
            csgoPath / cleanMapName,
        };

        std::filesystem::path vpkPath;
        for (const auto& p : candidates) {
            std::error_code ec;
            if (std::filesystem::exists(p, ec) && !ec) {
                vpkPath = p;
                break;
            }
        }

        if (vpkPath.empty()) {
            LOGF(WARNING, "MapManager: Could not find VPK for map '{}' (searched in {})",
                 baseName, csgoPath.string());
            return;
        }

        LOGF(INFO, "MapManager: Loading VPK: {}", vpkPath.string());

        if (!m_vpkParser.Load(vpkPath.string())) {
            LOGF(WARNING, "MapManager: Failed to parse VPK");
            return;
        }

        m_mapLoaded = true;
        LOGF(INFO, "MapManager: VPK loaded successfully for map '{}'", baseName);

        // Smoke-test: try extracting the entity lump
        std::string entPath = "maps/" + baseName + "/entities/default_ents.vents_c";
        if (m_vpkParser.HasFile(entPath)) {
            std::vector<uint8_t> bytes;
            if (m_vpkParser.ReadFile(entPath, bytes))
                LOGF(INFO, "MapManager: Entity lump '{}' extracted ({} bytes)",
                     entPath, bytes.size());
            else
                LOGF(WARNING, "MapManager: Failed to read entity lump: {}", entPath);
        } else {
            LOGF(WARNING, "MapManager: Entity lump not found: {}", entPath);
        }

    } catch (const std::exception& ex) {
        LOGF(WARNING, "MapManager: Exception during VPK load: {}", ex.what());
    } catch (...) {
        LOGF(WARNING, "MapManager: Unknown exception during VPK load");
    }
}
