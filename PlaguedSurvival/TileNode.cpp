#include "TileNode.hpp"

TileNode::TileNode(const SceneLayers& scene_layers, const sf::Texture& texture, int hit_points)
	: Entity(scene_layers, hit_points)
	, m_sprite(texture)
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

void TileNode::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}
