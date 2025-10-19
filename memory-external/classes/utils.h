#pragma once
#include <windows.h>
#include <string>

#include "config.hpp"
#include "vector.hpp"

Vector3 CalculateAngle(Vector3 from, Vector3 to);

class Utils {
private:
public:
	void update_console_title();
	bool is_in_bounds(const Vector3& pos, int width, int height);
};

inline Utils utils;