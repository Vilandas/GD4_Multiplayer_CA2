#include "Camera.hpp"

Camera::Camera(sf::View view)
	: m_view(view)
	, m_has_constraint(false)
{
	SetCenter(view.getCenter());
}

const sf::View& Camera::GetView() const
{
	return m_view;
}

void Camera::SetBoundsConstraint(sf::FloatRect bounds_constraint)
{
	m_bounds_constraint = bounds_constraint;
	m_has_constraint = true;
	SetCenter(GetCenter());
}

void Camera::SetCenter(sf::Vector2f position)
{
	const sf::Vector2f size = m_view.getSize();
	const float bounds_right = m_bounds_constraint.left + m_bounds_constraint.width;
	const float bounds_bottom = m_bounds_constraint.top + m_bounds_constraint.height;

	position -= size / 2.f;

	if (m_has_constraint)
	{
		if (position.x < m_bounds_constraint.left)
		{
			position.x = m_bounds_constraint.left;
		}
		else if (position.x + size.x > bounds_right)
		{
			position.x = bounds_right - size.x;
		}

		if (position.y < m_bounds_constraint.top)
		{
			position.y = m_bounds_constraint.top;
		}
		else if (position.y + size.y > bounds_bottom)
		{
			position.y = bounds_bottom - size.y;
		}
	}

	m_view.setCenter(position + size / 2.f);
	setPosition(position);
}

void Camera::SetPosition(sf::Vector2f position)
{
	m_view.setCenter(position + (m_view.getSize() / 2.f));
}

void Camera::SetSize(sf::Vector2f size)
{
	m_view.setSize(size);
	SetCenter(m_view.getCenter());
}

void Camera::SetSize(float width, float height)
{
	m_view.setSize(width, height);
	SetCenter(m_view.getCenter());
}

sf::Vector2f Camera::GetCenter() const
{
	return m_view.getCenter();
}

sf::Vector2f Camera::GetSize() const
{
	return m_view.getSize();
}

sf::FloatRect Camera::GetBoundingRect() const
{
	return { getPosition(), m_view.getSize() };
}