#include "SceneNode.hpp"

#include <algorithm>
#include <cassert>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include "Collision.hpp"
#include "Utility.hpp"

SceneNode::SceneNode(
	const SceneLayers& scene_layers,
	Category::Type category)
	: m_scene_layers(scene_layers)
	, m_children()
	, m_parent(nullptr)
	, m_default_category(category)
{
}

void SceneNode::AttachChild(Ptr child)
{
	child->m_parent = this;
	m_children.emplace_back(std::move(child));
}

SceneNode::Ptr SceneNode::DetachChild(const SceneNode& node)
{
	const auto found = std::find_if(m_children.begin(), m_children.end(), [&](Ptr& p) {return p.get() == &node; });
	assert(found != m_children.end());

	Ptr result = std::move(*found);
	result->m_parent = nullptr;
	m_children.erase(found);
	return result;
}

void SceneNode::Update(sf::Time dt, CommandQueue& commands)
{
	UpdateCurrent(dt, commands);
	UpdateChildren(dt, commands);
}

sf::Vector2f SceneNode::GetWorldPosition() const
{
	return GetWorldTransform() * sf::Vector2f();
}

sf::Transform SceneNode::GetWorldTransform() const
{
	sf::Transform transform = sf::Transform::Identity;
	for (const SceneNode* node = this; node != nullptr; node = node->m_parent)
	{
		transform = node->getTransform() * transform;
	}
	return transform;
}

const SceneNode::SceneLayers& SceneNode::GetSceneLayers() const
{
	return m_scene_layers;
}

void SceneNode::OnCommand(const Command& command, sf::Time dt)
{
	//Is this command for me?
	if (command.category & GetCategory())
	{
		command.action(*this, dt);
	}

	//Pass command on to children
	for (const Ptr& child : m_children)
	{
		child->OnCommand(command, dt);
	}
}

unsigned int SceneNode::GetCategory() const
{
	return static_cast<int>(m_default_category);
}

sf::FloatRect SceneNode::GetBoundingRect() const
{
	return {};
}

sf::Vector2f SceneNode::GetVelocity() const
{
	return {};
}

float SceneNode::GetDeltaTimeInSeconds() const
{
	return 1;
}

bool SceneNode::IsDestroyed() const
{
	return false;
}

bool SceneNode::IsMarkedForRemoval() const
{
	return IsDestroyed();
}

void SceneNode::PredictCollisionsWithScene(SceneNode& scene_graph, std::set<SceneNode*>& collisions)
{
	for (Ptr& child : scene_graph.m_children)
	{
		if (this != &*child && Collision::Intersects(*this, *child) && !IsDestroyed() && !child->IsDestroyed())
		{
			collisions.insert(&*child);
		}

		PredictCollisionsWithScene(*child, collisions);
	}
}

void SceneNode::PredictCollisionsWithChunks(Layers layer, std::set<SceneNode*>& collisions)
{
	WorldChunks::Instance().CheckCollision(this, layer, collisions);
}

void SceneNode::PredictCollisionWithNode(SceneNode& scene_node, std::set<SceneNode*>& collisions)
{
	if (this != &scene_node && Collision::Intersects(*this, scene_node) && !IsDestroyed() && !scene_node.IsDestroyed())
	{
		collisions.insert(&scene_node);
	}
}

void SceneNode::RemoveWrecks()
{
	auto wreck_field_begin = std::remove_if(m_children.begin(), m_children.end(), std::mem_fn(&SceneNode::IsMarkedForRemoval));
	m_children.erase(wreck_field_begin, m_children.end());
	std::for_each(m_children.begin(), m_children.end(), std::mem_fn(&SceneNode::RemoveWrecks));
}

void SceneNode::HandleCollisions()
{
	// Do Nothing
}

void SceneNode::UpdateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Do Nothing
}

void SceneNode::UpdateChildren(sf::Time dt, CommandQueue& commands)
{
	for (Ptr& child : m_children)
	{
		child->Update(dt, commands);
	}
}

void SceneNode::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	//Apply transform of the current node
	states.transform *= getTransform();

	//Draw the node and children with changed transform
	DrawCurrent(target, states);
	DrawChildren(target, states);
	sf::FloatRect rect = GetBoundingRect();

	//if (GetCategory() == static_cast<unsigned int>(Category::kPlatform)) return;
	//DrawBoundingRect(target, states, rect);
}

void SceneNode::DrawCurrent(sf::RenderTarget&, sf::RenderStates states) const
{
	//Do nothing by default
}

void SceneNode::DrawChildren(sf::RenderTarget& target, sf::RenderStates states) const
{
	int i = 0;
	int size = m_children.size();
	for (const Ptr& child : m_children)
	{
		child->draw(target, states);
		i++;
	}
}

void SceneNode::DrawBoundingRect(sf::RenderTarget& target, sf::RenderStates states, const sf::FloatRect& rect)
{
	sf::RectangleShape shape;
	shape.setPosition(sf::Vector2f(rect.left, rect.top));
	shape.setSize(sf::Vector2f(rect.width, rect.height));
	shape.setFillColor(sf::Color::Transparent);
	shape.setOutlineColor(sf::Color::Green);
	shape.setOutlineThickness(1.f);
	target.draw(shape);
}

bool Collision(const SceneNode& lhs, const SceneNode& rhs)
{
	return lhs.GetBoundingRect().intersects(rhs.GetBoundingRect());
}

float Distance(const SceneNode& lhs, const SceneNode& rhs)
{
	return Utility::Length(lhs.GetWorldPosition() - rhs.GetWorldPosition());
}