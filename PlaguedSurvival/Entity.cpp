#include "Entity.hpp"

#include "Utility.hpp"


/// <summary>
/// </summary>
/// <param name="scene_layers">Scene Layers</param>
/// <param name="hitpoints">Total hitpoints for the entity</param>
/// <param name="acceleration_speed">Rate of acceleration</param>
/// <param name="max_velocity">Entities max velocity</param>
/// <param name="deceleration">Deceleration rate</param>
/// <param name="gravity">Gravity</param>
Entity::Entity(
	const SceneLayers& scene_layers,
	int hitpoints,
	float acceleration_speed,
	sf::Vector2f max_velocity,
	float deceleration,
	float gravity)
	: SceneNode(scene_layers)
	, m_hitpoints(hitpoints)
	, m_acceleration_speed(acceleration_speed)
	, m_max_velocity(max_velocity)
	, m_deceleration(deceleration)
	, m_gravity(gravity)
	, m_delta_time_in_seconds(0)
	, m_is_marked_for_removal(false)
{
}

/// <summary>
/// Sets the entities Velocity
/// </summary>
/// <param name="velocity">Velocity to set to</param>
void Entity::SetVelocity(sf::Vector2f velocity)
{
	m_velocity = velocity;
}

/// <summary>
/// Sets the entities Gravity
/// </summary>
/// <param name="gravity"></param>
void Entity::SetGravity(float gravity)
{
	m_gravity = gravity;
}

/// <summary>
/// Sets the entities velocity
/// </summary>
/// <param name="vx">Other x value</param>
/// <param name="vy">Other y value</param>
void Entity::SetVelocity(float vx, float vy)
{
	m_velocity.x = vx;
	m_velocity.y = vy;
}

/// <summary>
/// Add velocity to current velocity
/// </summary>
/// <param name="velocity">Other Velocity</param>
void Entity::AddVelocity(sf::Vector2f velocity)
{
	m_velocity += velocity;
}


/// <summary>
/// Add velocity to current velocity
/// </summary>
/// <param name="vx">Other X velocity</param>
/// <param name="vy">Other y velocity</param>
void Entity::AddVelocity(float vx, float vy)
{
	m_velocity.x += vx;
	m_velocity.y += vy;
}

/// <summary>
/// Adds direction to the list of possible directions
/// </summary>
/// <param name="direction">Direction to add to list</param>
void Entity::AddDirection(sf::Vector2i direction)
{
	m_directions[direction] = 0;
	UpdateDirectionUnit();
}

/// <summary>
/// Removes a direction from the list of possible directions
/// </summary>
/// <param name="direction">Direction to remove from list</param>
void Entity::RemoveDirection(sf::Vector2i direction)
{
	m_directions.erase(direction);
	UpdateDirectionUnit();
}

/// <summary>
/// Returns the current entities velocity
/// </summary>
/// <returns>Velocity</returns>
sf::Vector2f Entity::GetVelocity() const
{
	return m_velocity;
}

/// <summary>
/// Get time since last frame update in seconds
/// </summary>
/// <returns>Delta time in seconds</returns>
float Entity::GetDeltaTimeInSeconds() const
{
	return m_delta_time_in_seconds;
}

int	Entity::GetIdentifier()
{
	return m_identifier;
}

void Entity::SetIdentifier(int identifier)
{
	m_identifier = identifier;
}

/// <summary>
/// Update
/// </summary>
/// <param name="dt">Delta time</param>
/// <param name="commands">Command queue</param>
void Entity::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	m_delta_time_in_seconds = dt.asSeconds();

	if (IsDestroyed())
	{
		m_is_marked_for_removal = true;
		return;
	}

	UpdateDirections(dt);
	Accelerate(dt);
	Decelerate(dt);

	if (m_gravity != 0.f)
	{
		ApplyGravity(dt);
	}

	ValidateVelocity();
	HandleCollisions();

	move(m_velocity * dt.asSeconds());
}

void Entity::UpdateDirections(sf::Time dt)
{
	std::queue<sf::Vector2i> remove_queue;

	for (auto it = m_directions.begin(); it != m_directions.end(); ++it)
	{
		it->second += dt.asSeconds();

		if (it->second > 0.02f)
		{
			remove_queue.emplace(it->first);
		}
	}

	while (!remove_queue.empty())
	{
		const sf::Vector2i to_remove = remove_queue.front();
		remove_queue.pop();
		RemoveDirection(to_remove);
	}
}

/// <summary>
/// Applies gravity to the current velocity
/// </summary>
/// <param name="dt">Delta time</param>
void Entity::ApplyGravity(sf::Time dt)
{
	m_velocity.y += m_gravity * dt.asSeconds();
}

/// <summary>
/// Accelerate the entity over time
/// </summary>
/// <param name="dt">Delta Time</param>
void Entity::Accelerate(sf::Time dt)
{
	m_velocity += m_direction_unit * m_acceleration_speed * dt.asSeconds();
}

/// <summary>
/// Decelerate the entity over time
/// </summary>
/// <param name="dt">Delta Time</param>
void Entity::Decelerate(sf::Time dt)
{
	const float deceleration = m_deceleration * dt.asSeconds();

	if (m_direction_unit.x == 0.f)
	{
		if (m_velocity.x > 0)
		{
			m_velocity.x = std::max(m_velocity.x - deceleration, 0.f);
		}
		else if (m_velocity.x < 0)
		{
			m_velocity.x = std::min(m_velocity.x + deceleration, 0.f);
		}
	}

	if (m_gravity == 0.f && m_direction_unit.y == 0.f)
	{
		if (m_velocity.y > 0)
		{
			m_velocity.y = std::max(m_velocity.y - deceleration, 0.f);
		}
		else if (m_velocity.y < 0)
		{
			m_velocity.y = std::min(m_velocity.y + deceleration, 0.f);
		}
	}
}

/// <summary>
/// Ensures the velocity does not exceed the entities max velocity
/// </summary>
void Entity::ValidateVelocity()
{
	if (m_velocity.x > m_max_velocity.x)
	{
		m_velocity.x = m_max_velocity.x;
	}
	else if (m_velocity.x < -m_max_velocity.x)
	{
		m_velocity.x = -m_max_velocity.x;
	}

	if (m_velocity.y > m_max_velocity.y)
	{
		m_velocity.y = m_max_velocity.y;
	}
	else if (m_velocity.y < -m_max_velocity.y)
	{
		m_velocity.y = -m_max_velocity.y;
	}
}

/// <summary>
/// Changes direction to the new direction
/// </summary>
void Entity::UpdateDirectionUnit()
{
	sf::Vector2i new_direction;

	for (const auto& pair : m_directions)
	{
		new_direction += pair.first;
	}

	m_direction_unit = Utility::UnitVector(new_direction);
}

void Entity::OnDamage()
{
	// Do Nothing
}

int Entity::GetHitPoints() const
{
	return m_hitpoints;
}

sf::Vector2f Entity::GetDirectionUnit() const
{
	return m_direction_unit;
}

sf::Vector2f Entity::GetMaxVelocity() const
{
	return m_max_velocity;
}

void Entity::Repair(unsigned int points)
{
	m_hitpoints += points;
}

void Entity::Damage(unsigned int points)
{
	m_hitpoints -= points;

	if (m_hitpoints < 0)
	{
		m_hitpoints = 0;
	}

	OnDamage();
}

void Entity::Destroy()
{
	m_hitpoints = 0;
}

bool Entity::IsDestroyed() const
{
	return m_hitpoints <= 0;
}
