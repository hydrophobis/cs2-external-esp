#include "Cache.hpp"

#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"
#include "core/vpk/MapManager.hpp"

#include "assets/fonts/WeaponIcons.h"

static bool IsGrenadeItem(short idx) {
	return idx == weapon_flashbang || idx == weapon_hegrenade || idx == weapon_frag_grenade
		|| idx == weapon_smokegrenade || idx == weapon_molotov || idx == weapon_incgrenade
		|| idx == weapon_decoy || idx == weapon_c4;
}

static const char* GrenadeIcon(short idx) {
	switch (idx) {
	case weapon_flashbang:       return WeaponIcons::FLASHBANG;
	case weapon_hegrenade:
	case weapon_frag_grenade:    return WeaponIcons::FRAG_GRENADE;
	case weapon_smokegrenade:    return WeaponIcons::SMOKEGRENADE;
	case weapon_molotov:         return WeaponIcons::MOLOTOV;
	case weapon_incgrenade:      return WeaponIcons::INCGRENADE;
	case weapon_decoy:           return WeaponIcons::DECOY;
	case weapon_c4:              return WeaponIcons::C4;
	default:                     return "?";
	}
}

static const char* GrenadeName(short idx) {
	switch (idx) {
	case weapon_flashbang:       return "Flash";
	case weapon_hegrenade:
	case weapon_frag_grenade:    return "HE";
	case weapon_smokegrenade:    return "Smoke";
	case weapon_molotov:         return "Molotov";
	case weapon_incgrenade:      return "Incendiary";
	case weapon_decoy:           return "Decoy";
	case weapon_c4:             return "C4";
	default:                     return "Grenade";
	}
}

static bool IsVisibleToLocal(const Player& player, const Player& local) {
	if (!local.localplayer)
		return player.visible;

	const int localIndex = static_cast<unsigned char>(local.index);
	if (localIndex < 0 || localIndex >= 64)
		return player.visible;

	const size_t word = static_cast<size_t>(localIndex >> 5);
	const uint32_t bit = 1u << (localIndex & 31);
	if (word >= player.spotted_by_mask.size())
		return player.visible;

	return (player.spotted_by_mask[word] & bit) != 0;
}

bool Cache::Refresh() {
	return Get().RefreshImpl();
}

Snapshot Cache::CopySnapshot() {
	std::lock_guard<std::mutex> lock(Get().mtx);
	return {
		Get().game,
		Get().bomb,
		Get().local,
		Get().globals,
		Get().players,
		Get().world_entities
	};
}

bool Cache::RefreshImpl() {
	auto p = Engine::GetProcess();
	auto client = Engine::GetClient();

	if (!p)
		return false;

	auto now = steady_clock::now();

	Game gameSnapshot;
	Bomb bombSnapshot;
	Globals globalsSnapshot;

	if (!gameSnapshot.Update())
		return false;

#ifdef _DEBUG
	if (now - last < (cfg::dev::cache_refresh_rate * 1ms))
		return true;
#else
	if (now - last < 5ms)
		return true;
#endif

	gameSnapshot.UpdateEntityList();
	if (!globalsSnapshot.Update())
		return false;
	MapManager::Get().Update(globalsSnapshot.map_name);
	bombSnapshot.Update();

	std::vector<Player> scan;
	scan.reserve(globalsSnapshot.max_clients);
	Player localPlayerSnapshot;
	bool hasLocalPlayer = false;
	for (int i = 0; i < globalsSnapshot.max_clients; i++) {
		auto player = Player(i, gameSnapshot.entity_list, gameSnapshot.list_entry);

		if (!player.Update())
			continue;

		if (player.localplayer)
		{
			localPlayerSnapshot = player;
			hasLocalPlayer = true;
		}

		scan.push_back(player);
	}

	std::vector<WorldEntity> worldScan;
	ScanWorldEntities(worldScan, gameSnapshot.entity_list, globalsSnapshot.max_clients);

	const Player* visibilityLocal = hasLocalPlayer ? &localPlayerSnapshot : (local.localplayer ? &local : nullptr);
	if (visibilityLocal) {
		for (auto& player : scan) {
			player.visible = IsVisibleToLocal(player, *visibilityLocal);
		}
	}

	{
		std::lock_guard<std::mutex> lock(mtx);
		game = std::move(gameSnapshot);
		bomb = std::move(bombSnapshot);
		globals = std::move(globalsSnapshot);
		if (hasLocalPlayer)
			this->local = std::move(localPlayerSnapshot);
		players = std::move(scan);
		world_entities = std::move(worldScan);

		duration = duration_cast<std::chrono::milliseconds>(last - now);
		last = now;
	}

	return true;
}

void Cache::ScanWorldEntities(std::vector<WorldEntity>& out, uintptr_t entityList, int maxClients) {
	auto p = Engine::GetProcess();
	if (!p) return;

	if (!cfg::esp::dropped_weapons && !cfg::esp::grenade_esp) {
		return;
	}

	out.clear();

	constexpr int MAX_ENTITY_SCAN = 512;

	for (int i = 0; i < MAX_ENTITY_SCAN; i++) {
		if (i < maxClients)
			continue;

		int chunkIndex = i >> 9;
		int slotIndex = i & 0x1FF;

		uintptr_t chunk = p->read<uintptr_t>(entityList + 0x10 + (uintptr_t)chunkIndex * 8);
		if (!chunk) continue;

		uintptr_t entity = p->read<uintptr_t>(chunk + (uintptr_t)(slotIndex + 1) * 0x70);
		if (!entity) continue;

		short itemIdx = p->read<short>(entity + offsets::pawn::m_AttributeManager
			+ offsets::pawn::m_Item + offsets::pawn::m_iItemDefinitionIndex);

		if (itemIdx <= 0) continue;

		bool isGrenade = IsGrenadeItem(itemIdx);
		bool isWeapon = !isGrenade && itemIdx < 100;

		if (!isGrenade && !isWeapon) continue;

		uintptr_t gameSceneNode = p->read<uintptr_t>(entity + offsets::pawn::m_pGameSceneNode);
		if (!gameSceneNode) continue;

		Vec3_t pos = p->read<Vec3_t>(gameSceneNode + offsets::bomb::m_vecAbsOrigin);
		if (pos.zero()) continue;

		WorldEntity we;
		we.pos = pos;
		we.item_index = itemIdx;

		if (isGrenade) {
			we.type = WorldEntity::Type::GrenadeProjectile;
			we.name = GrenadeName(itemIdx);
			we.icon = GrenadeIcon(itemIdx);

			if (itemIdx == weapon_smokegrenade) {
				int smokeTick = p->read<int>(entity + offsets::pawn::m_nSmokeEffectTickBegin);
				(void)smokeTick;
			}
		} else {
			we.type = WorldEntity::Type::DroppedWeapon;
			Weapon tempWeapon(entity, 0);
			tempWeapon.item_index = itemIdx;
			we.name = tempWeapon.ToString();
			we.icon = tempWeapon.ToIcon();
		}

		out.push_back(we);
	}
}
