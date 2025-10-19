#include "reader.hpp"
#include <thread>
#include <cmath>
#include <limits>
#include "../classes/auto_updater.hpp"
#include "../classes/config.hpp"

// CGame
void CGame::init() {
	std::cout << "[cs2] Waiting for cs2.exe..." << std::endl;

	process = std::make_shared<pProcess>();
	while (!process->AttachProcess("cs2.exe"))
		std::this_thread::sleep_for(std::chrono::seconds(1));

	std::cout << "[cs2] Attached to cs2.exe\n" << std::endl;

	do {
		base_client = process->GetModule("client.dll");
		base_engine = process->GetModule("engine2.dll");
		if (base_client.base == 0 || base_engine.base == 0) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			std::cout << "[cs2] Failed to find module client.dll/engine2.dll, waiting for the game to load it..." << std::endl;
		}
	} while (base_client.base == 0 || base_engine.base == 0);

	GetClientRect(process->hwnd_, &game_bounds);

	buildNumber = process->read<uintptr_t>(base_engine.base + updater::offsets::dwBuildNumber);
}

void CGame::close() {
	std::cout << "[cs2] Deattaching from process" << std::endl;
	process->Close();
}

CPlayer* CGame::get_nearest_player() {
	if (players.empty())
		return nullptr;

	CPlayer* nearest = nullptr;
#undef max
	float minScreenDist = std::numeric_limits<float>::max();

	float centerX = game_bounds.right * 0.5f;
	float centerY = game_bounds.bottom * 0.5f;

	for (auto& player : players) {
		// These checks are already performed in the main loop but are kept for safety
		if (player.pCSPlayerPawn == localpCSPlayerPawn || player.team == localTeam)
			continue;

		Vector3 screenPos = world_to_screen(&player.head);
		if (screenPos.z < 0.01f)
			continue;

		float dx = screenPos.x - centerX;
		float dy = screenPos.y - centerY;
		float screenDist = std::sqrt(dx * dx + dy * dy);

		if (screenDist < minScreenDist) {
			minScreenDist = screenDist;
			nearest = &player;
		}
	}

	return nearest;
}

Vector3 CC4::get_origin() {
	return { 0,0,0 };
}

