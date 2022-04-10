#include "MultiplayerGameState.hpp"

#include "MusicPlayer.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>
#include <iostream>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Network/Packet.hpp>

#include "DangerTrigger.hpp"
#include "PlayerColors.hpp"

sf::IpAddress GetAddressFromFile()
{
	{
		//Try to open existing file ip.txt
		std::ifstream input_file("ip.txt");
		std::string ip_address;
		if (input_file >> ip_address)
		{
			return ip_address;
		}
	}

	//If open/read failed, create a new file
	std::ofstream output_file("ip.txt");
	std::string local_address = "127.0.0.1";
	output_file << local_address;
	return local_address;
}

MultiplayerGameState::MultiplayerGameState(StateStack& stack, Context context, bool is_host)
	: State(stack, context)
	, m_world(*context.window, *context.textures, *context.fonts, *context.sounds, *context.camera, true)
	, m_window(*context.window)
	, m_texture_holder(*context.textures)
	, m_font_holder(*context.fonts)
	, m_music(*context.music)
	, m_camera(m_window.getDefaultView())
	, m_lobby_gui(m_window, m_camera)
	, m_connected(false)
	, m_game_server(nullptr)
	, m_active_state(true)
	, m_has_focus(true)
	, m_host(is_host)
	, m_lobby(true)
	, m_client_timeout(sf::seconds(2.f))
	, m_time_since_last_packet(sf::seconds(0.f))
{
	m_background_sprite.setTexture(context.textures->Get(Textures::kTitleScreen));

	m_broadcast_text.setFont(context.fonts->Get(Fonts::Main));
	m_broadcast_text.setPosition(1024.f / 2, 100.f);

	//We reuse this text for "Attempt to connect" and "Failed to connect" messages
	m_failed_connection_text.setFont(context.fonts->Get(Fonts::Main));
	m_failed_connection_text.setString("Attempting to connect...");
	m_failed_connection_text.setCharacterSize(35);
	m_failed_connection_text.setFillColor(sf::Color::White);
	Utility::CentreOrigin(m_failed_connection_text);
	m_failed_connection_text.setPosition(m_window.getSize().x / 2.f, m_window.getSize().y / 2.f);

	//Render an "establishing connection" frame for user feedback
	m_window.clear(sf::Color::Black);
	m_window.draw(m_failed_connection_text);
	m_window.display();
	m_failed_connection_text.setString("Could not connect to the remote server");
	Utility::CentreOrigin(m_failed_connection_text);

	m_lobby_text.setFont(context.fonts->Get(Fonts::Main));
	m_lobby_text.setString("Lobby");
	m_lobby_text.setCharacterSize(35);
	m_lobby_text.setFillColor(sf::Color::White);
	Utility::CentreOrigin(m_lobby_text);
	m_lobby_text.setPosition(960, 200);

	m_waiting_for_host_text.setFont(context.fonts->Get(Fonts::Main));
	m_waiting_for_host_text.setString("Waiting for host to start the game");
	m_waiting_for_host_text.setCharacterSize(24);
	m_waiting_for_host_text.setFillColor(sf::Color::White);
	Utility::CentreOrigin(m_waiting_for_host_text);
	m_waiting_for_host_text.setPosition(960, 800);

	sf::IpAddress ip;
	if (m_host)
	{
		m_game_server.reset(new GameServer());
		ip = "127.0.0.1";

		auto start_button = std::make_shared<GUI::Button>(context);
		start_button->setPosition(860, 860);
		start_button->SetText("Start Game");
		start_button->SetCallback([this]()
			{
				sf::Packet packet;
				packet << static_cast<opt::ClientPacket>(Client::PacketType::RequestStartGame);
				m_socket.send(packet);
			});

		m_lobby_gui.Pack(start_button);
	}
	else
	{
		ip = GetAddressFromFile();
	}

	if (m_socket.connect(ip, SERVER_PORT, sf::seconds(5.f)) == sf::TcpSocket::Done)
	{
		m_connected = true;
	}
	else
	{
		m_failed_connection_clock.restart();
	}

	m_socket.setBlocking(false);
}

void MultiplayerGameState::Draw()
{
	if (m_connected)
	{
		//Show broadcast messages in default view
		m_window.setView(m_window.getDefaultView());

		if (m_lobby)
		{
			m_window.draw(m_background_sprite);

			sf::RectangleShape backgroundShape;
			backgroundShape.setFillColor(sf::Color(0, 0, 0, 200));
			backgroundShape.setSize(sf::Vector2f(600, 800));
			Utility::CentreOrigin(backgroundShape);
			backgroundShape.setPosition(960, 540);

			m_window.draw(backgroundShape);
			m_window.draw(m_lobby_gui);
			m_window.draw(m_lobby_text);

			if (!m_host)
			{
				m_window.draw(m_waiting_for_host_text);
			}


			if (m_local_player_identifiers.size() < 2 && m_player_invitation_time < sf::seconds(0.5f))
			{
				m_window.draw(m_player_invitation_text);
			}
		}
		else
		{
			m_world.Draw();
		}

		if (!m_broadcasts.empty())
		{
			m_window.draw(m_broadcast_text);
		}
	}
	else
	{
		m_window.draw(m_failed_connection_text);
	}
}

