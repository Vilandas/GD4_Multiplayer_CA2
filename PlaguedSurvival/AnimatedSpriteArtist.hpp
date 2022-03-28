#pragma once
#include <vector>

#include "AnimatedSprite.hpp"

class AnimatedSpriteArtist : public sf::Drawable
{
public:
	AnimatedSpriteArtist(std::vector<AnimatedSprite> sprite_states);
	AnimatedSpriteArtist(const std::vector<AnimationData>& data, const TextureHolder& textures);

	void UpdateCurrent(sf::Time dt);

	AnimatedSprite CurrentSpriteState() const;
	sf::FloatRect GetBoundingRect() const;
	sf::FloatRect GetLargestBounds() const;
	sf::FloatRect GetSmallestBounds() const;
	void ChangeState(int index);
	void Pause();
	void Play();
	void Flipped(bool flipped);

private:
	AnimatedSprite& CurrentSpriteState();
	void CalculateLargestAndSmallest();

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	std::vector<AnimatedSprite> m_sprite_states;
	int m_current_sprite_index;
	bool m_flipped;

	int m_largest_bounds_index;
	int m_smallest_bounds_index;
};