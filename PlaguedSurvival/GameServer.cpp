#include "GameServer.hpp"

#include <iostream>

#include "NetworkProtocol.hpp"
#include <SFML/System.hpp>

#include <SFML/Network/Packet.hpp>

#include "PlayerAction.hpp"
#include "Utility.hpp"
#include "WorldInfo.hpp"

//It is essential to set the sockets to non-blocking - m_socket.setBlocking(false)
//otherwise the server will hang waiting to read input from a connection

GameServer::RemotePeer::RemotePeer() :m_ready(false), m_timed_out(false)
{
	m_socket.setBlocking(false);
}

GameServer::GameServer()
	: m_thread(&GameServer::ExecutionThread, this)
	, m_listening_state(false)
	, m_lobby(true)
	, m_client_timeout(sf::seconds(1.f))
	, m_max_connected_players(15)
	, m_connected_players(0)
	, m_player_count(0)
	, m_peers(1)
	, m_waiting_thread_end(false)
	, m_alive_players()
{
	m_listener_socket.setBlocking(false);
	m_peers[0].reset(new RemotePeer());
	m_thread.launch();
}

GameServer::~GameServer()
{
	m_waiting_thread_end = true;
	m_thread.wait();
}

//This is the same as SpawnSelf but indicate that a player from a different client is entering the world

void GameServer::NotifyPlayerSpawn(opt::PlayerIdentifier player_identifier)
{
	sf::Packet packet;
	//First thing for every packet is what type of packet it is
	packet << static_cast<opt::ServerPacket>(Server::PacketType::PlayerConnect);
	packet << player_identifier << m_player_info[player_identifier].m_position.x << m_player_info[player_identifier].m_position.y;
	for (std::size_t i = 0; i < m_connected_players; ++i)
	{
		if (m_peers[i]->m_ready)
		{
			m_peers[i]->m_socket.send(packet);
		}
	}
}

//This is the same as PlayerEvent, but for real-time actions. This means that we are changing an ongoing state to either true or false, so we add a Boolean value to the parameters

void GameServer::NotifyPlayerRealtimeChange(opt::PlayerIdentifier player_identifier, opt::Action action, bool action_enabled)
{
	sf::Packet packet;
	//First thing for every packet is what type of packet it is
	packet << static_cast<opt::ServerPacket>(Server::PacketType::PlayerRealtimeChange);
	packet << player_identifier;
	packet << action;
	packet << action_enabled;

	for (std::size_t i = 0; i < m_connected_players; ++i)
	{
		if (m_peers[i]->m_ready)
		{
			m_peers[i]->m_socket.send(packet);
		}
	}
}

//This takes two sf::Int32 variables, the player identifier and the action identifier
//as declared in the Player class. This is used to inform all peers that plane X has
//triggered an action

void GameServer::NotifyPlayerEvent(opt::PlayerIdentifier player_identifier, opt::Action action)
{
	sf::Packet packet;
	//First thing for every packet is what type of packet it is
	packet << static_cast<opt::ServerPacket>(Server::PacketType::PlayerEvent);
	packet << player_identifier;
	packet << action;

	for (std::size_t i = 0; i < m_connected_players; ++i)
	{
		if (m_peers[i]->m_ready)
		{
			m_peers[i]->m_socket.send(packet);
		}
	}
}

void GameServer::SetListening(bool enable)
{
	//Check if the server listening socket is already listening
	if (enable)
	{
		if (!m_listening_state)
		{
			m_listening_state = (m_listener_socket.listen(SERVER_PORT) == sf::TcpListener::Done);
		}
	}
	else
	{
		m_listener_socket.close();
		m_listening_state = false;
	}
}

