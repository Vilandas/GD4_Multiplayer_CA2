#pragma once
#include "Container.hpp"
#include "State.hpp"
#include "World.hpp"
#include "Player.hpp"
#include "GameServer.hpp"
#include "Label.hpp"
#include "NetworkProtocol.hpp"

class LobbyState : public State
{
public:
	LobbyState(StateStack& stack, Context context, bool is_host);
	void Draw() override;
	bool Update(sf::Time dt) override;
	bool HandleEvent(const sf::Event& event) override;
	void OnActivate() override;
	void OnDestroy() override;

private:
	struct Player
	{
		std::shared_ptr<GUI::Label> m_name;
	};

private:
	void UpdateBroadcastMessage(sf::Time elapsed_time);
	void HandlePacket(opt::ServerPacket packet_type, sf::Packet& packet);
	void GeneratePlayer(opt::PlayerIdentifier identifier);
	void GeneratePlayer(opt::PlayerIdentifier identifier, const std::string& name);

private:
	sf::RenderWindow& m_window;
	TextureHolder& m_texture_holder;
	FontHolder& m_font_holder;

	Camera m_camera;
	GUI::Container m_gui_container;
	sf::Sprite m_background_sprite;

	std::map<opt::PlayerIdentifier, Player> m_players;
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
	sf::Time m_client_timeout;
	sf::Time m_time_since_last_packet;
};