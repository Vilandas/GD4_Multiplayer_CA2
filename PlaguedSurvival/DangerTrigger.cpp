#include "DangerTrigger.hpp"

#include "Utility.hpp"

DangerTrigger DangerTrigger::m_instance;

DangerTrigger::DangerTrigger()
	: m_dangers()
	, m_danger_time()
	, m_dangers_per_second(0.8f)
	, m_danger_increment_per_second(0.1f)
{
}

void DangerTrigger::Clear()
{
	m_dangers.clear();
}

void DangerTrigger::AddDangerObject(Dangerous* danger)
{
	m_dangers.emplace_back(danger);
}

void DangerTrigger::RemoveDangerObject(const Dangerous* danger)
{
	m_dangers.erase(std::find(m_dangers.begin(), m_dangers.end(), danger));
}

void DangerTrigger::Update(sf::Time dt)
{
	if (m_dangers.empty()) return;

	m_danger_time += dt.asSeconds();

	if (m_danger_time >= 1)
	{
		m_danger_time -= 1;

		int dps = static_cast<int>(m_dangers_per_second);

		if (dps > m_dangers.size())
		{
			dps = m_dangers.size();
		}

		for (int i = 0; i < dps; i++)
		{
			const int chosenIndex = Utility::RandomInt(m_dangers.size());
			m_dangers[chosenIndex]->Trigger();
		}
	}

	m_dangers_per_second += m_danger_increment_per_second * dt.asSeconds();
}
