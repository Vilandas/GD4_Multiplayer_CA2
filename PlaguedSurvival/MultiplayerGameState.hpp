#pragma once
#include <iostream>
#include <fstream>

#include "Container.hpp"
#include "Button.hpp"
#include "State.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "GameServer.hpp"
#include "Label.hpp"
#include "NetworkProtocol.hpp"

/**
 * Vilandas Morrissey - D00218436
 */

class MultiplayerGameState : public State
{
public:
	MultiplayerGameState(StateStack& stack, Context context, bool is_host);
	void Draw() override;
	bool Update(sf::Time dt) override;
	void UpdateStatistics(sf::Time dt);
	void UpdateLobby(sf::Time dt);
	void UpdateGame(sf::Time dt);
	void ReceivePacket();
	void SendPacket(sf::Packet& packet);

	bool HandleEvent(const sf::Event& event) override;
	void DisableAllRealtimeActions();
	void OnActivate() override;
	void OnDestroy() override;

private:
	typedef std::unique_ptr<Player> PlayerPtr;

	struct PlayerData
	{
		std::shared_ptr<GUI::Label> m_name;
		PlayerPtr m_player;
		opt::GamesWon m_games_won;
	};

private:
	void UpdateBroadcastMessage(sf::Time elapsed_time);
	void HandlePacket(opt::ServerPacket packet_type, sf::Packet& packet);
	void GeneratePlayer(opt::PlayerIdentifier identifier);
	void GeneratePlayer(opt::PlayerIdentifier identifier, const std::string& name);
	void SaveData() const;

private:
	World m_world;
	sf::RenderWindow& m_window;
	TextureHolder& m_texture_holder;
	FontHolder& m_font_holder;
	MusicPlayer& m_music;

	Camera m_camera;
	GUI::Container m_lobby_gui;
	sf::Sprite m_background_sprite;
	sf::Text m_lobby_text;
	sf::Text m_waiting_for_host_text;

	sf::Text m_statistics_text;
	sf::Time m_statistics_update_time;
	sf::Uint32 m_bytes_received;
	sf::Uint32 m_bytes_sent;

	opt::GamesWon m_games_won;
	std::map<opt::PlayerIdentifier, PlayerData> m_players;
	std::vector<opt::PlayerIdentifier> m_local_player_identifiers;
	sf::TcpSocket m_socket;
	bool m_connected;
	std::unique_ptr<GameServer> m_game_server;
	sf::Clock m_tick_clock;

	std::vector<std::string> m_broadcasts;
	sf::Text m_broadcast_text;
	sf::Time m_broadcast_elapsed_time;

	sf::Text m_player_invitation_text;
	sf::Time m_player_invitation_time;

	sf::Text m_failed_connection_text;
	sf::Clock m_failed_connection_clock;

	bool m_active_state;
	bool m_has_focus;
	bool m_host;
	bool m_lobby;
	sf::Time m_client_timeout;
	sf::Time m_time_since_last_packet;
};