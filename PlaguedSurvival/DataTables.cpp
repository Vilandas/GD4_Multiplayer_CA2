#include "DataTables.hpp"

#include "ParticleType.hpp"
#include "PlatformerCharacterType.hpp"

std::vector<PlatformerCharacterData> InitializePlatformerCharacterData()
{
	std::vector<PlatformerCharacterData> data(static_cast<int>(PlatformerCharacterType::kPlatformerCount));
	data[static_cast<int>(PlatformerCharacterType::kBruno)].m_jump_force = 140;
	data[static_cast<int>(PlatformerCharacterType::kBruno)].m_acceleration = 1000;
	data[static_cast<int>(PlatformerCharacterType::kBruno)].m_max_velocity = sf::Vector2f(100, 200);
	data[static_cast<int>(PlatformerCharacterType::kBruno)].m_deceleration = 1000;
	data[static_cast<int>(PlatformerCharacterType::kBruno)].m_gravity = 200;
	data[static_cast<int>(PlatformerCharacterType::kBruno)].m_coyote_time = 0.5f;
	data[static_cast<int>(PlatformerCharacterType::kBruno)].m_health = 100;
	data[static_cast<int>(PlatformerCharacterType::kBruno)].m_animation_data = PlatformerAnimationData{
		{Textures::kBrunoIdle, 21, 35, 12, 1},
		{Textures::kBrunoRun, 23, 34, 8, 1},
		{Textures::kBrunoIdle, 19, 34, 9, 1}
	};

	return data;
}


std::vector<ParticleData> InitializeParticleData()
{
	std::vector<ParticleData> data(static_cast<int>(ParticleType::kParticleCount));

	data[static_cast<int>(ParticleType::kPropellant)].m_color = sf::Color(255, 255, 50);
	data[static_cast<int>(ParticleType::kPropellant)].m_lifetime = sf::seconds(0.6f);

	data[static_cast<int>(ParticleType::kSmoke)].m_color = sf::Color(50, 50, 50);
	data[static_cast<int>(ParticleType::kSmoke)].m_lifetime = sf::seconds(4.f);

	return data;
}

