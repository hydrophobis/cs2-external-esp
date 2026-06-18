#include "VpkParser.hpp"
#include <fstream>
#include <algorithm>
#include <iostream>
#include "core/logger/LogHelper.hpp"

// Helper to read null-terminated string
static std::string ReadString(std::ifstream& file) {
    std::string str;
    char c;
    while (file.get(c) && c != '\0') {
        str += c;
    }
    return str;
}

static std::string getArchiveName(const std::string& dirVpkPath, uint16_t archiveIndex) {
    // Find "_dir.vpk" at the end of the path and replace it with _%03d.vpk
    size_t pos = dirVpkPath.rfind("_dir.vpk");
    if (pos != std::string::npos) {
        char buf[64];
        sprintf_s(buf, "_%03d.vpk", archiveIndex);
        return dirVpkPath.substr(0, pos) + buf;
    }
    // If the path doesn't end with _dir.vpk, but we are looking for archives,
    // e.g. it was just de_dust2.vpk. This shouldn't normally happen, but let's try replacing ".vpk"
    pos = dirVpkPath.rfind(".vpk");
    if (pos != std::string::npos) {
        char buf[64];
        sprintf_s(buf, "_%03d.vpk", archiveIndex);
        return dirVpkPath.substr(0, pos) + buf;
    }
    return dirVpkPath;
}

bool VpkParser::Load(const std::string& filepath) {
    m_vpkPath = filepath;
    m_entries.clear();

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        LOGF(WARNING, "VpkParser::Load: Failed to open file: {}", filepath);
        return false;
    }

    VPKHeader_v1 header1;
    file.read(reinterpret_cast<char*>(&header1), sizeof(VPKHeader_v1));
    if (!file || header1.Signature != 0x55AA1234) {
        LOGF(WARNING, "VpkParser::Load: Invalid signature or file too small: {}", filepath);
        return false;
    }

    if (header1.Version == 1) {
        m_headerSize = sizeof(VPKHeader_v1);
        m_treeSize = header1.TreeSize;
    } else if (header1.Version == 2) {
        VPKHeader_v2 header2;
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(&header2), sizeof(VPKHeader_v2));
        if (!file) {
            LOGF(WARNING, "VpkParser::Load: Failed to read VPK v2 header");
            return false;
        }
        m_headerSize = sizeof(VPKHeader_v2);
        m_treeSize = header2.TreeSize;
    } else {
        LOGF(WARNING, "VpkParser::Load: Unsupported VPK version: {}", header1.Version);
        return false;
    }

    file.seekg(m_headerSize, std::ios::beg);

    while (true) {
        std::string extension = ReadString(file);
        if (extension.empty()) {
            break;
        }

        while (true) {
            std::string path = ReadString(file);
            if (path.empty()) {
                break;
            }

            while (true) {
                std::string filename = ReadString(file);
                if (filename.empty()) {
                    break;
                }

                VPKDirectoryEntry entry;
                file.read(reinterpret_cast<char*>(&entry), sizeof(VPKDirectoryEntry));
                if (!file) {
                    LOGF(WARNING, "VpkParser::Load: Failed to read VPKDirectoryEntry for {}/{}.{}", path, filename, extension);
                    return false;
                }

                VpkFileEntry fileEntry;
                fileEntry.crc = entry.CRC;
                fileEntry.preloadBytes = entry.PreloadBytes;
                fileEntry.archiveIndex = entry.ArchiveIndex;
                fileEntry.entryOffset = entry.EntryOffset;
                fileEntry.entryLength = entry.EntryLength;
                fileEntry.extension = extension;
                fileEntry.path = path;
                fileEntry.filename = filename;

                if (entry.PreloadBytes > 0) {
                    fileEntry.preloadData.resize(entry.PreloadBytes);
                    file.read(reinterpret_cast<char*>(fileEntry.preloadData.data()), entry.PreloadBytes);
                    if (!file) {
                        LOGF(WARNING, "VpkParser::Load: Failed to read preload data for {}/{}.{}", path, filename, extension);
                        return false;
                    }
                }

                std::string fullInternalPath;
                if (!path.empty()) {
                    fullInternalPath = path + "/" + filename + "." + extension;
                } else {
                    fullInternalPath = filename + "." + extension;
                }

                std::string lookupPath = fullInternalPath;
                std::transform(lookupPath.begin(), lookupPath.end(), lookupPath.begin(), ::tolower);

                m_entries[lookupPath] = std::move(fileEntry);
            }
        }
    }

    LOGF(INFO, "VpkParser::Load: Loaded {} entries from {}", m_entries.size(), filepath);
    return true;
}

bool VpkParser::HasFile(const std::string& internalPath) const {
    std::string normPath = internalPath;
    std::replace(normPath.begin(), normPath.end(), '\\', '/');
    std::transform(normPath.begin(), normPath.end(), normPath.begin(), ::tolower);

    return m_entries.find(normPath) != m_entries.end();
}

bool VpkParser::ReadFile(const std::string& internalPath, std::vector<uint8_t>& outBytes) const {
    std::string normPath = internalPath;
    std::replace(normPath.begin(), normPath.end(), '\\', '/');
    std::transform(normPath.begin(), normPath.end(), normPath.begin(), ::tolower);

    auto it = m_entries.find(normPath);
    if (it == m_entries.end()) {
        return false;
    }

    const auto& entry = it->second;
    outBytes.clear();
    outBytes.reserve(entry.preloadBytes + entry.entryLength);

    if (entry.preloadBytes > 0) {
        outBytes.insert(outBytes.end(), entry.preloadData.begin(), entry.preloadData.end());
    }

    if (entry.entryLength > 0) {
        std::string filePath = m_vpkPath;
        if (entry.archiveIndex != 0x7fff) {
            filePath = getArchiveName(m_vpkPath, entry.archiveIndex);
        }

        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            LOGF(WARNING, "VpkParser::ReadFile: Failed to open archive file: {}", filePath);
            return false;
        }

        uint64_t actualOffset = entry.entryOffset;
        if (entry.archiveIndex == 0x7fff) {
            uint32_t treeEndOffset = m_headerSize + m_treeSize;
            actualOffset = treeEndOffset + entry.entryOffset;
        }

        file.seekg(actualOffset, std::ios::beg);
        std::vector<uint8_t> dataBytes(entry.entryLength);
        file.read(reinterpret_cast<char*>(dataBytes.data()), entry.entryLength);
        if (!file) {
            LOGF(WARNING, "VpkParser::ReadFile: Failed to read {} bytes at offset {} from {}", entry.entryLength, actualOffset, filePath);
            return false;
        }

        outBytes.insert(outBytes.end(), dataBytes.begin(), dataBytes.end());
    }

    return true;
}

std::vector<std::string> VpkParser::GetFileList() const {
    std::vector<std::string> paths;
    paths.reserve(m_entries.size());
    for (const auto& pair : m_entries) {
        paths.push_back(pair.first);
    }
    return paths;
}
