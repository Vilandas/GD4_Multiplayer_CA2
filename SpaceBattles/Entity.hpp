#pragma once
#include <unordered_map>

#include "CommandQueue.hpp"
#include "SceneNode.hpp"
#include "Vector2iHash.hpp"

class Entity : public SceneNode
{
public:
	Entity(
		const SceneLayers& scene_layers,
		int hitpoints = 1,
		float acceleration_speed = 1,
		sf::Vector2f max_velocity = sf::Vector2f(1, 1),
		float deceleration = 1,
		float gravity = 1);

	void SetVelocity(sf::Vector2f velocity);
	void SetVelocity(float vx, float vy);
	void AddVelocity(sf::Vector2f velocity);
	void AddVelocity(float vx, float vy);
	void AddDirection(sf::Vector2i direction);
	void RemoveDirection(sf::Vector2i direction);
	sf::Vector2f GetVelocity() const override;
	float GetDeltaTimeInSeconds() const override;

	int	GetIdentifier();
	void SetIdentifier(int identifier);

	int GetHitPoints() const;
	sf::Vector2f GetDirectionUnit() const;
	sf::Vector2f GetMaxVelocity() const;
	void Repair(unsigned int points);
	void Damage(int points);
	void Destroy();
	void SetGravity(float gravity);
	bool IsDestroyed() const override;

protected:
	void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;
	void UpdateDirections(sf::Time dt);

private:
	void Accelerate(sf::Time dt);
	void Decelerate(sf::Time dt);
	void ApplyGravity(sf::Time dt);
	void ValidateVelocity();
	void UpdateDirectionUnit();

private:
	int m_hitpoints;
	float m_acceleration_speed;
	sf::Vector2f m_max_velocity;
	float m_deceleration;
	float m_gravity;
	sf::Vector2f m_velocity;
	std::unordered_map<sf::Vector2i, float, Vector2iHash> m_directions;
	sf::Vector2f m_direction_unit;
	float m_delta_time_in_seconds;

	bool m_is_marked_for_removal;
	int m_identifier;
};