void GameServer::ExecutionThread()
{
	SetListening(true);

	sf::Time frame_rate = sf::seconds(1.f / 60.f);
	sf::Time frame_time = sf::Time::Zero;
	sf::Time tick_rate = sf::seconds(1.f / 20.f);
	sf::Time tick_time = sf::Time::Zero;
	sf::Time danger_rate = sf::seconds(1.f);
	sf::Time danger_time = sf::Time::Zero;
	sf::Clock frame_clock, tick_clock, danger_clock;

	while (!m_waiting_thread_end)
	{
		HandleIncomingConnections();
		HandleIncomingPackets();

		frame_time += frame_clock.getElapsedTime();
		frame_clock.restart();

		tick_time += tick_clock.getElapsedTime();
		tick_clock.restart();

		//Fixed update step
		while (frame_time >= frame_rate)
		{
			frame_time -= frame_rate;
		}

		//Fixed tick step
		while (tick_time >= tick_rate)
		{
			Tick();
			tick_time -= tick_rate;
		}

		if (!m_lobby)
		{
			danger_time += danger_clock.getElapsedTime();
			danger_clock.restart();

			if (danger_time >= danger_rate)
			{
				UpdateDangers(danger_time);
				danger_time = sf::Time::Zero;
			}
		}

		//sleep
		sf::sleep(sf::milliseconds(100));
	}
}

void GameServer::Tick()
{
	UpdateClientState();

	if (!m_lobby && m_alive_players <= 1)
	{
		const opt::PlayerIdentifier winner_id = FindWinnerIdentity();

		sf::Packet packet;
		packet << static_cast<opt::ServerPacket>(Server::PacketType::MissionSuccess)
			<< winner_id;

		SendToAll(packet);
	}
}

opt::PlayerIdentifier GameServer::FindWinnerIdentity() const
{
	for (const auto& player : m_player_info)
	{
		if (player.second.m_hitpoints > 0)
		{
			return player.first;
		}
	}

	return 0;
}

sf::Time GameServer::Now() const
{
	return m_clock.getElapsedTime();
}

void GameServer::HandleIncomingPackets()
{
	bool detected_timeout = false;

	for (PeerPtr& peer : m_peers)
	{
		if (peer->m_ready)
		{
			sf::Packet packet;
			while (peer->m_socket.receive(packet) == sf::Socket::Done)
			{
				//Interpret the packet and react to it
				HandleIncomingPacket(packet, *peer, detected_timeout);

				peer->m_last_packet_time = Now();
				packet.clear();
			}

			if (Now() > peer->m_last_packet_time + m_client_timeout)
			{
				peer->m_timed_out = true;
				detected_timeout = true;
			}
		}
	}

	if (detected_timeout)
	{
		HandleDisconnections();
	}
}

