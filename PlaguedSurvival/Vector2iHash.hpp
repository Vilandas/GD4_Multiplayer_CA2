#pragma once
#include <memory>
#include <SFML/System/Vector2.hpp>

/// <summary>
/// A hash function used to hash a vector2 of integers.
///	Implementation modified from boost hash_combine
/// </summary>
///	<see cref="https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x"/>
struct Vector2iHash {
	size_t operator()(const sf::Vector2i vector) const
	{
		const size_t hash1 = std::hash<int>{}(vector.x);
		const size_t hash2 = std::hash<int>{}(vector.y);

		return hash1 ^ hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2);
	}
};