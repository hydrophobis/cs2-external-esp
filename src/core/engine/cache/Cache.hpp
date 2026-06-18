#pragma once
#include "core/engine/classes/Game.hpp"
#include "core/engine/classes/Bomb.hpp"
#include "core/engine/classes/Player.hpp"
#include "core/engine/classes/Globals.hpp"

using namespace std::chrono;

struct WorldEntity {
	enum class Type {
		DroppedWeapon,
		GrenadeProjectile
	};

	Type type = Type::DroppedWeapon;
	Vec3_t pos;
	short item_index = -1;
	const char* name = "";
	const char* icon = "";
	float timer = 0.f;
	bool is_planted = false;
};

struct Snapshot {
	Game game;
	Bomb bomb;
	Player local;
	Globals globals;
	std::vector<Player> players;
	std::vector<WorldEntity> world_entities;
};

class Cache {
public:
	Game game;
	Bomb bomb;
	Player local;
	Globals globals;
	std::vector<Player> players;
	std::vector<WorldEntity> world_entities;
public:
	static Cache& Get()
	{
		static Cache instance{};
		return instance;
	}

	static Snapshot CopySnapshot();

	static bool Refresh();
private:
	std::mutex mtx;
	milliseconds duration{1};
	steady_clock::time_point last{};
private:
	bool RefreshImpl();
	void ScanWorldEntities(std::vector<WorldEntity>& out, uintptr_t entityList, int maxClients);
};
