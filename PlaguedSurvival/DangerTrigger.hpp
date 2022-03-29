#pragma once
#include <vector>
#include <SFML/System/Time.hpp>

#include "Dangerous.hpp"

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

private:
	DangerTrigger();

private:
	static DangerTrigger m_instance;

	std::vector<Dangerous*> m_dangers;
	float m_danger_time;
	float m_dangers_per_second;
	float m_danger_increment_per_second;
};