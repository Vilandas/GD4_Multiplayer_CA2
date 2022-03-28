#include "AnimatedSpriteArtist.hpp"

#include <cassert>
#include <utility>

#include "ResourceHolder.hpp"

AnimatedSpriteArtist::AnimatedSpriteArtist(std::vector<AnimatedSprite> sprite_states)
	: m_sprite_states(std::move(sprite_states))
	, m_current_sprite_index(0)
	, m_flipped(false)
	, m_largest_bounds_index(0)
	, m_smallest_bounds_index(0)
{
	CalculateLargestAndSmallest();
}

AnimatedSpriteArtist::AnimatedSpriteArtist(const std::vector<AnimationData>& data, const TextureHolder& textures)
	: m_current_sprite_index(0)
	, m_flipped(false)
	, m_largest_bounds_index(0)
	, m_smallest_bounds_index(0)
{
	for (auto& i : data)
	{
		m_sprite_states.emplace_back(i, textures.Get(i.m_texture));
	}

	CalculateLargestAndSmallest();
}

void AnimatedSpriteArtist::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	CurrentSpriteState().DrawCurrent(target, states, m_flipped);
}

void AnimatedSpriteArtist::UpdateCurrent(sf::Time dt)
{
	CurrentSpriteState().UpdateCurrent(dt);
}

AnimatedSprite AnimatedSpriteArtist::CurrentSpriteState() const
{
	return m_sprite_states[m_current_sprite_index];
}

sf::FloatRect AnimatedSpriteArtist::GetBoundingRect() const
{
	return CurrentSpriteState().GetBoundingRect();
}

sf::FloatRect AnimatedSpriteArtist::GetLargestBounds() const
{
	AnimatedSprite sprite = m_sprite_states[m_largest_bounds_index];
	return sprite.GetBoundingRect();
}

sf::FloatRect AnimatedSpriteArtist::GetSmallestBounds() const
{
	AnimatedSprite sprite = m_sprite_states[m_smallest_bounds_index];
	return sprite.GetBoundingRect();
}

void AnimatedSpriteArtist::ChangeState(int index)
{
	if (index != m_current_sprite_index)
	{
		assert(index >= 0 && index < m_sprite_states.size());

		CurrentSpriteState().Reset();
		m_current_sprite_index = index;
	}
}

void AnimatedSpriteArtist::Pause()
{
	CurrentSpriteState().Pause();
}

void AnimatedSpriteArtist::Play()
{
	CurrentSpriteState().Play();
}

void AnimatedSpriteArtist::Flipped(bool flipped)
{
	m_flipped = flipped;
}

AnimatedSprite& AnimatedSpriteArtist::CurrentSpriteState()
{
	return m_sprite_states[m_current_sprite_index];
}

void AnimatedSpriteArtist::CalculateLargestAndSmallest()
{
	float largest_product = std::numeric_limits<float>::min();
	float smallest_product = std::numeric_limits<float>::max();

	for (int i = 0; i < m_sprite_states.size(); i++)
	{
		const sf::FloatRect bounds = m_sprite_states[i].GetBoundingRect();
		const float product = bounds.width * bounds.height;

		if (product < smallest_product)
		{
			smallest_product = product;
			m_smallest_bounds_index = i;
		}

		if (product > largest_product)
		{
			largest_product = product;
			m_largest_bounds_index = i;
		}
	}
}
