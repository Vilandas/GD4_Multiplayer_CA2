#pragma once
#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>

#include <vector>
#include <memory>
#include <set>

#include "Command.hpp"
#include "CommandQueue.hpp"
#include "Layers.hpp"
#include "Utility.hpp"

class SceneNode : public sf::Transformable, public sf::Drawable, private sf::NonCopyable
{
public:
	typedef  std::unique_ptr<SceneNode> Ptr;
	typedef std::pair<SceneNode*, SceneNode*> Pair;
	typedef std::array<SceneNode*, static_cast<int>(Layers::kLayerCount)> SceneLayers;

public:
	explicit SceneNode(
		const SceneLayers& scene_layers,
		Category::Type category = Category::kNone);

	void AttachChild(Ptr child);
	Ptr DetachChild(const SceneNode& node);

	void Update(sf::Time dt, CommandQueue& commands);

	sf::Vector2f GetWorldPosition() const;
	sf::Transform GetWorldTransform() const;
	const SceneLayers& GetSceneLayers() const;

	void OnCommand(const Command& command, sf::Time dt);
	virtual unsigned int GetCategory() const;
	virtual sf::FloatRect GetBoundingRect() const;
	virtual sf::Vector2f GetVelocity() const;
	virtual float GetDeltaTimeInSeconds() const;

	virtual bool IsDestroyed() const;
	virtual bool IsMarkedForRemoval() const;

	void PredictCollisionsWithScene(SceneNode& scene_graph, std::set<SceneNode*>& collisions);
	void RemoveWrecks();

protected:
	virtual void HandleCollisions();

private:
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands);
	void UpdateChildren(sf::Time dt, CommandQueue& commands);

	//Note draw is from sf::Drawable hence the name, lower case d
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual void DrawCurrent(sf::RenderTarget&, sf::RenderStates states) const;
	void DrawChildren(sf::RenderTarget& target, sf::RenderStates states) const;

	static void DrawBoundingRect(sf::RenderTarget& target, sf::RenderStates states, const sf::FloatRect& bounding_rect);

private:
	const SceneLayers& m_scene_layers;
	std::vector<Ptr> m_children;
	SceneNode* m_parent;
	Category::Type m_default_category;
};
bool Collision(const SceneNode& lhs, const SceneNode& rhs);
float Distance(const SceneNode& lhs, const SceneNode& rhs);