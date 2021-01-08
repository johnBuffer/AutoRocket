#pragma once
#include <SFML/Graphics.hpp>


struct GaugeBar
{
	float max_value;
	float value;

	sf::Vector2f size;
	sf::Vector2f position;

	GaugeBar()
		: max_value(0.0f)
		, value(0.0f)
	{}

	GaugeBar(float max, sf::Vector2f pos, sf::Vector2f size_)
		: max_value(max)
		, value(0.0f)
		, position(pos)
		, size(size_)
	{}

	void setValue(float v)
	{
		value = v;
	}

	void render(sf::RenderTarget& target, sf::RenderStates states) const
	{
		sf::RectangleShape border(size);
		border.setPosition(position);
		target.draw(border, states);

		const sf::Vector2f padding(1.0f, 1.0f);
		sf::RectangleShape inner(size - 2.0f * padding);
		inner.setPosition(position + padding);
		inner.setFillColor(sf::Color::Black);
		target.draw(inner, states);

		const float bar_width = (size.x - 4.0f) * (value / max_value);
		sf::RectangleShape bar(sf::Vector2f(bar_width, size.y - 4.0f));
		bar.setPosition(position + 2.0f * padding);
		target.draw(bar, states);
	}
};

