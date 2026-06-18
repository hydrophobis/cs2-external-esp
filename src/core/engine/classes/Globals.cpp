#include "Globals.hpp"

#include "core/engine/Engine.hpp"
#include "core/offsets/Dumper.hpp"
#include <cstring>

bool Globals::Update() {
	auto p = Engine::GetProcess();
	auto client = Engine::GetClient();

	if (!p)
		return false;

	this->address = p->read<uintptr_t>(client.base + offsets::globalVars);

	if (!this->address)
		return false;

	this->current_time = p->read<long>(this->address + offsets::global::currentTime);

	this->max_clients = p->read<int>(this->address + offsets::global::maxClients);
	this->in_match = this->max_clients > 1;

	auto map_name_addr = p->read<DWORD64>(this->address + offsets::global::currentMapName);

	std::memset(this->map_name, 0, sizeof(this->map_name));
	if (!p->read_raw(map_name_addr, this->map_name, sizeof(this->map_name)))
		return false;
	this->map_name[sizeof(this->map_name) - 1] = '\0';

	return true;
}

