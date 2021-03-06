#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SpriteNode.hpp"
#include "Layers.hpp"
#include "NetworkNode.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include <array>

#include "BloomEffect.hpp"
#include "CommandQueue.hpp"
#include "NetworkOptimisations.hpp"
#include "SoundPlayer.hpp"

#include "NetworkProtocol.hpp"
#include "PlatformerCharacter.hpp"
#include "WorldInfo.hpp"

/**
 * Vilandas Morrissey - D00218436
 */

namespace sf
{
	class RenderTarget;
}

typedef PlatformerCharacter PlayerObject;

class World : private sf::NonCopyable
{
public:
	explicit World(
		sf::RenderWindow& render_window,
		TextureHolder& textures,
		FontHolder& fonts,
		SoundPlayer& sounds,
		Camera& camera,
		bool networked = false);

	void Update(sf::Time dt);
	void Draw();

	CommandQueue& GetCommandQueue();
	void SetWorldHeight(float height);

	bool HasPlayerAchievedVictory() const;
	sf::IntRect GetBackgroundRect(sf::Texture& texture) const;

	PlayerObject* GetAlivePlayer() const;
	PlayerObject* GetPlayer(opt::PlayerIdentifier identifier) const;
	PlayerObject* AddPlayer(opt::PlayerIdentifier identifier, const std::string& name, bool is_camera_target);
	void RemovePlayer(opt::PlayerIdentifier identifier);
	bool PollGameAction(GameActions::Action& out);

private:
	void BuildScene();
	void KillPlayersNotInWorld();

private:
	sf::RenderWindow& m_window;
	sf::RenderTexture m_scene_texture;
	TextureHolder& m_textures;
	FontHolder& m_fonts;
	SoundPlayer& m_sounds;
	Camera& m_camera;
	SceneNode::SceneLayers m_scene_layers;
	SceneNode m_scenegraph;
	CommandQueue m_command_queue;

	sf::FloatRect m_world_bounds;
	std::vector<PlayerObject*> m_player_characters;
	opt::PlayerCount m_alive_players;

	BloomEffect m_bloom_effect;
	bool m_networked_world;
	NetworkNode* m_network_node;
	SpriteNode* m_finish_sprite;

	float m_elapsed_time;
};