#include "WorldChunks.hpp"

#include <iostream>

#include "SceneNode.hpp"
#include "WorldInfo.hpp"

WorldChunks WorldChunks::m_instance;

WorldChunks::WorldChunks()
	: m_chunks(WorldInfo::WORLD_CHUNKS)
{
}

void WorldChunks::Clear()
{
	for (auto& chunk : m_chunks)
	{
		chunk.clear();
	}
}

void WorldChunks::AddToChunk(SceneNode* scene_node, Layers layer)
{
	const size_t index = static_cast<size_t>(std::floorf(scene_node->getPosition().x / WorldInfo::WIDTH_PER_CHUNK));
	m_chunks[index][layer].emplace_back(scene_node);
}

void WorldChunks::RemoveFromChunk(SceneNode* scene_node, Layers layer)
{
	const size_t index = static_cast<size_t>(std::floorf(scene_node->getPosition().x / WorldInfo::WIDTH_PER_CHUNK));

	auto& chunk = m_chunks[index][layer];
	chunk.erase(std::remove(chunk.begin(), chunk.end(), scene_node), chunk.end());
}

void WorldChunks::CheckCollision(SceneNode* scene_node, Layers layer, std::set<SceneNode*>& collisions)
{
	size_t index = static_cast<size_t>(std::floorf(scene_node->getPosition().x / WorldInfo::WIDTH_PER_CHUNK));
	size_t iterations = 3;

	if (index == 0)
	{
		iterations--;
	}
	else if (index >= m_chunks.size() - 2)
	{
		iterations--;
		index = m_chunks.size() - 2;
	}
	else
	{
		index--;
	}

	int counter = 0;

	for (size_t i = 0; i < iterations; i++)
	{
		for (const auto& node : m_chunks[index + i][layer])
		{
			counter++;
			scene_node->PredictCollisionWithNode(*node, collisions);
		}
	}

	//std::cout << "Collisions Checked: " << counter << std::endl;
}