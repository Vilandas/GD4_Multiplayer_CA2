#pragma once
#include <vector>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Time.hpp>

#include "ResourceIdentifiers.hpp"

/**
 * Vilandas Morrissey - D00218436
 */

struct AnimationData
{
	Textures m_texture;
	size_t m_frame_width;
	size_t m_frame_height;
	size_t m_frames;
	float m_time_per_frame;
};

struct PlatformerAnimationData
{
	AnimationData m_idle;
	AnimationData m_run;
	AnimationData m_jump;

	std::vector<AnimationData> ToVector() const
	{
		return { m_idle, m_run, m_jump };
	}
};

struct PlatformerCharacterData
{
	float m_jump_force;
	float m_acceleration;
	sf::Vector2f m_max_velocity;
	float m_deceleration;
	float m_gravity;
	float m_coyote_time;
	int m_health;
	PlatformerAnimationData m_animation_data;
};

struct ParticleData
{
	sf::Color m_color;
	sf::Time m_lifetime;
};

std::vector<PlatformerCharacterData> InitializePlatformerCharacterData();
std::vector<ParticleData> InitializeParticleData();