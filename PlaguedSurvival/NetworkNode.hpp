#pragma once
#include "SceneNode.hpp"
#include "NetworkProtocol.hpp"

#include <queue>

class NetworkNode : public SceneNode
{
public:
	NetworkNode(const SceneLayers& scene_layers);
	void NotifyGameAction(GameActions::Type type, sf::Vector2f position);
	bool PollGameAction(GameActions::Action& out);

	unsigned int GetCategory() const override;

private:
	std::queue<GameActions::Action> m_pending_actions;
};