void GameServer::HandleIncomingPacket(sf::Packet& packet, RemotePeer& receiving_peer, bool& detected_timeout)
{
	opt::ClientPacket packet_type;
	packet >> packet_type;

	switch (static_cast<Client::PacketType> (packet_type))
	{
	case Client::PacketType::Quit:
	{
		receiving_peer.m_timed_out = true;
		detected_timeout = true;
	}
	break;

	case Client::PacketType::UpdateGamesWon:
	{
		opt::PlayerIdentifier player_identifier;
		opt::GamesWon games_won;
		packet >> player_identifier >> games_won;

		m_player_info[player_identifier].m_games_won = games_won;

		sf::Packet games_won_packet;
		games_won_packet << static_cast<opt::ServerPacket>(Server::PacketType::GamesWonUpdated)
			<< player_identifier
			<< games_won;

		SendToAll(games_won_packet);
	}
	break;

	case Client::PacketType::PlayerEvent:
	{
		opt::PlayerIdentifier player_identifier;
		opt::Action action;
		packet >> player_identifier >> action;

		if (static_cast<PlayerAction>(action) == PlayerAction::kAttack)
		{
			if (PlayerCanAttack(player_identifier))
			{
				PlayerAttack(player_identifier);
				NotifyPlayerEvent(player_identifier, action);
			}
		}
		else
		{
			NotifyPlayerEvent(player_identifier, action);
		}
	}
	break;

	case Client::PacketType::PlayerRealtimeChange:
	{
		opt::PlayerIdentifier player_identifier;
		opt::Action action;
		bool action_enabled;
		packet >> player_identifier >> action >> action_enabled;
		NotifyPlayerRealtimeChange(player_identifier, action, action_enabled);
	}
	break;

	case Client::PacketType::RequestCoopPartner:
	{
		m_alive_players++;
		opt::PlayerIdentifier identifier = GetFreeIdentifier();
		receiving_peer.m_player_identifiers.emplace_back(identifier);

		m_player_info[identifier].m_name = std::to_string(identifier);
		m_player_info[identifier].m_position = sf::Vector2f(0, 0);
		m_player_info[identifier].m_hitpoints = 100;

		sf::Packet request_packet;
		request_packet << static_cast<opt::ServerPacket>(Server::PacketType::AcceptCoopPartner);
		request_packet << identifier;

		receiving_peer.m_socket.send(request_packet);
		m_player_count++;

		// Tell everyone else about the new player
		sf::Packet notify_packet;
		notify_packet << static_cast<opt::ServerPacket>(Server::PacketType::PlayerConnect);
		notify_packet << identifier;

		for (PeerPtr& peer : m_peers)
		{
			if (peer.get() != &receiving_peer && peer->m_ready)
			{
				peer->m_socket.send(notify_packet);
			}
		}
	}
	break;

	case Client::PacketType::StillHereUpdate:
	{
		opt::PlayerIdentifier player_identifier;
		packet >> player_identifier;
	}
	break;

	case Client::PacketType::PositionUpdate:
	{
		opt::PlayerCount player_count;
		packet >> player_count;

		for (opt::PlayerCount i = 0; i < player_count; ++i)
		{
			opt::PlayerIdentifier player_identifier;
			sf::Vector2f player_position;
			packet >> player_identifier >> player_position.x >> player_position.y;

			m_player_info[player_identifier].m_position = player_position;

			if (m_player_info[player_identifier].m_hitpoints > 0 && IsPlayerUnderWorld(player_identifier))
			{
				m_player_info[player_identifier].m_hitpoints = 0;
				m_alive_players--;

				sf::Packet notify_packet;
				notify_packet << static_cast<opt::ServerPacket>(Server::PacketType::PlayerDied)
					<< player_identifier;

				SendToAll(notify_packet);
			}
		}
	}
	break;

	case Client::PacketType::GameEvent:
	{
		opt::Action action;
		float x;
		float y;

		packet >> action;
		packet >> x;
		packet >> y;
	}
	break;

	case Client::PacketType::RequestStartGame:
	{
		SetListening(false);
		sf::Packet packet;
		packet << static_cast<opt::ServerPacket>(Server::PacketType::StartGame);
		SendToAll(packet);

		m_lobby = false;
	}
	break;
	}
}

opt::PlayerIdentifier GameServer::GetFreeIdentifier() const
{
	opt::PlayerIdentifier identifier = 1;

	while (true)
	{
		if (m_player_info.count(identifier))
		{
			identifier++;
		}
		else
		{
			return identifier;
		}
	}
}

void GameServer::HandleIncomingConnections()
{
	if (!m_listening_state)
	{
		return;
	}

	if (m_listener_socket.accept(m_peers[m_connected_players]->m_socket) == sf::TcpListener::Done)
	{
		m_alive_players++;
		const opt::PlayerIdentifier identifier = GetFreeIdentifier();

		//Order the new client to spawn its player 1
		m_player_info[identifier].m_name = "Player " + std::to_string(identifier);
		m_player_info[identifier].m_position = sf::Vector2f(0, 0);
		m_player_info[identifier].m_hitpoints = 100;
		m_player_info[identifier].m_games_won = 0;


		sf::Packet packet;
		packet << static_cast<opt::ServerPacket>(Server::PacketType::SpawnSelf);
		packet << identifier;

		m_peers[m_connected_players]->m_player_identifiers.emplace_back(identifier);

		BroadcastMessage("New player");
		InformWorldState(m_peers[m_connected_players]->m_socket);
		NotifyPlayerSpawn(identifier);

		m_peers[m_connected_players]->m_socket.send(packet);
		m_peers[m_connected_players]->m_ready = true;
		m_peers[m_connected_players]->m_last_packet_time = Now();

		m_player_count++;
		m_connected_players++;

		if (m_connected_players >= m_max_connected_players)
		{
			SetListening(false);
		}
		else
		{
			m_peers.emplace_back(PeerPtr(new RemotePeer()));
		}
	}
}

