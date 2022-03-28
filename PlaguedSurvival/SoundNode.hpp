#pragma once
#include "SceneNode.hpp"
#include "ResourceIdentifiers.hpp"

class SoundPlayer;

class SoundNode : public SceneNode
{
public:
	explicit SoundNode(const SceneLayers& scene_layers, SoundPlayer& player);
	void PlaySound(SoundEffect sound, sf::Vector2f position);

	virtual unsigned int GetCategory() const override;

private:
	SoundPlayer& m_sounds;
};