#include "DataTables.hpp"

#include "ParticleType.hpp"
#include "PlatformerCharacterType.hpp"

std::vector<PlatformerCharacterData> InitializePlatformerCharacterData()
{
	std::vector<PlatformerCharacterData> data(static_cast<int>(PlatformerCharacterType::kPlatformerCount));
	data[static_cast<int>(PlatformerCharacterType::kDoc)].m_jump_force = 400;
	data[static_cast<int>(PlatformerCharacterType::kDoc)].m_acceleration = 1000;
	data[static_cast<int>(PlatformerCharacterType::kDoc)].m_max_velocity = sf::Vector2f(300, 800);
	data[static_cast<int>(PlatformerCharacterType::kDoc)].m_deceleration = 4000;
	data[static_cast<int>(PlatformerCharacterType::kDoc)].m_gravity = 200;
	data[static_cast<int>(PlatformerCharacterType::kDoc)].m_coyote_time = 0.25f;
	data[static_cast<int>(PlatformerCharacterType::kDoc)].m_health = 100;
	data[static_cast<int>(PlatformerCharacterType::kDoc)].m_animation_data = PlatformerAnimationData{
		{Textures::kDocIdle, 190, 256, 10, 1.25f},
		{Textures::kDocRun, 256, 256, 4, 1},
		{Textures::kDocIdle, 190, 256, 10, 1}
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

