#include "GameState.hpp"

#include <iostream>

#include "DangerTrigger.hpp"
#include "Player.hpp"

GameState::GameState(StateStack& stack, Context context)
: State(stack, context)
, m_world(*context.window, *context.textures, *context.fonts, *context.sounds, *context.camera, false)
, m_player(nullptr, 1, context.keys1)
{
	m_world.AddPlayer(1, "", true);
	m_player.SetMissionStatus(MissionStatus::kMissionRunning);
	// Play game theme
	context.music->Play(MusicThemes::kMenuTheme);
}

void GameState::Draw()
{
	m_world.Draw();
}

bool GameState::Update(sf::Time dt)
{
	m_world.Update(dt);

	DangerTrigger::Instance().Update(dt);

	if (m_world.HasPlayerAchievedVictory())
	{
		m_player.SetMissionStatus(MissionStatus::kMissionSuccess);
		RequestStackPush(StateID::kGameOver);
	}

	CommandQueue& commands = m_world.GetCommandQueue();
	m_player.HandleRealtimeInput(commands);
	return true;
}

bool GameState::HandleEvent(const sf::Event& event)
{
	CommandQueue& commands = m_world.GetCommandQueue();
	m_player.HandleEvent(event, commands);

	//Escape should bring up the Pause Menu
	if(event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
	{
		RequestStackPush(StateID::kPause);
	}
	return true;
}
