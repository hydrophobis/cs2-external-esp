#pragma once
#include "../memory/memory.hpp"
#include "../classes/vector.hpp"
#include <map>
#include <string>
#include <mutex>



struct view_matrix_t {
	float* operator[ ](int index) {
		return matrix[index];
	}

	float matrix[4][4];
};

static Vector3 viewAngle;

class CBones {
public:
	std::map<std::string, Vector3> bonePositions;
};

class CC4 {
public:
	uintptr_t get_planted();
	uintptr_t get_node();
	Vector3 get_origin();
};

class CPlayer {
public:
	uintptr_t entity;
	int team;
	uintptr_t pCSPlayerPawn;
	uintptr_t gameSceneNode;
	uintptr_t boneArray;
	uintptr_t spottedState;
	int health;
	int armor;
	std::string name;
	Vector3 origin;
	Vector3 head;
	CBones bones;
	bool is_defusing;
	bool is_spotted;
	int32_t money;
	float flashAlpha;
	int handle;
	std::string weapon;
	void ReadBones();
	void ReadHead();
	Vector3 velocity;
};

class CGame
{
public:
	std::shared_ptr<pProcess> process;
	ProcessModule base_client;
	ProcessModule base_engine;
	RECT game_bounds;
	uintptr_t buildNumber;
	bool inGame;
	Vector3 localOrigin;
	Vector3 localVelocity;
	bool isC4Planted;
	int localTeam;
	CC4 c4;
	std::vector<CPlayer> players = {};
	CPlayer* localPlayerPtr;
	Vector3 localAimPunch;
	void init();
	void loop();
	void close();
	Vector3 world_to_screen(Vector3* v);
	CPlayer* get_nearest_player();
	uintptr_t localpCSPlayerPawn;

private:
	view_matrix_t view_matrix;
	Vector3 lastPunch = { 0, 0, 0 };
	uintptr_t entity_list;
	uintptr_t localPlayer;
	std::uint32_t localPlayerPawn;
	uintptr_t localList_entry2;
};

inline CGame g_game;

inline std::mutex reader_mutex;