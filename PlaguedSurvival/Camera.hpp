#pragma once
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/View.hpp>

/**
 * Vilandas Morrissey - D00218436
 */

class Camera : public sf::Transformable
{
public:
	Camera(sf::View view);
	const sf::View& GetView() const;

	void SetBoundsConstraint(sf::FloatRect bounds_constraint);
	void SetCenter(sf::Vector2f position);
	void SetPosition(sf::Vector2f position);
	void SetSize(sf::Vector2f size);
	void SetSize(float width, float height);

	sf::Vector2f GetCenter() const;
	sf::Vector2f GetSize() const;
	sf::FloatRect GetBoundingRect() const;

private:
	sf::View m_view;
	sf::FloatRect m_bounds_constraint;
	bool m_has_constraint;
};