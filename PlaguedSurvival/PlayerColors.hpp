#pragma once
#include <SFML/Graphics/Color.hpp>

enum class PlayerColors
{
	kWhite,
	kBlue,
	kRed,
	kGreen,
	kMagenta,
	kYellow,
	kCyan,
	kOrange,
	kTeal,
	kMidnightBlue,
	kDeepPink,
	kTurquoise,
	kDarkGreen,
	kMaroon,
	kAquaMarine,
};

namespace ExtraColors
{
	static const sf::Color Orange(255, 215, 0);
	static const sf::Color Teal(0, 128, 128);
	static const sf::Color MidnightBlue(25, 25, 112);
	static const sf::Color DeepPink(255, 20, 147);
	static const sf::Color Turquoise(64, 224, 208);
	static const sf::Color DarkGreen(0, 100, 0);
	static const sf::Color Maroon(128, 0, 0);
	static const sf::Color AquaMarine(127, 255, 212);

	inline sf::Color GetColor(PlayerColors playerColor)
	{
		switch (playerColor)
		{
		case PlayerColors::kWhite: return sf::Color::White;
		case PlayerColors::kBlue: return sf::Color::Blue;
		case PlayerColors::kRed: return sf::Color::Red;
		case PlayerColors::kGreen: return sf::Color::Green;
		case PlayerColors::kMagenta: return sf::Color::Magenta;
		case PlayerColors::kYellow: return sf::Color::Yellow;
		case PlayerColors::kCyan: return sf::Color::Cyan;
		case PlayerColors::kOrange: return Orange;
		case PlayerColors::kTeal: return Teal;
		case PlayerColors::kMidnightBlue: return MidnightBlue;
		case PlayerColors::kDeepPink: return DeepPink;
		case PlayerColors::kTurquoise: return Turquoise;
		case PlayerColors::kDarkGreen: return DarkGreen;
		case PlayerColors::kMaroon: return Maroon;
		case PlayerColors::kAquaMarine: return AquaMarine;
		}

		return sf::Color::White;
	}
}