#pragma once

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>

#include "DataTables.hpp"

/**
 * Vilandas Morrissey - D00218436
 */

namespace sf
{
	class Texture;
}

class AnimatedSprite
{
public:
	AnimatedSprite(const AnimationData& data, const sf::Texture& texture);

	void DrawCurrent(sf::RenderTarget& target, sf::RenderStates& states, bool flipped) const;
	void UpdateCurrent(sf::Time dt);

	sf::Sprite CurrentSprite() const;
	sf::FloatRect GetBoundingRect();
	void Pause();
	void Play();
	void Reset();

	void SetColor(sf::Color color);

private:
	void Advance();
	sf::Sprite& CurrentSprite();

private:
	std::vector<sf::Sprite> m_sprites;
	float m_time_since_last_frame;
	float m_time_per_frame;
	int m_current_index;
	bool m_loop;
	bool m_paused;
};
