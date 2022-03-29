#include "PlatformerCharacter.hpp"

#include <array>
#include <SFML/Graphics/RenderTarget.hpp>

#include "Collision.hpp"
#include "DataTables.hpp"
#include "PlatformerAnimationState.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"

enum class CollisionLocation;

namespace
{
	const std::vector<PlatformerCharacterData> Table = InitializePlatformerCharacterData();
}


PlatformerCharacter::PlatformerCharacter(
	const SceneLayers& scene_layers,
	PlatformerCharacterType type,
	const TextureHolder& textures,
	const FontHolder& fonts,
	SoundPlayer& sounds,
	Camera& camera,
	bool is_camera_target)
	: Entity(
		scene_layers,
		Table[static_cast<int>(type)].m_health,
		Table[static_cast<int>(type)].m_acceleration,
		Table[static_cast<int>(type)].m_max_velocity,
		Table[static_cast<int>(type)].m_deceleration,
		Table[static_cast<int>(type)].m_gravity)
	, m_type(type)
	, m_camera(camera)
	, m_artist(Table[static_cast<int>(type)].m_animation_data.ToVector(), textures)
	, m_sounds(sounds)
	, m_coyote_time(Table[static_cast<int>(type)].m_coyote_time)
	, m_air_time(0)
	, m_jumping(false)
	, m_camera_move_constraint(false)
	, m_is_camera_target(is_camera_target)
{
}

unsigned PlatformerCharacter::GetCategory() const
{
	return Category::kPlayerCharacter;
}

sf::FloatRect PlatformerCharacter::GetBoundingRect() const
{
	return GetWorldTransform().transformRect(m_artist.GetSmallestBounds());
}

void PlatformerCharacter::Jump()
{
	if (!IsJumping() && m_air_time <= m_coyote_time)
	{
		m_jumping = true;
		const sf::Vector2f velocity = GetVelocity();

		SetVelocity(GetVelocity().x, -Table[static_cast<int>(m_type)].m_jump_force);
	}
}

void PlatformerCharacter::ResetJump()
{
	m_air_time = 0.f;
	m_jumping = false;
}

bool PlatformerCharacter::IsGrounded() const
{
	return m_air_time > 0;
}

bool PlatformerCharacter::IsJumping() const
{
	return m_jumping;
}
/// <summary>
/// Handle Collisions between the Player Character and any SceneNode with a collider
/// </summary>
void PlatformerCharacter::HandleCollisions()
{
	std::set<SceneNode*> collisions;

	PredictCollisionsWithScene(*GetSceneLayers()[static_cast<int>(Layers::kPlatforms)], collisions);

	for (SceneNode* node : collisions)
	{
		if (Category::kPlatform & node->GetCategory())
		{
			const CollisionLocation location = Collision::CollisionLocation(*this, *node);

			BlockingCollision(location);
		}
	}
}

void PlatformerCharacter::BlockingCollision(CollisionLocation location)
{
	const sf::Vector2f velocity = GetVelocity();

	switch (location)
	{
	case CollisionLocation::kLeft:
		if (velocity.x < 0)
		{
			SetVelocity(0, velocity.y);
		}
		break;

	case CollisionLocation::kRight:
		if (velocity.x > 0)
		{
			SetVelocity(0, velocity.y);
		}
		break;

	case CollisionLocation::kTop:
		if (velocity.y < 0)
		{
			SetVelocity(velocity.x, 0);
		}
		break;

	case CollisionLocation::kBottom:
		ResetJump();

		if (velocity.y > 0)
		{
			SetVelocity(velocity.x, 0);
		}
		break;

	case CollisionLocation::kNone:
		break;
	}
}

void PlatformerCharacter::ApplyGravity(sf::Time dt)
{
	if (GetVelocity().y > -300)
	{
		Entity::ApplyGravity(sf::seconds(dt.asSeconds() * 4));
	}
	else Entity::ApplyGravity(dt);
}

void PlatformerCharacter::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_artist, states);
}

void PlatformerCharacter::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	m_air_time += dt.asSeconds();

	Entity::UpdateCurrent(dt, commands);
	m_artist.UpdateCurrent(dt);

	UpdateAnimationState();
	UpdateCamera(dt);
}

void PlatformerCharacter::UpdateAnimationState()
{
	const sf::Vector2f velocity = GetVelocity();

	if (velocity.x != 0.f)
	{
		m_artist.ChangeState(static_cast<int>(PlatformerAnimationState::kRun));

		if (velocity.x > 0)
		{
			m_artist.Flipped(false);
		}
		else if (velocity.x < 0)
		{
			m_artist.Flipped(true);
		}
	}
	else
	{
		m_artist.ChangeState(static_cast<int>(PlatformerAnimationState::kIdle));
	}
}

void PlatformerCharacter::UpdateCamera(sf::Time dt)
{
	if (!m_is_camera_target) return;

	const sf::Vector2f distance = getPosition() - m_camera.GetCenter();

	if (m_camera_move_constraint || Utility::Length(distance) > 100)
	{
		m_camera_move_constraint = true;

		const sf::Vector2f new_position = distance * 0.02f;
		m_camera.SetCenter(m_camera.GetCenter() + new_position);

		if (Utility::Length(new_position) <= 0.2f)
		{
			m_camera_move_constraint = false;
		}
	}
}