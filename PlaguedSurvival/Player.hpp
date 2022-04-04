#pragma once
#include "Command.hpp"
#include "KeyBinding.hpp"
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Window/Event.hpp>
#include <map>
#include <unordered_set>

#include "CommandQueue.hpp"
#include "MissionStatus.hpp"
#include "NetworkOptimisations.hpp"
#include "PlayerAction.hpp"

class Player
{
public:
	Player(sf::TcpSocket* socket, opt::PlayerIdentifier identifier, const KeyBinding* binding);
	void HandleEvent(const sf::Event& event, CommandQueue& commands);
	void HandleRealtimeInput(CommandQueue& commands);
	void HandleRealtimeNetworkInput(CommandQueue& commands);

	//React to events or realtime state changes recevied over the network
	void HandleNetworkEvent(PlayerAction action, CommandQueue& commands);
	void HandleNetworkRealtimeChange(PlayerAction action, bool action_enabled);

	void SetMissionStatus(MissionStatus status);
	MissionStatus GetMissionStatus() const;

	void DisableAllRealtimeActions();
	bool IsLocal() const;

private:
	void InitialiseActions();

private:
	const KeyBinding* m_key_binding;
	std::map<PlayerAction, Command> m_action_binding;
	std::map<PlayerAction, Command> m_on_release_action_binding;
	std::map<PlayerAction, bool> m_action_proxies;
	std::unordered_set<PlayerAction> m_active_actions;
	MissionStatus m_current_mission_status;
	opt::PlayerIdentifier m_identifier;
	sf::TcpSocket* m_socket;
};