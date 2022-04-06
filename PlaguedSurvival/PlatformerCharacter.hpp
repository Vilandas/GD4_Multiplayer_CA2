#pragma once
#include <SFML/Graphics/Sprite.hpp>

#include "AnimatedSpriteArtist.hpp"
#include "Camera.hpp"
#include "CollisionLocation.hpp"
#include "Entity.hpp"
#include "PlatformerCharacterType.hpp"
#include "ResourceIdentifiers.hpp"
#include "TextNode.hpp"
#include "SoundPlayer.hpp"

class PlatformerCharacter : public Entity
{
public:
	PlatformerCharacter(
		const SceneLayers& scene_layers,
		PlatformerCharacterType type,
		const TextureHolder& textures,
		const FontHolder& fonts,
		SoundPlayer& sounds,
		Camera& camera,
		bool is_camera_target = true);

	unsigned GetCategory() const override;
	sf::FloatRect GetBoundingRect() const override;

	void Jump();
	void ResetJump();
	bool IsGrounded() const;
	bool IsJumping() const;
	bool IsAlive() const;

	void SetName(const std::string& name) const;
	void SetColor(sf::Color color);
	void Kill();
	void Destroy() override;

	bool IsDestroyed() const override;

protected:
	void HandleCollisions() override;
	void BlockingCollision(CollisionLocation location);

	void ApplyGravity(sf::Time dt) override;

private:
	void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const override;
	void UpdateCurrent(sf::Time dt, CommandQueue& commands) override;

	void UpdateAnimationState();
	void UpdateCamera(sf::Time dt);

private:
	PlatformerCharacterType m_type;
	Camera& m_camera;
	AnimatedSpriteArtist m_artist;
	SoundPlayer& m_sounds;
	TextNode* m_name_display;
	float m_coyote_time;
	float m_air_time;
	bool m_jumping;
	bool m_camera_move_constraint;
	bool m_is_camera_target;
	bool m_destroyed;
};
