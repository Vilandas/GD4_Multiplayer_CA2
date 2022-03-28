#include "Collision.hpp"

#include "Utility.hpp"

/// <summary>
/// Tests for an intersection between two SceneNodes. Predicts if two nodes will collide.
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <returns>True if nodes a & b intersect</returns>
bool Collision::Intersects(SceneNode& a, SceneNode& b)
{
	const sf::FloatRect a_bounds = PredictMove(a);
	const sf::FloatRect b_bounds = PredictMove(b);

	return a_bounds.intersects(b_bounds);
}

/// <summary>
/// Tests for the intersection location between two SceneNodes. Predicts if two nodes will collide.
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <returns>CollisionLocation/returns>
CollisionLocation Collision::CollisionLocation(SceneNode& a, SceneNode& b)
{
	const sf::FloatRect a_bounds = PredictMove(a);
	const sf::FloatRect b_bounds = PredictMove(b);

	if (a_bounds.intersects(b_bounds))
	{
		const float total_half_width = (a_bounds.width + b_bounds.width) / 2;
		const float total_half_height = (a_bounds.height + b_bounds.height) / 2;

		// distance from centre
		const sf::Vector2f distance = Utility::GetRectCenter(a_bounds) - Utility::GetRectCenter(b_bounds);

		const sf::Vector2f o = sf::Vector2f(
			std::ceil(total_half_width - std::abs(distance.x)),
			std::ceil(total_half_height - std::abs(distance.y))
		);

		if (o.x >= o.y)
		{
			return distance.y > 0 ? CollisionLocation::kTop : CollisionLocation::kBottom;
		}
		return distance.x > 0 ? CollisionLocation::kLeft : CollisionLocation::kRight;
	}

	return CollisionLocation::kNone;
}

sf::FloatRect Collision::PredictMove(const sf::FloatRect& bounds, const sf::Vector2f& velocity)
{
	return {
		bounds.left + velocity.x,
		bounds.top + velocity.y,
		bounds.width,
		bounds.height
	};
}

sf::FloatRect Collision::PredictMove(const SceneNode& node)
{
	const sf::FloatRect bounds = node.GetBoundingRect();
	const sf::Vector2f velocity = node.GetVelocity() * node.GetDeltaTimeInSeconds();

	return PredictMove(bounds, velocity);
}

