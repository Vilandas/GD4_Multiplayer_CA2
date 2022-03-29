#include "TileNode.hpp"

#include <utility>

TileNode::TileNode(const SceneLayers& scene_layers, const sf::Texture& texture, int hit_points)
	: Entity(scene_layers, hit_points)
	, m_sprite(texture)
	, m_below_tiles()
{
}

unsigned TileNode::GetCategory() const
{
	return static_cast<int>(Category::kPlatform);
}

sf::FloatRect TileNode::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_sprite.getGlobalBounds());
}

void TileNode::AddBelowTile(TileNode* tile)
{
	m_below_tiles.push(tile);
}

void TileNode::SetIsTop()
{
	if (m_is_top) return;

	m_is_top = true;
	DangerTrigger::Instance().AddDangerObject(this);
}

void TileNode::SetIsTop(const std::queue<TileNode*>& below_tiles)
{
	SetIsTop();

	m_below_tiles = below_tiles;
}

void TileNode::Trigger()
{
	Damage(1);
}

void TileNode::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}

void TileNode::OnDamage()
{
	if (IsDestroyed())
	{
		DangerTrigger::Instance().RemoveDangerObject(this);

		if (!m_below_tiles.empty())
		{
			TileNode* tile = m_below_tiles.front();
			m_below_tiles.pop();
			tile->SetIsTop(m_below_tiles);
		}
	}
}
