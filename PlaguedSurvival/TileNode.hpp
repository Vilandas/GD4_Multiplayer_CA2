#pragma once
#include "Dangerous.hpp"
#include "DangerTrigger.hpp"
#include "Entity.hpp"

class TileNode : public Entity, public Dangerous
{
public:
	explicit TileNode(
		const SceneLayers& scene_layers,
		const sf::Texture& texture,
		int hit_points = 2);

	unsigned GetCategory() const override;
	sf::FloatRect GetBoundingRect() const override;

	void AddBelowTile(TileNode* tile);
	void SetIsTop();
	void SetIsTop(const std::queue<TileNode*>& below_tiles);

	void Trigger() override;

private:
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	void OnDamage() override;

private:
	sf::Sprite m_sprite;
	bool m_is_top;

	std::queue<TileNode*> m_below_tiles;
};

