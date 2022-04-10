#include "WorldInfo.hpp"

/**
 * Vilandas Morrissey - D00218436
 */

WorldInfo WorldInfo::m_instance;

WorldInfo::WorldInfo()
	: m_show_collision_bounds()
{
}

bool WorldInfo::ShowCollisionBounds() const
{
	return m_show_collision_bounds;
}

void WorldInfo::SetShowCollisionBounds(bool flag)
{
	m_show_collision_bounds = flag;
}
