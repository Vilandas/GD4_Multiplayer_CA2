#include "AnimatedSprite.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include "Utility.hpp"

AnimatedSprite::AnimatedSprite(const AnimationData& data, const sf::Texture& texture)
	: m_time_since_last_frame(0)
	, m_time_per_frame(data.m_time_per_frame)
	, m_current_index(0)
	, m_loop(true)
	, m_paused(false)
{
	for (int i = 0; i < data.m_frames; i++)
	{
		sf::IntRect rect(i * data.m_frame_width, 0, data.m_frame_width, data.m_frame_height);
		sf::Sprite sprite(texture, rect);

		Utility::CentreOrigin(sprite);
		m_sprites.emplace_back(sprite);
	}
}

void AnimatedSprite::DrawCurrent(sf::RenderTarget& target, sf::RenderStates& states, bool flipped) const
{
	sf::Sprite sprite = CurrentSprite();

	if (flipped)
	{
		sf::Vector2f scale = sprite.getScale();
		sprite.setScale(-scale.x, scale.y);
	}

	target.draw(sprite, states);
}

void AnimatedSprite::UpdateCurrent(sf::Time dt)
{
	if (!m_paused)
	{
		m_time_since_last_frame += 0.1f;
		m_time_since_last_frame += dt.asSeconds();

		if (m_time_since_last_frame > m_time_per_frame)
		{
			Advance();
			m_time_since_last_frame -= m_time_per_frame;
		}
	}
}

sf::Sprite AnimatedSprite::CurrentSprite() const
{
	return m_sprites[m_current_index];
}

sf::FloatRect AnimatedSprite::GetBoundingRect()
{
	return CurrentSprite().getGlobalBounds();
}

void AnimatedSprite::Pause()
{
	m_paused = true;
}

void AnimatedSprite::Play()
{
	m_paused = false;
}

void AnimatedSprite::Reset()
{
	m_current_index = 0;
	m_time_since_last_frame = 0;
}

void AnimatedSprite::Advance()
{
	m_current_index++;

	if (m_current_index >= m_sprites.size())
	{
		m_current_index = 0;
	}
}

sf::Sprite& AnimatedSprite::CurrentSprite()
{
	return m_sprites[m_current_index];
}
