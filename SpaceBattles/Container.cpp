#include "Container.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

namespace GUI
{
	Container::Container(sf::RenderWindow& window, Camera& camera)
		: m_window(window)
		, m_camera(camera)
		, m_selected_child(-1)
	{
	}

	void Container::DeactivateAllExcept(const Component::Ptr& exception) const
	{
		for (const auto& child : m_children)
		{
			if (child != exception)
			{
				child->Deactivate();
			}
		}
	}

	void Container::DeactivateAll() const
	{
		for (const auto& child : m_children)
		{
			child->Deactivate();
		}
	}

	bool Container::Pack(const Component::Ptr& component)
	{
		component->SetParent(this);
		m_children.emplace_back(component);
		if (!HasSelection() && component->IsSelectable())
		{
			Select(m_children.size() - 1);
		}

		return true;
	}

	bool Container::IsSelectable() const
	{
		return false;
	}

	void Container::HandleEvent(const sf::Event& event)
	{
		if (HasSelection() && m_children[m_selected_child]->IsActive())
		{
			m_children[m_selected_child]->HandleEvent(event);
		}

		if (event.type == sf::Event::MouseMoved)
		{
			for (size_t i = 0; i < m_children.size(); i++)
			{
				const sf::Vector2f mouse = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_camera.GetView());
				const auto child = m_children[i];
				if (child->GetBoundingRect().contains(mouse.x, mouse.y))
				{
					Select(i);
				}
			}
		}
		else if (event.type == sf::Event::MouseButtonReleased)
		{
			if (event.mouseButton.button == sf::Mouse::Left && HasSelection())
			{
				const sf::Vector2f mouse = m_window.mapPixelToCoords(sf::Mouse::getPosition(m_window), m_camera.GetView());
				if (m_children[m_selected_child]->GetBoundingRect().contains(mouse.x, mouse.y))
				{
					m_children[m_selected_child]->Activate();
				}
			}
		}
		else if (event.type == sf::Event::KeyReleased)
		{
			if (event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::Up)
			{
				SelectPrevious();
			}
			else if (event.key.code == sf::Keyboard::S || event.key.code == sf::Keyboard::Down)
			{
				SelectNext();
			}
			else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space)
			{
				if (HasSelection())
				{
					m_children[m_selected_child]->Activate();
				}
			}
		}
	}

	void Container::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();
		for (const Component::Ptr& child : m_children)
		{
			target.draw(*child, states);
		}

	}

	bool Container::HasSelection() const
	{
		return m_selected_child >= 0;
	}

	void Container::Select(std::size_t index)
	{
		if (index < m_children.size() && m_children[index]->IsSelectable())
		{
			if (HasSelection())
			{
				m_children[m_selected_child]->Deselect();
			}
			m_children[index]->Select();
			m_selected_child = index;
		}
	}

	void Container::SelectNext()
	{
		if (!HasSelection())
		{
			return;
		}
		//Search for the next component that is selectable and wrap around if necessary
		int next = m_selected_child;
		do
		{
			next = (next + 1) % m_children.size();
		} while (!m_children[next]->IsSelectable());

		Select(next);
	}

	void Container::SelectPrevious()
	{
		if (!HasSelection())
		{
			return;
		}
		//Search for the next component that is selectable and wrap around if necessary
		int prev = m_selected_child;
		do
		{
			prev = (prev + m_children.size() - 1) % m_children.size();
		} while (!m_children[prev]->IsSelectable());

		Select(prev);
	}
}
