#include "Label.hpp"

#include "ResourceHolder.hpp"
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "Utility.hpp"

namespace GUI
{
	Label::Label(const std::string& text, const FontHolder& fonts)
	: m_text(text, fonts.Get(Fonts::Main), 16)
	{
	}

	bool Label::IsSelectable() const
	{
		return false;
	}

	std::string Label::GetText() const
	{
		return m_text.getString();
	}

	void Label::SetText(const std::string& text)
	{
		m_text.setString(text);
	}

	void Label::SetFillColor(sf::Color color)
	{
		m_text.setFillColor(color);
	}

	void Label::SetCharacterSize(unsigned int size)
	{
		m_text.setCharacterSize(size);
	}

	void Label::CentreOriginText()
	{
		Utility::CentreOrigin(m_text);
	}

	void Label::HandleEvent(const sf::Event& event)
	{
	}

	void Label::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();
		target.draw(m_text, states);
	}
}

