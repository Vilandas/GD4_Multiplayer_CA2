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
		Camera& camera,
		const TextureHolder& textures,
		const FontHolder& fonts,
		SoundPlayer& sounds);

	unsigned GetCategory() const override;
	sf::FloatRect GetBoundingRect() const override;
	int GetCollectedCoins() const;

	void Jump();
	void ResetJump();
	bool IsGrounded() const;
	bool IsJumping() const;

protected:
	void HandleCollisions() override;
	void BlockingCollision(CollisionLocation location);
	//void BouncyCollision(CollisionLocation location);
	//void VerticalMovementCollision(CollisionLocation location, TileNode* tile);

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
	bool m_jumping;
	bool m_camera_move_constraint;
	float m_coyote_time;
	float m_air_time;
};