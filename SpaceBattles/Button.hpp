#pragma once
#include <functional>

#include "Component.hpp"
#include "ResourceIdentifiers.hpp"

#include <memory>
#include <string>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>

#include "ButtonType.hpp"
#include "SoundPlayer.hpp"
#include "State.hpp"

namespace GUI
{
	class Button : public Component
	{
	public:
		typedef std::shared_ptr<Button> Ptr;
		typedef std::function<void()> Callback;

	public:
		Button(State::Context context);
		void SetCallback(Callback callback);
		void SetText(const std::string& text);
		void SetToggle(bool flag);

		sf::FloatRect GetBoundingRect() const override;
		bool IsSelectable() const override;
		void Select() override;
		void Deselect() override;
		void Activate() override;
		void Deactivate() override;
		void HandleEvent(const sf::Event& event) override;

	private:
		void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
		void ChangeTexture(ButtonType buttonType);

	private:
		Callback m_callback;
		sf::Sprite m_sprite;
		sf::Text m_text;
		bool m_is_toggle;
		SoundPlayer& m_sounds;
	};
}