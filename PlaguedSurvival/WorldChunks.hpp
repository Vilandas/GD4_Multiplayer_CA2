#pragma once
#include <set>
#include <unordered_map>
#include <vector>

#include "Layers.hpp"

class SceneNode;

class WorldChunks
{
public:
	WorldChunks(const WorldChunks&) = delete;

	static WorldChunks& Instance()
	{
		return m_instance;
	}

	void Clear();
	void AddToChunk(SceneNode* scene_node, Layers layer);
	void RemoveFromChunk(SceneNode* scene_node, Layers layer);

	void CheckCollision(SceneNode* scene_node, Layers layer, std::set<SceneNode*>& collisions);

private:
	WorldChunks();

	const int WORLD_CHUNKS = 52;
	const float WORLD_WIDTH = 3328;

private:
	static WorldChunks m_instance;

	float m_width_per_chunk;
	std::vector<std::unordered_map<Layers, std::vector<SceneNode*>>> m_chunks;
};
