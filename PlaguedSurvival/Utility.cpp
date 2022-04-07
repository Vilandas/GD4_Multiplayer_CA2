#define _USE_MATH_DEFINES
#include "Utility.hpp"

#include <cassert>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include <cmath>
#include <random>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Animation.hpp"

namespace
{
	unsigned long GenerateSeed()
	{
		return static_cast<unsigned long>(std::time(nullptr));
	}

	std::default_random_engine CreateRandomEngine(const unsigned long seed)
	{
		return std::default_random_engine(seed);
	}

	unsigned long Seed = GenerateSeed();
	auto RandomEngine = CreateRandomEngine(Seed);
}

//TODO should we just implement for base class - sf::Transformable?
void Utility::CentreOrigin(sf::Sprite& sprite)
{
	const sf::FloatRect bounds = sprite.getLocalBounds();
	sprite.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void Utility::CentreOrigin(sf::RectangleShape& shape)
{
	const sf::FloatRect bounds = shape.getLocalBounds();
	shape.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void Utility::CentreOrigin(sf::Text& text)
{
	const sf::FloatRect bounds = text.getLocalBounds();
	text.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

void Utility::CentreOrigin(Animation& animation)
{
	const sf::FloatRect bounds = animation.GetLocalBounds();
	animation.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

std::string Utility::toString(sf::Keyboard::Key key)
{
#define KEYTOSTRING_CASE(KEY) case sf::Keyboard::KEY: return #KEY;

	switch (key)
	{
		KEYTOSTRING_CASE(Unknown)
			KEYTOSTRING_CASE(A)
			KEYTOSTRING_CASE(B)
			KEYTOSTRING_CASE(C)
			KEYTOSTRING_CASE(D)
			KEYTOSTRING_CASE(E)
			KEYTOSTRING_CASE(F)
			KEYTOSTRING_CASE(G)
			KEYTOSTRING_CASE(H)
			KEYTOSTRING_CASE(I)
			KEYTOSTRING_CASE(J)
			KEYTOSTRING_CASE(K)
			KEYTOSTRING_CASE(L)
			KEYTOSTRING_CASE(M)
			KEYTOSTRING_CASE(N)
			KEYTOSTRING_CASE(O)
			KEYTOSTRING_CASE(P)
			KEYTOSTRING_CASE(Q)
			KEYTOSTRING_CASE(R)
			KEYTOSTRING_CASE(S)
			KEYTOSTRING_CASE(T)
			KEYTOSTRING_CASE(U)
			KEYTOSTRING_CASE(V)
			KEYTOSTRING_CASE(W)
			KEYTOSTRING_CASE(X)
			KEYTOSTRING_CASE(Y)
			KEYTOSTRING_CASE(Z)
			KEYTOSTRING_CASE(Num0)
			KEYTOSTRING_CASE(Num1)
			KEYTOSTRING_CASE(Num2)
			KEYTOSTRING_CASE(Num3)
			KEYTOSTRING_CASE(Num4)
			KEYTOSTRING_CASE(Num5)
			KEYTOSTRING_CASE(Num6)
			KEYTOSTRING_CASE(Num7)
			KEYTOSTRING_CASE(Num8)
			KEYTOSTRING_CASE(Num9)
			KEYTOSTRING_CASE(Escape)
			KEYTOSTRING_CASE(LControl)
			KEYTOSTRING_CASE(LShift)
			KEYTOSTRING_CASE(LAlt)
			KEYTOSTRING_CASE(LSystem)
			KEYTOSTRING_CASE(RControl)
			KEYTOSTRING_CASE(RShift)
			KEYTOSTRING_CASE(RAlt)
			KEYTOSTRING_CASE(RSystem)
			KEYTOSTRING_CASE(Menu)
			KEYTOSTRING_CASE(LBracket)
			KEYTOSTRING_CASE(RBracket)
			KEYTOSTRING_CASE(SemiColon)
			KEYTOSTRING_CASE(Comma)
			KEYTOSTRING_CASE(Period)
			KEYTOSTRING_CASE(Quote)
			KEYTOSTRING_CASE(Slash)
			KEYTOSTRING_CASE(BackSlash)
			KEYTOSTRING_CASE(Tilde)
			KEYTOSTRING_CASE(Equal)
			KEYTOSTRING_CASE(Dash)
			KEYTOSTRING_CASE(Space)
			KEYTOSTRING_CASE(Return)
			KEYTOSTRING_CASE(BackSpace)
			KEYTOSTRING_CASE(Tab)
			KEYTOSTRING_CASE(PageUp)
			KEYTOSTRING_CASE(PageDown)
			KEYTOSTRING_CASE(End)
			KEYTOSTRING_CASE(Home)
			KEYTOSTRING_CASE(Insert)
			KEYTOSTRING_CASE(Delete)
			KEYTOSTRING_CASE(Add)
			KEYTOSTRING_CASE(Subtract)
			KEYTOSTRING_CASE(Multiply)
			KEYTOSTRING_CASE(Divide)
			KEYTOSTRING_CASE(Left)
			KEYTOSTRING_CASE(Right)
			KEYTOSTRING_CASE(Up)
			KEYTOSTRING_CASE(Down)
			KEYTOSTRING_CASE(Numpad0)
			KEYTOSTRING_CASE(Numpad1)
			KEYTOSTRING_CASE(Numpad2)
			KEYTOSTRING_CASE(Numpad3)
			KEYTOSTRING_CASE(Numpad4)
			KEYTOSTRING_CASE(Numpad5)
			KEYTOSTRING_CASE(Numpad6)
			KEYTOSTRING_CASE(Numpad7)
			KEYTOSTRING_CASE(Numpad8)
			KEYTOSTRING_CASE(Numpad9)
			KEYTOSTRING_CASE(F1)
			KEYTOSTRING_CASE(F2)
			KEYTOSTRING_CASE(F3)
			KEYTOSTRING_CASE(F4)
			KEYTOSTRING_CASE(F5)
			KEYTOSTRING_CASE(F6)
			KEYTOSTRING_CASE(F7)
			KEYTOSTRING_CASE(F8)
			KEYTOSTRING_CASE(F9)
			KEYTOSTRING_CASE(F10)
			KEYTOSTRING_CASE(F11)
			KEYTOSTRING_CASE(F12)
			KEYTOSTRING_CASE(F13)
			KEYTOSTRING_CASE(F14)
			KEYTOSTRING_CASE(F15)
			KEYTOSTRING_CASE(Pause)
	}

	return "";
}

double Utility::ToRadians(int degrees)
{
	return (degrees * M_PI) / 180;
}

sf::Vector2f Utility::UnitVector(sf::Vector2f vector)
{
	if (vector == sf::Vector2f(0.f, 0.f))
	{
		return { 0, 0 };
	}

	return vector / Length(vector);
}

sf::Vector2f Utility::UnitVector(sf::Vector2i vector)
{
	if (vector == sf::Vector2i(0, 0))
	{
		return { 0, 0 };
	}

	const float length = Length(vector);

	return {vector.x / length, vector.y / length};
}

float Utility::Length(sf::Vector2f vector)
{
	return sqrtf(powf(vector.x, 2) + powf(vector.y, 2));
}

float Utility::Length(sf::Vector2i vector)
{
	return sqrtf(pow(vector.x, 2) + powf(vector.y, 2));
}

float Utility::ToDegrees(float angle_in_radians)
{
	return angle_in_radians * (180 / M_PI);
}

sf::IntRect Utility::GetIntRect(const sf::Texture& texture)
{
	return {0, 0, static_cast<int>(texture.getSize().x), static_cast<int>(texture.getSize().y)};
}

sf::Vector2f Utility::GetRectCenter(const sf::FloatRect& rect)
{
	return { rect.left + rect.width / 2, rect.top + rect.height / 2 };
}

int Utility::RandomInt(int exclusive_max)
{
	const std::uniform_int_distribution<> distr(0, exclusive_max - 1);
	return distr(RandomEngine);
}

unsigned long Utility::GetSeed()
{
	return Seed;
}

void Utility::UpdateRandomEngine(unsigned long seed)
{
	RandomEngine = std::default_random_engine(seed);
}
