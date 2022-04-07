#include "TileNode.hpp"

#include <utility>

TileNode::TileNode(const SceneLayers& scene_layers, const sf::Texture& texture, const sf::Texture& crack_texture, int hit_points)
	: Entity(scene_layers, hit_points)
	, m_sprite(texture)
	, m_crack_sprite(crack_texture)
	, m_is_top()
	, m_active_collision()
	, m_left_tile(nullptr)
	, m_right_tile(nullptr)
	, m_below_tiles()
{
}

unsigned TileNode::GetCategory() const
{
	return m_active_collision
	? static_cast<unsigned int>(Category::kActivePlatform)
	: static_cast<unsigned int>(Category::kPlatform);
}

sf::FloatRect TileNode::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

void TileNode::AddBelowTile(TileNode* tile)
{
	m_below_tiles.push(tile);
}

void TileNode::SetIsTop(bool is_new)
{
	if (m_is_top) return;

	m_is_top = true;
	DangerTrigger::Instance().AddDangerObject(this);

	if (is_new)
	{
		m_active_collision = true;
		WorldChunks::Instance().AddToChunk(this, Layers::kActivePlatforms);
	}
	else
	{
		SetActiveCollision();
	}
}

void TileNode::SetIsTop(const std::queue<TileNode*>& below_tiles)
{
	SetIsTop();

	m_below_tiles = below_tiles;
}

void TileNode::SetActiveCollision()
{
	if (m_active_collision) return;

	m_active_collision = true;
	Ptr tile = GetSceneLayers()[static_cast<int>(Layers::kPlatforms)]->DetachChild(*this);
	GetSceneLayers()[static_cast<int>(Layers::kActivePlatforms)]->AttachChild(std::move(tile));
	WorldChunks::Instance().AddToChunk(this, Layers::kActivePlatforms);
}

void TileNode::SetLeftTile(TileNode* tile)
{
	m_left_tile = tile;
}

void TileNode::SetRightTile(TileNode* tile)
{
	m_right_tile = tile;
}

void TileNode::Trigger()
{
	Damage(1);
}

void TileNode::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);

	if (GetHitPoints() == 1)
	{
		target.draw(m_crack_sprite, states);
	}
}

void TileNode::OnDamage()
{
	if (IsDestroyed())
	{
		WorldChunks::Instance().RemoveFromChunk(this, Layers::kActivePlatforms);
		DangerTrigger::Instance().RemoveDangerObject(this);

		if (!m_below_tiles.empty())
		{
			TileNode* tile = m_below_tiles.front();
			m_below_tiles.pop();
			tile->SetIsTop(m_below_tiles);
		}

		if (m_left_tile != nullptr)
		{
			m_left_tile->SetActiveCollision();
			m_left_tile->SetRightTile(nullptr);
		}
		if (m_right_tile != nullptr)
		{
			m_right_tile->SetActiveCollision();
			m_right_tile->SetLeftTile(nullptr);
		}
	}
}
