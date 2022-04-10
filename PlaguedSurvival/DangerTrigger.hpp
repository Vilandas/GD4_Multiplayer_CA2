#pragma once
#include <vector>
#include <SFML/System/Time.hpp>

#include "Dangerous.hpp"

/**
 * Vilandas Morrissey - D00218436
 */

class DangerTrigger
{
public:
	DangerTrigger(const DangerTrigger&) = delete;

	static DangerTrigger& Instance()
	{
		return m_instance;
	}

	void Clear();
	void AddDangerObject(Dangerous* danger);
	void RemoveDangerObject(const Dangerous* danger);
	void Update(sf::Time dt);
	void Update(float dt_as_seconds);

private:
	DangerTrigger();

	const float DEFAULT_DPS = 0.8f;
	const float DEFAULT_IPS = 0.1f;

private:
	static DangerTrigger m_instance;

	std::vector<Dangerous*> m_dangers;
	float m_danger_time;
	float m_dangers_per_second;
	float m_danger_increment_per_second;
};