bool MultiplayerGameState::Update(sf::Time dt)
{
	//Connected to the Server: Handle all the network logic
	if (m_connected)
	{
		if (m_lobby)
		{
			UpdateLobby(dt);
		}
		else
		{
			UpdateGame(dt);
		}
	}

	//Failed to connect and waited for more than 5 seconds: Back to menu
	else if (m_failed_connection_clock.getElapsedTime() >= sf::seconds(5.f))
	{
		RequestStackClear();
		RequestStackPush(StateID::kMenu);
	}
	return true;
}

void MultiplayerGameState::UpdateLobby(sf::Time dt)
{
	ReceivePacket();
	UpdateBroadcastMessage(dt);

	//Regular updates
	if (m_tick_clock.getElapsedTime() > sf::seconds(10.f / 20.f))
	{
		sf::Packet update_packet;
		update_packet << static_cast<opt::ClientPacket>(Client::PacketType::StillHereUpdate);

		for (const opt::PlayerIdentifier identifier : m_local_player_identifiers)
		{
			update_packet << identifier;
		}

		m_socket.send(update_packet);
		m_tick_clock.restart();
	}

	m_time_since_last_packet += dt;
}

void MultiplayerGameState::UpdateGame(sf::Time dt)
{
	m_world.Update(dt);

	//Remove players who were destroyed
	bool found_local_plane = false;
	for (auto itr = m_players.begin(); itr != m_players.end();)
	{
		//Check if there are no more local planes for remote clients
		if (std::find(m_local_player_identifiers.begin(), m_local_player_identifiers.end(), itr->first) != m_local_player_identifiers.end())
		{
			found_local_plane = true;
		}

		if (!m_world.GetPlayer(itr->first))
		{
			itr = m_players.erase(itr);

			//No more players left : Mission failed
			if (m_players.empty())
			{
				RequestStackPush(StateID::kGameOver);
			}
		}
		else
		{
			++itr;
		}
	}

	if (!found_local_plane)
	{
		RequestStackPush(StateID::kGameOver);
	}

	//Only handle the realtime input if the window has focus and the game is unpaused
	if (m_active_state && m_has_focus)
	{
		CommandQueue& commands = m_world.GetCommandQueue();
		for (auto& pair : m_players)
		{
			pair.second.m_player->HandleRealtimeInput(commands);
		}
	}

	//Always handle the network input
	CommandQueue& commands = m_world.GetCommandQueue();
	for (auto& pair : m_players)
	{
		pair.second.m_player->HandleRealtimeNetworkInput(commands);
	}

	ReceivePacket();
	UpdateBroadcastMessage(dt);

	//Events occurring in the game
	GameActions::Action game_action;
	while (m_world.PollGameAction(game_action))
	{
		sf::Packet packet;
		packet << static_cast<opt::ClientPacket>(Client::PacketType::GameEvent);
		packet << static_cast<opt::Action>(game_action.type);
		packet << game_action.position.x;
		packet << game_action.position.y;

		m_socket.send(packet);
	}

	//Regular position updates
	if (m_tick_clock.getElapsedTime() > sf::seconds(1.f / 20.f))
	{
		sf::Packet position_update_packet;
		position_update_packet << static_cast<opt::ClientPacket>(Client::PacketType::PositionUpdate);
		position_update_packet << static_cast<opt::LocalPlayers>(m_local_player_identifiers.size());

		for (const opt::PlayerIdentifier identifier : m_local_player_identifiers)
		{
			if (const PlayerObject* player = m_world.GetPlayer(identifier))
			{
				position_update_packet
					<< identifier
					<< player->getPosition().x
					<< player->getPosition().y
					<< static_cast<opt::HitPoints>(player->GetHitPoints());
			}
		}
		m_socket.send(position_update_packet);
		m_tick_clock.restart();
	}
	m_time_since_last_packet += dt;
}

void MultiplayerGameState::ReceivePacket()
{
	//Handle messages from the server that may have arrived
	sf::Packet packet;
	if (m_socket.receive(packet) == sf::Socket::Done)
	{
		m_time_since_last_packet = sf::seconds(0.f);
		opt::ServerPacket packet_type;
		packet >> packet_type;
		HandlePacket(packet_type, packet);
	}
	else
	{
		//Check for timeout with the server
		if (m_time_since_last_packet > m_client_timeout)
		{
			m_connected = false;
			m_failed_connection_text.setString("Lost connection to the server");
			Utility::CentreOrigin(m_failed_connection_text);

			m_failed_connection_clock.restart();
		}
	}
}

