#include "World.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include <SFML/Graphics/RectangleShape.hpp>

#include "ParticleNode.hpp"
#include "ParticleType.hpp"
#include "PlayerColors.hpp"
#include "PostEffect.hpp"
#include "SoundNode.hpp"
#include "TileNode.hpp"
#include "Utility.hpp"

World::World(sf::RenderWindow& render_window, TextureHolder& textures, FontHolder& fonts, SoundPlayer& sounds, Camera& camera, bool networked)
	: m_window(render_window)
	, m_textures(textures)
	, m_fonts(fonts)
	, m_sounds(sounds)
	, m_camera(camera)
	, m_scene_layers()
	, m_scenegraph(m_scene_layers)
	, m_world_bounds(0.f, 0.f, WorldInfo::WORLD_WIDTH, WorldInfo::WORLD_HEIGHT)
	, m_alive_players(0)
	, m_networked_world(networked)
	, m_network_node(nullptr)
	, m_finish_sprite(nullptr)
	, m_elapsed_time()
{
	m_scene_texture.create(m_window.getSize().x, m_window.getSize().y);

	//m_camera.SetSize(384 * 1.5f, 216 * 1.5f);
	//m_camera.SetSize(640, 360);
	m_camera.SetBoundsConstraint(m_world_bounds);

	DangerTrigger::Instance().Clear();
	WorldChunks::Instance().Clear();
	BuildScene();
}

void World::Update(sf::Time dt)
{
	m_elapsed_time += dt.asSeconds();

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

	if (!m_networked_world)
	{
		KillPlayersNotInWorld();
	}
}

