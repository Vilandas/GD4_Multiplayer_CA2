#include "GameOverState.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "Player.hpp"
#include "ResourceHolder.hpp"
#include "Utility.hpp"

GameOverState::GameOverState(StateStack& stack, Context context, const std::string& text)
	: State(stack, context)
	, m_game_over_text()
	, m_elapsed_time(sf::Time::Zero)
{
	const sf::Font& font = context.fonts->Get(Fonts::Main);
	const sf::Vector2f windowSize(context.window->getSize());

	const std::string chosen_text = context.game_winner->length() == 0
		? text
		: *context.game_winner + " Wins!";

	m_game_over_text.setFont(font);
	m_game_over_text.setString(chosen_text);

	m_game_over_text.setCharacterSize(70);
	Utility::CentreOrigin(m_game_over_text);
	m_game_over_text.setPosition(0.5f * windowSize.x, 0.4f * windowSize.y);

	*context.game_winner = "";
}

void GameOverState::Draw()
{
	sf::RenderWindow& window = *GetContext().window;
	window.setView(window.getDefaultView());

	// Create dark, semitransparent background
	sf::RectangleShape backgroundShape;
	backgroundShape.setFillColor(sf::Color(0, 0, 0, 150));
	backgroundShape.setSize(window.getView().getSize());

	window.draw(backgroundShape);
	window.draw(m_game_over_text);
}

bool GameOverState::Update(sf::Time dt)
{
	// Show state for 3 seconds, after return to menu
	m_elapsed_time += dt;
	if (m_elapsed_time > sf::seconds(3))
	{
		RequestStackClear();
		RequestStackPush(StateID::kMenu);
	}
	return false;
}

bool GameOverState::HandleEvent(const sf::Event&)
{
	return false;
}