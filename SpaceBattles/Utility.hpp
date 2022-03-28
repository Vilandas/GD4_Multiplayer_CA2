#pragma once
#include <string>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Animation.hpp"

namespace sf
{
	class Sprite;
	class Text;
}

class Utility
{
public:
	static void CentreOrigin(sf::Sprite& sprite);
	static void CentreOrigin(sf::Text& text);
	static void CentreOrigin(Animation& animation);
	static std::string toString(sf::Keyboard::Key key);
	static double ToRadians(int degrees);
	static sf::Vector2f UnitVector(sf::Vector2f vector);
	static sf::Vector2f UnitVector(sf::Vector2i vector);
	static float Length(sf::Vector2f vector);
	static float Length(sf::Vector2i vector);
	static float ToDegrees(float angle);
	static sf::IntRect GetIntRect(const sf::Texture& texture);
	static sf::Vector2f GetRectCenter(const sf::FloatRect& rect);
	static int RandomInt(int exclusive_max);
};