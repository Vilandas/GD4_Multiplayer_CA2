#include "SpriteNode.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

SpriteNode::SpriteNode(
	const SceneLayers& scene_layers,
	const sf::Texture& texture)
	: SceneNode(scene_layers)
	, m_sprite(texture)
{
}

SpriteNode::SpriteNode(
	const SceneLayers& scene_layers,
	const sf::Texture& texture,
	const sf::IntRect& textureRect)
	: SceneNode(scene_layers)
	, m_sprite(texture, textureRect)
{
}

SpriteNode::SpriteNode(
	const SceneLayers& scene_layers,
	const sf::Texture& texture,
	const sf::IntRect& textureRect,
	const float position_x,
	const float position_y)
	: SceneNode(scene_layers)
	, m_sprite(texture, textureRect)
{
	setPosition(position_x, position_y);
}

void SpriteNode::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_sprite, states);
}
