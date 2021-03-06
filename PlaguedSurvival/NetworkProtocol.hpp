#pragma once
#include <SFML/System/Vector2.hpp>

/**
 * Vilandas Morrissey - D00218436
 */

const unsigned short SERVER_PORT = 50000;

namespace Server
{
	//These are packets that come from the Server
	enum class PacketType
	{
		BroadcastMessage,
		InitialState,
		StartGame,
		PlayerEvent,
		PlayerRealtimeChange,
		PlayerConnect,
		PlayerDisconnect,
		AcceptCoopPartner,
		SpawnSelf,
		UpdateClientState,
		UpdateDangerTime,
		GamesWonUpdated,
		PlayerDied,
		MissionSuccess
	};
}

namespace Client
{
	//Messages sent from the Client
	enum class PacketType
	{
		StillHereUpdate,
		RequestStartGame,
		PlayerEvent,
		PlayerRealtimeChange,
		RequestCoopPartner,
		PositionUpdate,
		GameEvent,
		UpdateGamesWon,
		Quit
	};
}

namespace GameActions
{
	enum Type
	{
		EnemyExplode
	};

	struct Action
	{
		Action()
		{
		}

		Action(Type type, sf::Vector2f position) :type(type), position(position)
		{
		}

		Type type;
		sf::Vector2f position;
	};
}