#pragma once
#include <string>
#include <SFML/Graphics/Text.hpp>

#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"

class TextNode : public SceneNode
{
public:
	explicit TextNode(const SceneLayers& scene_layers, const FontHolder& fonts, const std::string& text);
	void SetString(const std::string& text);
	void SetFillColor(sf::Color color);

private:
	virtual void DrawCurrent(sf::RenderTarget&, sf::RenderStates states) const override;

private:
	sf::Text m_text;
};