#include "World.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include <SFML/Graphics/RectangleShape.hpp>

#include "ParticleNode.hpp"
#include "ParticleType.hpp"
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
	, m_tile_size(64)
	, m_world_bounds(0.f, 0.f, m_tile_size * 52.f, m_tile_size * 24.f)
	, m_spawn_position(100, 100)
	, m_networked_world(networked)
	, m_network_node(nullptr)
	, m_finish_sprite(nullptr)
	, m_elapsed_time()
{
	m_scene_texture.create(m_window.getSize().x, m_window.getSize().y);

	m_camera.SetCenter(m_spawn_position);
	//m_camera.SetSize(384 * 1.5f, 216 * 1.5f);
	//m_camera.SetSize(640, 360);
	m_camera.SetBoundsConstraint(m_world_bounds);

	DangerTrigger::Instance().Clear();
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

	UpdateSounds();
}

void World::Draw()
{
	std::cout << "\nDrawing" << std::endl;
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
	std::cout << "Done" << std::endl;
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
		static_cast<int>(m_world_bounds.height)
	};
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

PlayerObject* World::AddPlayer(opt::PlayerIdentifier identifier, bool is_camera_target)
{
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
	player->setPosition(400, 0);
	player->SetIdentifier(identifier);

	m_player_characters.emplace_back(player.get());
	m_scene_layers[static_cast<int>(Layers::kPlayers)]->AttachChild(std::move(player));
	return m_player_characters.back();
}

//Debug function
//PlayerObject* World::AddPlayer(opt::PlayerIdentifier identifier, bool is_camera_target)
//{
//	for (int i = identifier; i < 20; i++)
//	{
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
//		player->setPosition(400 + (100 * i), 0);
//		player->SetIdentifier(identifier);
//
//		m_player_characters.emplace_back(player.get());
//		m_scene_layers[static_cast<int>(Layers::kPlayers)]->AttachChild(std::move(player));
//	}
//	return m_player_characters.front();
//}

void World::RemovePlayer(opt::PlayerIdentifier identifier)
{
	PlayerObject* player = GetPlayer(identifier);
	if (player)
	{
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
		{0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,9,4,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,1,4,4,4,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0},
		{0,0,2,2,2,2,2,2,2,2,2,2,2,3,0,0,1,4,4,4,4,4,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0},
		{1,2,4,4,4,4,4,4,4,4,4,4,4,5,0,0,9,4,4,4,4,4,4,3,0,0,0,0,0,0,0,0,0,0,0,0,1,2,2,2,2,4,2,2,2,2,2,2,2,3},
		{9,4,4,4,4,4,4,4,4,4,4,4,4,5,0,0,6,7,7,7,7,7,4,4,3,0,0,0,0,0,0,0,0,0,0,1,4,4,4,4,4,4,4,4,4,4,4,4,4,5},
		{6,7,7,7,7,7,7,7,7,7,7,7,7,8,0,0,0,0,0,0,0,0,9,4,5,0,0,0,0,0,0,0,0,1,2,4,7,7,7,7,7,7,7,7,7,7,7,7,7,5},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,7,4,3,0,0,0,0,0,0,0,9,4,8,0,0,0,0,0,0,0,0,0,0,0,0,0,5},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,2,2,2,2,2,2,2,7,7,0,0,0,0,0,0,0,0,0,0,0,0,7,7,8},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,2,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,4,3,0,0,0,4,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,2,0,0,9,0,0,0,0,9,5,0,0,0,0,0,0,0,0,0,4,4,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,7,0,0,9,0,0,0,0,9,5,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,4,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,9,0,0,0,0,9,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	};

	std::unordered_map<int, TileNode*> top_tiles(50);

	for (int i = 0; i < 15; i++)
	{
		TileNode* last_node = nullptr;
		for (int j = 0; j < 50; j++)
		{
			Textures texture_id = Textures::kDefault;
			bool skip = false;

			switch (map[i][j])
			{
			case 0:
				skip = true;
				break;

			case 1:
				texture_id = Textures::kDirt1;
				break;

			case 2:
				texture_id = Textures::kDirt2;
				break;

			case 3:
				texture_id = Textures::kDirt3;
				break;

			case 4:
				texture_id = Textures::kDirt5;
				break;

			case 5:
				texture_id = Textures::kDirt6;
				break;

			case 6:
				texture_id = Textures::kDirt7;
				break;

			case 7:
				texture_id = Textures::kDirt8;
				break;

			case 8:
				texture_id = Textures::kDirt9;
				break;

			case 9:
				texture_id = Textures::kDirt4;
				break;

			default:
				texture_id = Textures::kDefault;
			}

			if (!skip)
			{
				sf::Texture& texture = m_textures.Get(texture_id);
				std::unique_ptr<TileNode> tile(
					new TileNode(
						m_scene_layers,
						texture));

				tile->setScale(0.5, 0.5);
				tile->setPosition(
					m_tile_size + j * m_tile_size,
					(m_world_bounds.height - m_tile_size * 17) + i * m_tile_size
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

	//Prepare the background
	//sf::Texture& nebula = m_textures.Get(Textures::kBlueNebula);
	//nebula.setRepeated(true);

	//Add the background sprite to our scene
	//std::unique_ptr<SpriteNode> nebula_sprite(new SpriteNode(m_scene_layers, nebula, GetBackgroundRect(nebula)));
	//m_scene_layers[static_cast<int>(Layers::kBackground)]->AttachChild(std::move(nebula_sprite));

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