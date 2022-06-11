#pragma once

#include <cinttypes>

/** Represents a player character transform in DS1R, as a global 3D translation
	and a yaw angle in degrees.
*/
struct ds_pos
{
	float x;
	float y;
	float z;
	float angle;

	ds_pos()
		: x(0.0f)
		, y(0.0f)
		, z(0.0f)
		, angle(0.0f)
	{
	}

	ds_pos(float in_x, float in_y, float in_z, float in_angle)
		: x(in_x)
		, y(in_y)
		, z(in_z)
		, angle(in_angle)
	{
	}
};
static_assert(sizeof(ds_pos) == sizeof(float) * 4, "unexpected ds_pos size");

inline bool operator<(const ds_pos& lhs, const ds_pos& rhs)
{
	if (lhs.x == rhs.x)
	{
		if (lhs.y == rhs.y)
		{
			if (lhs.z == rhs.z)
			{
				return lhs.angle < rhs.angle;
			}
			return lhs.z < rhs.z;
		}
		return lhs.y < rhs.y;
	}
	return lhs.x < rhs.x;
}
