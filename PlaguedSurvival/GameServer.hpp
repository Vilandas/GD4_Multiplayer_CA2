#pragma once
#include <map>
#include <memory>
#include <string>
#include <SFML/Config.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Thread.hpp>

#include "NetworkOptimisations.hpp"

/**
 * Vilandas Morrissey - D00218436
 */

class GameServer
{
public:
	static constexpr int NAME_SIZE = 12;

public:
	explicit GameServer();
	~GameServer();
	void NotifyPlayerSpawn(opt::PlayerIdentifier player_identifier);
	void NotifyPlayerRealtimeChange(opt::PlayerIdentifier player_identifier, opt::Action action, bool action_enabled);
	void NotifyPlayerEvent(opt::PlayerIdentifier player_identifier, opt::Action action);

private:
	struct RemotePeer
	{
		RemotePeer();
		sf::TcpSocket m_socket;
		sf::Time m_last_packet_time;
		std::vector<opt::PlayerIdentifier> m_player_identifiers;
		bool m_ready;
		bool m_timed_out;
	};

	struct PlayerInfo
	{
		std::string m_name;
		sf::Vector2f m_position;
		sf::Int32 m_hitpoints;
		opt::GamesWon m_games_won;
		std::map<opt::Action, bool> m_realtime_actions;
	};

	typedef std::unique_ptr<RemotePeer> PeerPtr;

private:
	void SetListening(bool enable);
	void ExecutionThread();
	void Tick();
	opt::PlayerIdentifier FindWinnerIdentity() const;
	sf::Time Now() const;

	void HandleIncomingPackets();
	void HandleIncomingPacket(sf::Packet& packet, RemotePeer& receiving_peer, bool& detected_timeout);

	opt::PlayerIdentifier GetFreeIdentifier() const;
	void HandleIncomingConnections();
	void HandleDisconnections();

	void InformWorldState(sf::TcpSocket& socket);
	void BroadcastMessage(const std::string& message);
	void SendToAll(sf::Packet& packet);
	void UpdateClientState();
	void UpdateDangers(sf::Time dt);

	bool PlayerCanAttack(opt::PlayerIdentifier identifier);
	void PlayerAttack(opt::PlayerIdentifier identifier);

	bool IsPlayerUnderWorld(opt::PlayerIdentifier identifier);

private:
	sf::Thread m_thread;
	sf::Clock m_clock;
	sf::TcpListener m_listener_socket;
	bool m_listening_state;
	bool m_lobby;
	sf::Time m_client_timeout;

	std::size_t m_max_connected_players;
	std::size_t m_connected_players;

	opt::PlayerCount m_player_count;
	std::map<opt::PlayerIdentifier, PlayerInfo> m_player_info;

	std::vector<PeerPtr> m_peers;
	bool m_waiting_thread_end;

	int m_alive_players;
};
