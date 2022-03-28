#include "Component.hpp"
namespace GUI
{
	Component::Component()
	: m_parent(nullptr)
	, m_is_selected(false)
	, m_is_active(false)
	{
	}

	void Component::SetParent(Component* component)
	{
		m_parent = component;
	}

	sf::FloatRect Component::GetBoundingRect() const
	{
		return sf::FloatRect();
	}

	sf::Vector2f Component::GetWorldPosition() const
	{
		return GetWorldTransform() * sf::Vector2f();
	}

	sf::Transform Component::GetWorldTransform() const
	{
		sf::Transform transform = sf::Transform::Identity;

		for (const Component* component = this; component != nullptr; component = component->m_parent)
		{
			transform = component->getTransform() * transform;
		}

		return transform;
	}

	bool Component::IsSelected() const
	{
		return m_is_selected;
	}

	void Component::Select()
	{
		m_is_selected = true;
	}

	void Component::Deselect()
	{
		m_is_selected = false;
	}

	bool Component::IsActive() const
	{
		return m_is_active;
	}

	void Component::Activate()
	{
		m_is_active = true;
	}

	void Component::Deactivate()
	{
		m_is_active = false;
	}

}
