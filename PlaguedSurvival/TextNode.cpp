#include "TextNode.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

#include "ResourceHolder.hpp"
#include "Utility.hpp"

TextNode::TextNode(const SceneLayers& scene_layers, const FontHolder& fonts, const std::string& text)
	: SceneNode(scene_layers)
	, m_text(text, fonts.Get(Fonts::Main), 20)
{
}

void TextNode::SetString(const std::string& text)
{
	m_text.setString(text);
	Utility::CentreOrigin(m_text);
}

void TextNode::SetFillColor(sf::Color color)
{
	m_text.setFillColor(color);
}

void TextNode::DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(m_text, states);
}
