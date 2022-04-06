#pragma once
#include <array>

#include "Dangerous.hpp"
#include "DangerTrigger.hpp"
#include "Entity.hpp"

class TileNode : public Entity, public Dangerous
{
public:
	explicit TileNode(
		const SceneLayers& scene_layers,
		const sf::Texture& texture,
		const sf::Texture& crack_texture,
		int hit_points = 2);

	unsigned GetCategory() const override;
	sf::FloatRect GetBoundingRect() const override;

	void AddBelowTile(TileNode* tile);
	void SetIsTop(bool is_new = false);
	void SetIsTop(const std::queue<TileNode*>& below_tiles);
	void SetActiveCollision();

	void SetLeftTile(TileNode* tile);
	void SetRightTile(TileNode* tile);

	void Trigger() override;

private:
	void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void OnDamage() override;

private:
	sf::Sprite m_sprite;
	sf::Sprite m_crack_sprite;
	bool m_is_top;
	bool m_active_collision;

	TileNode* m_left_tile;
	TileNode* m_right_tile;
	std::queue<TileNode*> m_below_tiles;
};

