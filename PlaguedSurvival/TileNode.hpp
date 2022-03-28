#pragma once
#include "Entity.hpp"

class TileNode : public Entity
{
public:
	explicit TileNode(
		const SceneLayers& scene_layers,
		const sf::Texture& texture,
		int hit_points = 2);

	unsigned GetCategory() const override;
	sf::FloatRect GetBoundingRect() const override;

private:
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	sf::Sprite m_sprite;
};