void World::Draw()
{
	if (PostEffect::IsSupported())
	{
		m_scene_texture.clear(sf::Color(150, 150, 150, 255));
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

bool World::HasPlayerAchievedVictory() const
{
	if (m_networked_world)
	{
		return m_alive_players <= 1;
	}

	return m_alive_players == 0;
}

sf::IntRect World::GetBackgroundRect(sf::Texture& texture) const
{
	texture.setRepeated(true);
	return {
		0,
		0,
		static_cast<int>(m_world_bounds.width),
		static_cast<int>(m_world_bounds.height)
	};
}

PlayerObject* World::GetAlivePlayer() const
{
	if (m_alive_players > 0)
	{
		for (auto& player : m_player_characters)
		{
			if (player->IsAlive())
			{
				return player;
			}
		}
	}

	return nullptr;
}

PlayerObject* World::GetPlayer(opt::PlayerIdentifier identifier) const
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

PlayerObject* World::AddPlayer(opt::PlayerIdentifier identifier, const std::string& name, bool is_camera_target)
{
	m_alive_players++;

	std::unique_ptr<PlayerObject> player(
		new PlayerObject(
			m_scene_layers,
			PlatformerCharacterType::kDoc,
			m_textures,
			m_fonts,
			m_sounds,
			m_camera,
			is_camera_target
		));

	player->setScale(0.5f, 0.5f);
	player->setPosition(200 + (200 * identifier), 64);
	player->SetIdentifier(identifier);
	player->SetName(name);
	player->SetColor(ExtraColors::GetColor(static_cast<PlayerColors>(identifier - 1)));

	m_player_characters.emplace_back(player.get());

	Layers layer = is_camera_target
		? Layers::kLocalPlayer
		: Layers::kPlayers;

	m_scene_layers[static_cast<int>(layer)]->AttachChild(std::move(player));
	return m_player_characters.back();
}

//Debug function
//PlayerObject* World::AddPlayer(opt::PlayerIdentifier identifier, const std::string& name, bool is_camera_target)
//{
//	for (int i = identifier; i < 20; i++)
//	{
//		is_camera_target = i == 1;
//
//		std::unique_ptr<PlayerObject> player(
//			new PlayerObject(
//				m_scene_layers,
//				PlatformerCharacterType::kDoc,
//				m_textures,
//				m_fonts,
//				m_sounds,
//				m_camera,
//				is_camera_target
//			));
//
//		player->setScale(0.5f, 0.5f);
//		player->setPosition(200 + (200 * i), 0);
//		player->SetIdentifier(i);
//		player->SetName("Daniel");
//		player->SetColor(ExtraColors::GetColor(static_cast<PlayerColors>(i - 1)));
//
//		m_player_characters.emplace_back(player.get());
//
//		Layers layer = is_camera_target
//			? Layers::kLocalPlayer
//			: Layers::kPlayers;
//
//		m_scene_layers[static_cast<int>(layer)]->AttachChild(std::move(player));
//	}
//	return m_player_characters.front();
//}

void World::RemovePlayer(opt::PlayerIdentifier identifier)
{
	PlayerObject* player = GetPlayer(identifier);
	if (player)
	{
		m_alive_players--;

		player->Destroy();
		m_player_characters.erase(std::find(m_player_characters.begin(), m_player_characters.end(), player));
	}
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
		SceneNode::Ptr layer(new SceneNode(m_scene_layers));
		m_scene_layers[i] = layer.get();
		m_scenegraph.AttachChild(std::move(layer));
	}

	const int map[][50] =
	{
		{0,0,0,0,0,0,2,0,0,0,0,0,0,0,1,3,0,0,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,4,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,5,0,0,0,0,0,0,0,0,0,0,1,5,5,5,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0},
		{0,0,2,2,2,2,2,2,2,2,2,2,2,3,0,0,1,5,5,5,5,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,0},
		{1,2,5,5,5,5,5,5,5,5,5,5,5,6,0,0,4,5,5,5,5,5,5,3,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,5,2,2,2,2,2,2,2,3},
		{4,5,5,5,5,5,5,5,5,5,5,5,5,6,0,0,7,8,8,8,8,8,5,5,3,0,0,0,0,0,0,0,0,0,0,1,5,5,5,5,5,5,5,5,5,5,5,5,5,6},
		{7,8,8,8,8,8,8,8,8,8,8,8,8,9,0,0,0,0,0,0,0,0,4,5,6,0,0,0,0,0,0,0,0,1,2,5,8,8,8,8,8,8,8,8,8,8,8,8,8,6},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,8,5,3,0,0,0,0,0,0,0,4,5,9,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,2,2,2,2,2,2,2,8,8,0,0,0,0,0,0,0,0,0,0,0,0,8,8,9},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,2,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,5,3,0,0,0,5,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,2,0,0,4,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,5,5,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,8,0,0,4,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,8,0,0,0,0,0,0,5,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,4,0,0,0,0,4,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,8,9,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	};

	std::unordered_map<int, TileNode*> top_tiles(50);
	const sf::Texture& crack_texture = m_textures.Get(Textures::kCrack);

	for (int i = 0; i < 15; i++)
	{
		TileNode* last_node = nullptr;
		for (int j = 0; j < 50; j++)
		{
			const int id = map[i][j];
			const bool skip = id == 0;

			const Textures texture_id = skip
				? Textures::kDefault
				: static_cast<Textures>(static_cast<int>(Textures::kDirt1) + id - 1);

			if (!skip)
			{
				sf::Texture& texture = m_textures.Get(texture_id);
				std::unique_ptr<TileNode> tile(
					new TileNode(
						m_scene_layers,
						texture,
						crack_texture));

				tile->setScale(0.5, 0.5);
				tile->setPosition(
					WorldInfo::TILE_SIZE + j * WorldInfo::TILE_SIZE,
					(m_world_bounds.height - WorldInfo::TILE_SIZE * 17) + i * WorldInfo::TILE_SIZE
				);

				TileNode* current_node = tile.get();

				const auto result = top_tiles.emplace(j, tile.get());
				if (result.second)
				{
					tile->SetIsTop(true);
					m_scene_layers[static_cast<int>(Layers::kActivePlatforms)]->AttachChild(std::move(tile));
				}
				else
				{
					top_tiles[j]->AddBelowTile(tile.get());
					m_scene_layers[static_cast<int>(Layers::kPlatforms)]->AttachChild(std::move(tile));
				}

				if (last_node != nullptr)
				{
					current_node->SetLeftTile(last_node);
					last_node->SetRightTile(current_node);
				}
				else //last_node was air so this node is exposed
				{
					current_node->SetActiveCollision();
				}

				last_node = current_node;
			}
			else
			{
				if (last_node != nullptr)
				{
					last_node->SetActiveCollision();
				}

				last_node = nullptr;
			}

		}
	}

	if (m_networked_world)
	{
		std::unique_ptr<NetworkNode> network_node(new NetworkNode(m_scene_layers));
		m_network_node = network_node.get();
		m_scenegraph.AttachChild(std::move(network_node));
	}
}

void World::KillPlayersNotInWorld()
{
	for (const auto& player : m_player_characters)
	{
		if (!player->GetBoundingRect().intersects(m_world_bounds))
		{
			m_alive_players--;
			player->Kill();
		}
	}
}