bool MultiplayerGameState::HandleEvent(const sf::Event& event)
{
	if (m_lobby)
	{
		m_lobby_gui.HandleEvent(event);

		//If enter pressed, add second player co-op only if there is only 1 player
		if (event.key.code == sf::Keyboard::Return && m_local_player_identifiers.size() == 1)
		{
			sf::Packet packet;
			packet << static_cast<opt::ClientPacket>(Client::PacketType::RequestCoopPartner);
			m_socket.send(packet);
		}
	}
	else
	{
		//If B pressed, toggle bounds vision
		if (event.key.code == sf::Keyboard::B && event.type == sf::Event::KeyReleased)
		{
			const bool current_flag = WorldInfo::Instance().ShowCollisionBounds();
			WorldInfo::Instance().SetShowCollisionBounds(!current_flag);
		}


		//Game input handling
		CommandQueue& commands = m_world.GetCommandQueue();

		//Forward events to all players
		for (auto& pair : m_players)
		{
			pair.second.m_player->HandleEvent(event, commands);
		}

		if (event.type == sf::Event::KeyPressed)
		{
			//If escape is pressed, show the pause screen
			if (event.key.code == sf::Keyboard::Escape)
			{
				DisableAllRealtimeActions();
				RequestStackPush(StateID::kNetworkPause);
			}
		}
		else if (event.type == sf::Event::GainedFocus)
		{
			m_has_focus = true;
		}
		else if (event.type == sf::Event::LostFocus)
		{
			m_has_focus = false;
		}

	}

	return true;
}

void MultiplayerGameState::DisableAllRealtimeActions()
{
	m_active_state = false;
	for (opt::PlayerIdentifier identifier : m_local_player_identifiers)
	{
		m_players[identifier].m_player->DisableAllRealtimeActions();
	}
}

void MultiplayerGameState::OnActivate()
{
	m_active_state = true;
}

void MultiplayerGameState::OnDestroy()
{
	if (!m_host && m_connected)
	{
		//Inform server this client is dying
		sf::Packet packet;
		packet << static_cast<opt::ClientPacket>(Client::PacketType::Quit);
		m_socket.send(packet);
	}
}

void MultiplayerGameState::UpdateBroadcastMessage(sf::Time elapsed_time)
{
	if (m_broadcasts.empty())
	{
		return;
	}

	//Update broadcast timer
	m_broadcast_elapsed_time += elapsed_time;
	if (m_broadcast_elapsed_time > sf::seconds(2.f))
	{
		//If message has expired, remove it
		m_broadcasts.erase(m_broadcasts.begin());

		//Continue to display the next broadcast message
		if (!m_broadcasts.empty())
		{
			m_broadcast_text.setString(m_broadcasts.front());
			Utility::CentreOrigin(m_broadcast_text);
			m_broadcast_elapsed_time = sf::Time::Zero;
		}
	}
}

