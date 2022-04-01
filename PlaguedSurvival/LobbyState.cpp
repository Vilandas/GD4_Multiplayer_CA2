#include "LobbyState.hpp"

#include "MusicPlayer.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/IpAddress.hpp>

#include <fstream>
#include <iostream>
#include <SFML/Network/Packet.hpp>

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

LobbyState::LobbyState(StateStack& stack, Context context, bool is_host)
	: State(stack, context)
	, m_window(*context.window)
	, m_texture_holder(*context.textures)
	, m_font_holder(*context.fonts)
	, m_camera(m_window.getDefaultView())
	, m_gui_container(m_window, m_camera)
	, m_connected(false)
	, m_game_server(nullptr)
	, m_active_state(true)
	, m_has_focus(true)
	, m_host(is_host)
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

	sf::IpAddress ip;
	if (m_host)
	{
		m_game_server.reset(new GameServer());
		ip = "127.0.0.1";
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

	//Play game theme
	context.music->Play(MusicThemes::kMissionTheme);
}

void LobbyState::Draw()
{
	if (m_connected)
	{
		//Show broadcast messages in default view
		m_window.setView(m_window.getDefaultView());

		m_window.draw(m_background_sprite);
		m_window.draw(m_gui_container);

		if (!m_broadcasts.empty())
		{
			m_window.draw(m_broadcast_text);
		}

		if (m_local_player_identifiers.size() < 2 && m_player_invitation_time < sf::seconds(0.5f))
		{
			m_window.draw(m_player_invitation_text);
		}
	}
	else
	{
		m_window.draw(m_failed_connection_text);
	}
}

bool LobbyState::Update(sf::Time dt)
{
	//Connected to the Server: Handle all the network logic
	if (m_connected)
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

		UpdateBroadcastMessage(dt);

		//Regular updates
		if (m_tick_clock.getElapsedTime() > sf::seconds(1.f / 20.f))
		{
			sf::Packet update_packet;
			update_packet << static_cast<opt::ClientPacket>(Client::PacketType::StillHereUpdate);

			for (const sf::Int32 identifier : m_local_player_identifiers)
			{
				update_packet << identifier;
			}

			m_socket.send(update_packet);
			m_tick_clock.restart();
		}

		m_time_since_last_packet += dt;
	}

	//Failed to connect and waited for more than 5 seconds: Back to menu
	else if (m_failed_connection_clock.getElapsedTime() >= sf::seconds(5.f))
	{
		RequestStackClear();
		RequestStackPush(StateID::kMenu);
	}
	return true;
}

bool LobbyState::HandleEvent(const sf::Event& event)
{
	return true;
}

void LobbyState::OnActivate()
{
	m_active_state = true;
}

void LobbyState::OnDestroy()
{
	if (!m_host && m_connected)
	{
		//Inform server this client is dying
		sf::Packet packet;
		packet << static_cast<opt::ClientPacket>(Client::PacketType::Quit);
		m_socket.send(packet);
	}
}

void LobbyState::UpdateBroadcastMessage(sf::Time elapsed_time)
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

void LobbyState::HandlePacket(opt::ServerPacket packet_type, sf::Packet& packet)
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
			m_local_player_identifiers.push_back(player_identifier);
		}
		break;

		case Server::PacketType::PlayerConnect:
		{
			opt::PlayerIdentifier player_identifier;
			packet >> player_identifier;

			GeneratePlayer(player_identifier);
		}
		break;

		case Server::PacketType::PlayerDisconnect:
		{
			opt::PlayerIdentifier player_identifier;
			packet >> player_identifier;

			m_gui_container.Unpack(m_players[player_identifier].m_name);
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
			}
		}
		break;

		case Server::PacketType::AcceptCoopPartner:
		{
			opt::PlayerIdentifier player_identifier;
			packet >> player_identifier;

			GeneratePlayer(player_identifier);
			m_local_player_identifiers.emplace_back(player_identifier);
		}
		break;

		case Server::PacketType::UpdateClientState:
		{
		}
		break;
	}
}

void LobbyState::GeneratePlayer(opt::PlayerIdentifier identifier)
{
	std::string name = std::to_string(identifier);
	const auto label = std::make_shared<GUI::Label>(name, m_font_holder);

	label->setPosition(100, 200 + (20 * identifier));

	m_players[identifier].m_name = label;
	m_gui_container.Pack(label);
}

void LobbyState::GeneratePlayer(opt::PlayerIdentifier identifier, const std::string& name)
{
	const auto label = std::make_shared<GUI::Label>(name, m_font_holder);

	label->setPosition(100, 200 + (20 * identifier));

	m_players[identifier].m_name = label;
	m_gui_container.Pack(label);
}