Vector3 lastPunch = { 0,0,0 };
void CGame::loop() {
	std::lock_guard<std::mutex> lock(reader_mutex);

	if (updater::offsets::dwLocalPlayerController == 0x0)
		throw std::runtime_error("Offsets have not been correctly set, cannot proceed.");

	inGame = false;

	localPlayer = process->read<uintptr_t>(base_client.base + updater::offsets::dwLocalPlayerController);
	if (!localPlayer) return;

	localPlayerPawn = process->read<std::uint32_t>(localPlayer + updater::offsets::m_hPlayerPawn);
	if (!localPlayerPawn) return;

	entity_list = process->read<uintptr_t>(base_client.base + updater::offsets::dwEntityList);

	uintptr_t localList_entry2 = process->read<uintptr_t>(entity_list + 0x8 * ((localPlayerPawn & 0x7FFF) >> 9) + 16);
	localpCSPlayerPawn = process->read<uintptr_t>(localList_entry2 + 112 * (localPlayerPawn & 0x1FF));
	if (!localpCSPlayerPawn) return;

	view_matrix = process->read<view_matrix_t>(base_client.base + updater::offsets::dwViewMatrix);
	localTeam = process->read<int>(localPlayer + updater::offsets::m_iTeamNum);
	localOrigin = process->read<Vector3>(localpCSPlayerPawn + updater::offsets::m_vOldOrigin);

	inGame = true;
	int playerIndex = 0;
	std::vector<CPlayer> list;

	while (true) {
		playerIndex++;
		uintptr_t list_entry = process->read<uintptr_t>(entity_list + (8 * (playerIndex & 0x7FFF) >> 9) + 16);
		if (!list_entry) break;

		CPlayer player;
		player.entity = process->read<uintptr_t>(list_entry + 112 * (playerIndex & 0x1FF));
		if (!player.entity) continue;

		player.team = process->read<int>(player.entity + updater::offsets::m_iTeamNum);
		if (player.team == localTeam) continue;

		uint32_t playerPawn_h = process->read<std::uint32_t>(player.entity + updater::offsets::m_hPlayerPawn);
		uintptr_t list_entry2 = process->read<uintptr_t>(entity_list + 0x8 * ((playerPawn_h & 0x7FFF) >> 9) + 16);
		if (!list_entry2) continue;

		player.pCSPlayerPawn = process->read<uintptr_t>(list_entry2 + 112 * (playerPawn_h & 0x1FF));
		if (!player.pCSPlayerPawn) continue;

		player.health = process->read<int>(player.pCSPlayerPawn + updater::offsets::m_iHealth);
		if (player.health <= 0 || player.health > 100) continue;

		player.origin = process->read<Vector3>(player.pCSPlayerPawn + updater::offsets::m_vOldOrigin);
		if (player.origin.x == localOrigin.x && player.origin.y == localOrigin.y && player.origin.z == localOrigin.z)
			continue;

		// **--- MERGE CONFLICT RESOLVED ---**
		// This block is now fixed. The necessary lines for the aimbot have been
		// kept and uncommented, while the unused ESP code was removed.
		player.gameSceneNode = process->read<uintptr_t>(player.pCSPlayerPawn + updater::offsets::m_pGameSceneNode);
		player.boneArray = process->read<uintptr_t>(player.gameSceneNode + 0x210); // m_modelState + 128
		player.head = { player.origin.x, player.origin.y, player.origin.z + 73.f }; // For nearest player check

		list.push_back(player);
	}

	players.clear();
	players.assign(list.begin(), list.end());

	// --- Aimbot and Recoil Control Logic ---

	viewAngle = process->read<Vector3>(base_client.base + updater::offsets::dwViewAngles);
	localAimPunch = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? process->read<Vector3>(localpCSPlayerPawn + 5860) : Vector3{ 0, 0, 0 });

	CPlayer* target = get_nearest_player();
	if (target) {
		Vector3 localEye = localOrigin;
		localEye.z += (GetAsyncKeyState(VK_LCONTROL) & 0x8000 ? 54.f : 73.f);

		// --- Recoil Control ---
		Vector3 punchDelta = localAimPunch - lastPunch;
		Vector3 correction = { punchDelta.x * 2.f, punchDelta.y * 2.f, 0 };
		lastPunch = localAimPunch;

		Vector3 aimTo = viewAngle;

		// --- Aimbot ---
		if (GetAsyncKeyState(VK_LMENU) & 0x8000) {
			uintptr_t targetBoneAddr = target->boneArray + 6 * 32; // Head bone
			Vector3 targetBonePos = g_game.process->read<Vector3>(targetBoneAddr);

			target->velocity = { 0,0,0 };
			float leadDamper = 0.67f;

			aimTo = CalculateAngle(
				localEye,
				{
					targetBonePos.x + target->velocity.x * leadDamper,
					targetBonePos.y + target->velocity.y * leadDamper,
					targetBonePos.z + 11.f + target->velocity.z * leadDamper
				}
			);
		}
		aimTo = aimTo - correction;

		// Smoothing
		Vector3 delta = aimTo - viewAngle;
		float deltaLength = sqrt(delta.x * delta.x + delta.y * delta.y);
		if (deltaLength > 0.0f) {
			delta.x /= deltaLength;
			delta.y /= deltaLength;
		}

		float smoothFactor = 0.4f;
		Vector3 newAngle = viewAngle + delta * smoothFactor;

		// Manually clamp and normalize angles
		if (newAngle.x > 89.0f) newAngle.x = 89.0f;
		if (newAngle.x < -89.0f) newAngle.x = -89.0f;
		while (newAngle.y > 180.f) newAngle.y -= 360.f;
		while (newAngle.y < -180.f) newAngle.y += 360.f;
		newAngle.z = 0.0f;

		process->write<Vector3>(base_client.base + updater::offsets::dwViewAngles, newAngle);
	}
}

Vector3 CGame::world_to_screen(Vector3* v) {
	float _x = view_matrix[0][0] * v->x + view_matrix[0][1] * v->y + view_matrix[0][2] * v->z + view_matrix[0][3];
	float _y = view_matrix[1][0] * v->x + view_matrix[1][1] * v->y + view_matrix[1][2] * v->z + view_matrix[1][3];

	float w = view_matrix[3][0] * v->x + view_matrix[3][1] * v->y + view_matrix[3][2] * v->z + view_matrix[3][3];

	if (w < 0.01f) {
		return { 0, 0, 0 };
	}

	float inv_w = 1.f / w;
	_x *= inv_w;
	_y *= inv_w;

	float x = game_bounds.right * .5f;
	float y = game_bounds.bottom * .5f;

	x += 0.5f * _x * game_bounds.right + 0.5f;
	y -= 0.5f * _y * game_bounds.bottom + 0.5f;

	return { x, y, w };
}