void MultiplayerGameState::HandlePacket(opt::ServerPacket packet_type, sf::Packet& packet)
{
	const auto packet_type_cast = static_cast<Server::PacketType>(packet_type);
	switch (packet_type_cast)
	{
		//Send message to all Clients
		case Server::PacketType::BroadcastMessage:
		{
			std::string message;
			packet >> message;
			m_broadcasts.push_back(message);

			//Just added the first message, display immediately
			if (m_broadcasts.size() == 1)
			{
				m_broadcast_text.setString(m_broadcasts.front());
				Utility::CentreOrigin(m_broadcast_text);
				m_broadcast_elapsed_time = sf::Time::Zero;
			}
		}
		break;

		//Sent by the server to spawn player 1 on connect
		case Server::PacketType::SpawnSelf:
		{
			opt::PlayerIdentifier player_identifier;
			packet >> player_identifier;

			GeneratePlayer(player_identifier);
			m_players[player_identifier].m_player.reset(new Player(&m_socket, player_identifier, GetContext().keys1));
			m_local_player_identifiers.push_back(player_identifier);
			m_world.AddPlayer(player_identifier, m_players[player_identifier].m_name->GetText(), true);
		}
		break;

		case Server::PacketType::PlayerConnect:
		{
			opt::PlayerIdentifier player_identifier;
			packet >> player_identifier;


			GeneratePlayer(player_identifier);
			m_players[player_identifier].m_player.reset(new Player(&m_socket, player_identifier, nullptr));
			m_world.AddPlayer(player_identifier, m_players[player_identifier].m_name->GetText(), false);
		}
		break;

		case Server::PacketType::PlayerDisconnect:
		{
			opt::PlayerIdentifier player_identifier;
			packet >> player_identifier;

			m_world.RemovePlayer(player_identifier);
			m_lobby_gui.Unpack(m_players[player_identifier].m_name);
			m_players.erase(player_identifier);
		}
		break;

		case Server::PacketType::InitialState:
		{
			sf::Uint32 seed;
			opt::PlayerCount player_count;
			packet >> seed
				>> player_count;

			Utility::UpdateRandomEngine(seed);
			for (opt::PlayerCount i = 0; i < player_count; ++i)
			{
				opt::PlayerIdentifier player_identifier;
				std::string player_name;

				packet >> player_identifier
					>> player_name;

				GeneratePlayer(player_identifier, player_name);
				m_players[player_identifier].m_player.reset(new Player(&m_socket, player_identifier, nullptr));
				m_world.AddPlayer(player_identifier, player_name, false);
			}
		}
		break;

		case Server::PacketType::AcceptCoopPartner:
		{
			opt::PlayerIdentifier player_identifier;
			packet >> player_identifier;

			GeneratePlayer(player_identifier);
			m_players[player_identifier].m_player.reset(new Player(&m_socket, player_identifier, GetContext().keys2));
			m_local_player_identifiers.emplace_back(player_identifier);
			m_world.AddPlayer(player_identifier, std::to_string(player_identifier), false);
		}
		break;

		//Player's movement or fire keyboard state changes
		case Server::PacketType::PlayerRealtimeChange:
		{
			opt::PlayerIdentifier player_identifier;
			opt::Action action;
			bool action_enabled;
			packet >> player_identifier >> action >> action_enabled;
			auto itr = m_players.find(player_identifier);
			if (itr != m_players.end())
			{
				itr->second.m_player->HandleNetworkRealtimeChange(static_cast<PlayerAction>(action), action_enabled);
			}
		}
		break;

		//Player event, like missile fired occurs
		case Server::PacketType::PlayerEvent:
		{
			opt::PlayerIdentifier player_identifier;
			opt::Action action;
			packet >> player_identifier >> action;
			auto itr = m_players.find(player_identifier);
			if (itr != m_players.end())
			{
				itr->second.m_player->HandleNetworkEvent(static_cast<PlayerAction>(action), m_world.GetCommandQueue());
			}
		}
		break;


		case Server::PacketType::UpdateClientState:
		{
			if (m_lobby) break;

			opt::PlayerCount player_count;
			packet >> player_count;

			for (opt::PlayerCount i = 0; i < player_count; ++i)
			{
				sf::Vector2f player_position;
				opt::PlayerIdentifier player_identifier;
				packet >> player_identifier >> player_position.x >> player_position.y;

				PlayerObject* player = m_world.GetPlayer(player_identifier);
				bool is_local_plane = std::find(m_local_player_identifiers.begin(), m_local_player_identifiers.end(), player_identifier) != m_local_player_identifiers.end();
				if (player && !is_local_plane)
				{
					sf::Vector2f interpolated_position = player->getPosition() + (player_position - player->getPosition()) * 0.1f;
					player->setPosition(interpolated_position);
				}
			}
		}
		break;


		case Server::PacketType::UpdateDangerTime:
		{
			float time;
			packet >> time;

			DangerTrigger::Instance().Update(sf::seconds(time));
		}
		break;


		case Server::PacketType::StartGame:
		{
			m_lobby = false;
			m_music.Play(MusicThemes::kMenuTheme);
		}
		break;


		case Server::PacketType::MissionSuccess:
		{
			opt::PlayerIdentifier winner_id;
			packet >> winner_id;

			*GetContext().game_winner = winner_id == 0
				? "Nobody"
				: m_players[winner_id].m_name->GetText();

			RequestStackPush(StateID::kGameOver);
		}
		break;
	}
}

void MultiplayerGameState::GeneratePlayer(opt::PlayerIdentifier identifier)
{
	std::string name = "Player " + std::to_string(identifier);
	const auto label = std::make_shared<GUI::Label>(name, m_font_holder);

	label->SetCharacterSize(20);
	label->CentreOriginText();
	label->setPosition(960, 220 + (30 * identifier));
	label->SetFillColor(ExtraColors::GetColor(static_cast<PlayerColors>(identifier - 1)));

	m_players[identifier].m_name = label;
	m_lobby_gui.Pack(label);
}

void MultiplayerGameState::GeneratePlayer(opt::PlayerIdentifier identifier, const std::string& name)
{
	const auto label = std::make_shared<GUI::Label>(name, m_font_holder);

	label->SetCharacterSize(20);
	label->CentreOriginText();
	label->setPosition(960, 220 + (30 * identifier));
	label->SetFillColor(ExtraColors::GetColor(static_cast<PlayerColors>(identifier - 1)));

	m_players[identifier].m_name = label;
	m_lobby_gui.Pack(label);
}
