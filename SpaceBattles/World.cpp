#include "World.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include <limits>

#include "ParticleNode.hpp"
#include "ParticleType.hpp"
#include "PostEffect.hpp"
#include "SoundNode.hpp"
#include "Utility.hpp"

World::World(sf::RenderWindow& render_window, TextureHolder& textures, FontHolder& fonts, SoundPlayer& sounds, Camera& camera, bool networked)
	: m_window(render_window)
	, m_textures(textures)
	, m_fonts(fonts)
	, m_sounds(sounds)
	, m_camera(camera)
	, m_scene_layers()
	, m_scenegraph(m_scene_layers)
	, m_world_bounds(0.f, 0.f, 768.f, 432.f)
	, m_spawn_position(20, 100)
	, m_networked_world(networked)
	, m_network_node(nullptr)
	, m_finish_sprite(nullptr)
{
	m_scene_texture.create(m_window.getSize().x, m_window.getSize().y);

	m_camera.SetCenter(m_spawn_position);
	m_camera.SetSize(384 * 1.5f, 216 * 1.5f);
	m_camera.SetBoundsConstraint(m_world_bounds);

	BuildScene();
}

void World::Update(sf::Time dt)
{
	//Forward commands to the scenegraph until the command queue is empty
	while (!m_command_queue.IsEmpty())
	{
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
	}

	//Remove all destroyed entities
	//RemoveWrecks() only destroys the entities, not the pointers in m_player
	const auto first_to_remove = std::remove_if(
		m_player_characters.begin(),
		m_player_characters.end(),
		std::mem_fn(&PlayerObject::IsMarkedForRemoval));

	m_player_characters.erase(first_to_remove, m_player_characters.end());
	m_scenegraph.RemoveWrecks();

	//Apply movement
	m_scenegraph.Update(dt, m_command_queue);

	UpdateSounds();
}

void World::Draw()
{
	if (PostEffect::IsSupported())
	{
		m_scene_texture.clear();
		m_scene_texture.setView(m_camera.GetView());
		m_scene_texture.draw(m_scenegraph);
		m_scene_texture.display();
		m_bloom_effect.Apply(m_scene_texture, m_window);
	}
	else
	{
		m_window.setView(m_camera.GetView());
		m_window.draw(m_scenegraph);
	}
}

CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

void World::SetWorldHeight(float height)
{
	m_world_bounds.height = height;
}

bool World::HasAlivePlayer() const
{
	return true;
}

bool World::HasPlayerAchievedVictory() const
{
	return false;
}

sf::IntRect World::GetBackgroundRect(sf::Texture& texture) const
{
	texture.setRepeated(true);
	return {
		0,
		0,
		static_cast<int>(m_world_bounds.width),
		static_cast<int>(texture.getSize().y)
	};
}

PlayerObject* World::GetPlayer(int identifier) const
{
	for (PlayerObject* a : m_player_characters)
	{
		if (a->GetIdentifier() == identifier)
		{
			return a;
		}
	}
	return nullptr;
}

void World::RemovePlayer(int identifier)
{
	PlayerObject* player = GetPlayer(identifier);
	if (player)
	{
		player->Destroy();
		m_player_characters.erase(std::find(m_player_characters.begin(), m_player_characters.end(), player));
	}
}

PlayerObject* World::AddPlayer(int identifier)
{
	std::unique_ptr<PlayerObject> player(
		new PlayerObject(
			m_scene_layers,
			PlatformerCharacterType::kBruno,
			m_camera,
			m_textures,
			m_fonts,
			m_sounds));

	player->setPosition(m_camera.GetCenter());
	player->SetIdentifier(identifier);

	m_player_characters.emplace_back(player.get());
	m_scene_layers[static_cast<int>(Layers::kPlayers)]->AttachChild(std::move(player));
	return m_player_characters.back();
}

bool World::PollGameAction(GameActions::Action& out)
{
	return m_network_node->PollGameAction(out);
}

void World::BuildScene()
{
	//Initialize the different layers
	for (std::size_t i = 0; i < static_cast<int>(Layers::kLayerCount); ++i)
	{
		Category::Type category = i == static_cast<int>(Layers::kAir)
			? Category::Type::kScene
			: Category::Type::kNone;

		SceneNode::Ptr layer(new SceneNode(m_scene_layers, category));
		m_scene_layers[i] = layer.get();
		m_scenegraph.AttachChild(std::move(layer));
	}

	//Prepare the background
	sf::Texture& jungle = m_textures.Get(Textures::kJungle);

	//Add the background sprite to our scene
	std::unique_ptr<SpriteNode> jungle_sprite(new SpriteNode(m_scene_layers, jungle, GetBackgroundRect(jungle)));
	m_scene_layers[static_cast<int>(Layers::kBackground)]->AttachChild(std::move(jungle_sprite));

	if (m_networked_world)
	{
		std::unique_ptr<NetworkNode> network_node(new NetworkNode(m_scene_layers));
		m_network_node = network_node.get();
		m_scenegraph.AttachChild(std::move(network_node));
	}
}

void World::UpdateSounds()
{
	//sf::Vector2f listener_position;

	//// 0 players (multiplayer mode, until server is connected) -> view center
	//if (m_player_aircraft.empty())
	//{
	//	listener_position = m_camera.getCenter();
	//}

	//// 1 or more players -> mean position between all aircrafts
	//else
	//{
	//	for (Aircraft* aircraft : m_player_aircraft)
	//	{
	//		listener_position += aircraft->GetWorldPosition();
	//	}

	//	listener_position /= static_cast<float>(m_player_aircraft.size());
	//}

	//// Set listener's position
	//m_sounds.SetListenerPosition(listener_position);

	//// Remove unused sounds
	//m_sounds.RemoveStoppedSounds();
}