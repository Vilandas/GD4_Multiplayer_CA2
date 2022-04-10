#pragma once
#include <SFML/Graphics/Color.hpp>

/**
 * Vilandas Morrissey - D00218436
 */

enum class PlayerColors
{
	kWhite,
	kRed,
	kGreen,
	kBlue,
	kMagenta,
	kYellow,
	kCyan,
	kOrange,
	kApricot,
	kDeepPink,
	kPurple,
	kBrown,
	kMint,
	kDarkGreen,
	kTeal,
};

namespace ExtraColors
{
	static const sf::Color Orange(245, 130, 48);
	static const sf::Color Teal(0, 128, 128);
	static const sf::Color Purple(145, 30, 180);
	static const sf::Color DeepPink(255, 20, 147);
	static const sf::Color Apricot(255, 215, 180);
	static const sf::Color DarkGreen(0, 100, 0);
	static const sf::Color Brown(170, 110, 40);
	static const sf::Color Mint(170, 255, 195);

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
		case PlayerColors::kPurple: return Purple;
		case PlayerColors::kDeepPink: return DeepPink;
		case PlayerColors::kApricot: return Apricot;
		case PlayerColors::kDarkGreen: return DarkGreen;
		case PlayerColors::kBrown: return Brown;
		case PlayerColors::kMint: return Mint;
		}

		return sf::Color::White;
	}
}