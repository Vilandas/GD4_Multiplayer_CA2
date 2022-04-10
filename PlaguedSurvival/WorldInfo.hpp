#pragma once

/**
 * Vilandas Morrissey - D00218436
 */

class WorldInfo
{
public:
	WorldInfo(const WorldInfo&) = delete;

	static WorldInfo& Instance()
	{
		return m_instance;
	}

	static constexpr float TILE_SIZE = 64;
	static constexpr int X_TILE_COUNT = 52;
	static constexpr int Y_TILE_COUNt = 24;

	static constexpr float WORLD_WIDTH = X_TILE_COUNT * TILE_SIZE;
	static constexpr float WORLD_HEIGHT = Y_TILE_COUNt * TILE_SIZE;

	static constexpr int WORLD_CHUNKS = X_TILE_COUNT;
	static constexpr float WIDTH_PER_CHUNK = TILE_SIZE;		//= WORLD_WIDTH / WORLD_CHUNKS

	bool ShowCollisionBounds() const;
	void SetShowCollisionBounds(bool flag);

private:
	WorldInfo();

private:
	static WorldInfo m_instance;

	bool m_show_collision_bounds;
};
