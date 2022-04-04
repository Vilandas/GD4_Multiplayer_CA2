#include "GameServer.hpp"

#include <iostream>

#include "NetworkProtocol.hpp"
#include <SFML/System.hpp>

#include <SFML/Network/Packet.hpp>

#include "PickupType.hpp"
#include "Utility.hpp"

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
	, m_last_spawn_time(sf::Time::Zero)
	, m_time_for_next_spawn(sf::seconds(5.f))
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
	sf::Clock frame_clock, tick_clock;

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

		//sleep
		sf::sleep(sf::milliseconds(100));
	}
}

void GameServer::Tick()
{
	UpdateClientState();

	//Check if the game is over = all planes position.y < offset
	//bool all_aircraft_done = true;
	//for (const auto& current : m_aircraft_info)
	//{
	//	//As long one player has not crossed the finish line game on
	//	if (current.second.m_position.y > 0.f)
	//	{
	//		all_aircraft_done = false;
	//	}
	//}

	//if (all_aircraft_done)
	//{
	//	sf::Packet mission_success_packet;
	//	mission_success_packet << static_cast<sf::Int32>(Server::PacketType::MissionSuccess);
	//	SendToAll(mission_success_packet);
	//}

	//Remove aircraft that have been destroyed
	//for (auto itr = m_aircraft_info.begin(); itr != m_aircraft_info.end();)
	//{
	//	if (itr->second.m_hitpoints <= 0)
	//	{
	//		m_aircraft_info.erase(itr++);
	//	}
	//	else
	//	{
	//		++itr;
	//	}
	//}

	//Check if it is time to spawn enemies
	//if (Now() >= m_time_for_next_spawn + m_last_spawn_time)
	//{
	//	//Not going to spawn enemies near the end
	//	if (m_battlefield_rect.top > 600.f)
	//	{
	//		std::size_t enemy_count = 1 + Utility::RandomInt(2);
	//		float spawn_centre = static_cast<float>(Utility::RandomInt(500) - 250);

	//		//If there is only one enemy it is at the spawn_centre
	//		float plane_distance = 0.f;
	//		float next_spawn_position = spawn_centre;

	//		//If there are two then they are centred on the spawn centre
	//		if (enemy_count == 2)
	//		{
	//			plane_distance = static_cast<float>(150 + Utility::RandomInt(250));
	//			next_spawn_position = spawn_centre - plane_distance / 2.f;
	//		}

	//		//TODO Do we really need two packets here?
	//		//Send a spawn packet to the clients
	//		for (std::size_t i = 0; i < enemy_count; ++i)
	//		{
	//			sf::Packet packet;
	//			packet << static_cast<sf::Int32>(Server::PacketType::SpawnEnemy);
	//			packet << static_cast<sf::Int32>(1 + Utility::RandomInt(static_cast<int>(AircraftType::kAircraftCount) - 1));
	//			packet << m_world_height - m_battlefield_rect.top + 500;
	//			packet << next_spawn_position;

	//			next_spawn_position += plane_distance / 2.f;
	//			SendToAll(packet);
	//		}

	//		m_last_spawn_time = Now();
	//		m_time_for_next_spawn = sf::milliseconds(2000 + Utility::RandomInt(6000));
	//	}
	//}
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

	case Client::PacketType::PlayerEvent:
	{
		opt::PlayerIdentifier player_identifier;
		opt::Action action;
		packet >> player_identifier >> action;
		NotifyPlayerEvent(player_identifier, action);
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
		opt::PlayerIdentifier identifier = GetFreeIdentifier();
		receiving_peer.m_player_identifiers.emplace_back(identifier);

		m_player_info[identifier].name = std::to_string(identifier);
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

		//Enemy explodes, with a certain probability, drop a pickup
		//To avoid multiple messages only listen to the first peer (host)
		if (action == GameActions::EnemyExplode && Utility::RandomInt(3) == 0 && &receiving_peer == m_peers[0].get())
		{
			sf::Packet packet;
			packet << static_cast<opt::ServerPacket>(Server::PacketType::SpawnPickup);
			packet << static_cast<sf::Int32>(Utility::RandomInt(static_cast<int>(PickupType::kPickupCount)));
			packet << x;
			packet << y;

			SendToAll(packet);
		}
	}
	break;

	case Client::PacketType::RequestStartGame:
	{
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
		const opt::PlayerIdentifier identifier = GetFreeIdentifier();

		//Order the new client to spawn its player 1
		m_player_info[identifier].name = std::to_string(identifier);
		m_player_info[identifier].m_position = sf::Vector2f(0, 0);
		m_player_info[identifier].m_hitpoints = 100;


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
					<< m_player_info[identifier].name;
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