void GameServer::HandleDisconnections()
{
	for (auto itr = m_peers.begin(); itr != m_peers.end();)
	{
		if ((*itr)->m_timed_out)
		{
			//Inform everyone of a disconnection, erase
			for (opt::PlayerIdentifier identifier : (*itr)->m_player_identifiers)
			{
				m_alive_players--;
				SendToAll((sf::Packet() << static_cast<opt::ServerPacket>(Server::PacketType::PlayerDisconnect) << identifier));
				m_player_info.erase(identifier);
			}

			m_connected_players--;
			m_player_count -= (*itr)->m_player_identifiers.size();

			itr = m_peers.erase(itr);

			//If the number of peers has dropped below max_connections
			if (m_connected_players < m_max_connected_players)
			{
				m_peers.emplace_back(PeerPtr(new RemotePeer()));
				SetListening(true);
			}

			BroadcastMessage("A player has disconnected");
		}
		else
		{
			++itr;
		}
	}
}

void GameServer::InformWorldState(sf::TcpSocket& socket)
{
	sf::Packet packet;
	packet << static_cast<opt::ServerPacket>(Server::PacketType::InitialState)
	<< static_cast<sf::Uint32>(Utility::GetSeed())
	<< static_cast<opt::PlayerCount>(m_player_count);

	for (std::size_t i = 0; i < m_connected_players; ++i)
	{
		if (m_peers[i]->m_ready)
		{
			for (opt::PlayerIdentifier identifier : m_peers[i]->m_player_identifiers)
			{
				packet << identifier
					<< m_player_info[identifier].m_name
					<< m_player_info[identifier].m_games_won;
			}
		}
	}

	socket.send(packet);
}

void GameServer::BroadcastMessage(const std::string& message)
{
	sf::Packet packet;
	packet << static_cast<opt::ServerPacket>(Server::PacketType::BroadcastMessage);
	packet << message;
	for (std::size_t i = 0; i < m_connected_players; ++i)
	{
		if (m_peers[i]->m_ready)
		{
			m_peers[i]->m_socket.send(packet);
		}
	}
}

void GameServer::SendToAll(sf::Packet& packet)
{
	for (PeerPtr& peer : m_peers)
	{
		if (peer->m_ready)
		{
			peer->m_socket.send(packet);
		}
	}
}

void GameServer::UpdateClientState()
{
	sf::Packet update_client_state_packet;
	update_client_state_packet << static_cast<opt::ServerPacket>(Server::PacketType::UpdateClientState);

	if (!m_lobby)
	{
		update_client_state_packet << static_cast<opt::PlayerCount>(m_player_count);

		for (const auto& player : m_player_info)
		{
			update_client_state_packet << player.first << player.second.m_position.x << player.second.m_position.y;
		}
	}

	SendToAll(update_client_state_packet);
}

void GameServer::UpdateDangers(sf::Time dt)
{
	sf::Packet packet;
	packet << static_cast<opt::ServerPacket>(Server::PacketType::UpdateDangerTime)
		<< dt.asSeconds();

	SendToAll(packet);
}

bool GameServer::PlayerCanAttack(opt::PlayerIdentifier identifier)
{
	return true;
}

void GameServer::PlayerAttack(opt::PlayerIdentifier identifier)
{
	sf::FloatRect attack(m_player_info[identifier].m_position, sf::Vector2f(20, 20));
}

bool GameServer::IsPlayerUnderWorld(opt::PlayerIdentifier identifier)
{
	return m_player_info[identifier].m_position.y > WorldInfo::WORLD_HEIGHT;
}
