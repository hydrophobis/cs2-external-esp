#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

#pragma pack(push, 1)
struct VPKHeader_v1 {
    uint32_t Signature;    // Always 0x55aa1234
    uint32_t Version;      // 1 or 2
    uint32_t TreeSize;     // Size of directory tree in bytes
};

struct VPKHeader_v2 {
    uint32_t Signature;          // Always 0x55aa1234
    uint32_t Version;            // 2
    uint32_t TreeSize;           // Size of directory tree in bytes
    uint32_t FileDataSectionSize;
    uint32_t ArchiveMD5SectionSize;
    uint32_t OtherMD5SectionSize;
    uint32_t SignatureSectionSize;
};

struct VPKDirectoryEntry {
    uint32_t CRC;
    uint16_t PreloadBytes;
    uint16_t ArchiveIndex; // 0x7fff if data is in this file
    uint32_t EntryOffset;
    uint32_t EntryLength;
    uint16_t Terminator;   // Always 0xffff
};
#pragma pack(pop)

struct VpkFileEntry {
    uint32_t crc;
    uint16_t preloadBytes;
    uint16_t archiveIndex;
    uint32_t entryOffset;
    uint32_t entryLength;
    std::vector<uint8_t> preloadData;
    std::string extension;
    std::string path;
    std::string filename;
};

class VpkParser {
public:
    VpkParser() = default;
    ~VpkParser() = default;

    // Load VPK directory tree
    bool Load(const std::string& filepath);

    // Check if internal file path exists (case-insensitive, e.g. "maps/de_dust2/entities/default_ents.vents_c")
    bool HasFile(const std::string& internalPath) const;

    // Read full contents of an internal file (case-insensitive)
    bool ReadFile(const std::string& internalPath, std::vector<uint8_t>& outBytes) const;

    // Get all file paths in the VPK
    std::vector<std::string> GetFileList() const;

private:
    std::string m_vpkPath;
    std::unordered_map<std::string, VpkFileEntry> m_entries;
    uint32_t m_headerSize = 0;
    uint32_t m_treeSize = 0;
};
