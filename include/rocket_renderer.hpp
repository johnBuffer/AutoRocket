#pragma once

#include <SFML/Graphics.hpp>
#include "rocket.hpp"
#include "gauge_bar.hpp"


struct RocketRenderer
{
	sf::Texture rocket_texture;
	sf::Sprite rocket_sprite;

	sf::Texture flame;
	sf::Sprite flame_sprite;

	sf::Texture smoke;
	sf::Sprite smoke_sprite;

	sf::Font font;
	sf::Text text;
	sf::Text sub_text;

	RocketRenderer()
	{
		rocket_texture.loadFromFile("../res/rocket.png");
		rocket_sprite.setTexture(rocket_texture);

		flame.loadFromFile("../res/flame.png");
		flame_sprite.setTexture(flame);
		flame_sprite.setOrigin(118.0f, 67.0f);
		flame_sprite.setScale(0.15f, 0.15f);

		smoke.loadFromFile("../res/smoke.png");
		smoke_sprite.setTexture(smoke);
		smoke_sprite.setOrigin(126, 134);

		font.loadFromFile("../res/font.ttf");
		text.setFont(font);
		text.setCharacterSize(24);
		text.setFillColor(sf::Color::White);

		sub_text = text;
		sub_text.setCharacterSize(12);
	}

	void render(Rocket& rocket, sf::RenderTarget& target, sf::RenderStates states, bool draw_smoke)
	{
		// Body
		const float width = 15.0f;
		sf::RectangleShape body(sf::Vector2f(rocket.height, width));
		body.setOrigin(rocket.height * 0.5f, width * 0.5f);
		body.setRotation(rocket.angle * RAD_TO_DEG);
		body.setPosition(rocket.position);
		body.setTexture(&rocket_texture);
		body.setScale(1.5f, 1.5f);

		// Flame
		const float thruster_height = 20.0f;
		const float rand_pulse_left = (1.0f + rand() % 10 * 0.05f);
		const float v_scale_left = rocket.thruster.getAvgPowerRatio() * rand_pulse_left;
		const float angle = rocket.angle + rocket.thruster.angle;
		const sf::Vector2f rocket_dir(cos(rocket.angle), sin(rocket.angle));
		const sf::Vector2f thruster_dir(cos(angle), sin(angle));
		flame_sprite.setPosition(rocket.position + 0.5f * rocket.height * rocket_dir + thruster_height * thruster_dir);
		flame_sprite.setScale(0.15f * rocket.thruster.power * rand_pulse_left, 0.15f * v_scale_left);
		flame_sprite.setRotation(RAD_TO_DEG * (angle - HalfPI));

		// Smoke
		if (draw_smoke) {
			for (const Smoke& s : rocket.smoke) {
				const float smoke_scale = 0.25f * s.scale;
				smoke_sprite.setPosition(s.position);
				smoke_sprite.setRotation(RAD_TO_DEG * s.angle);
				smoke_sprite.setScale(smoke_scale, smoke_scale);

				const uint8_t smoke_color = static_cast<uint8_t>(255 * std::min(1.0f, 5.0f / s.scale));
				smoke_sprite.setColor(sf::Color(smoke_color, smoke_color, smoke_color, static_cast<uint8_t>(25 * (1.0f - s.getRatio()))));
				target.draw(smoke_sprite, states);
			}
		}

		// Render
		target.draw(flame_sprite, states);
		target.draw(body, states);

		// Gauge
		const float height = 20.0f;
		const float delta = 20.0f;
		const float delta_y = 70.0f;
		const float off_y = 20.0f;
		const float text_margin_y = 15.0f;
		text.setOrigin(0.0f, text.getGlobalBounds().height + text_margin_y);

		GaugeBar power_bar(1.0f, rocket.position + sf::Vector2f(delta, -off_y), sf::Vector2f(100.0f, height));
		const float rocket_power = rocket.thruster.getAvgPowerRatio();
		power_bar.setValue(rocket_power);
		power_bar.render(target, states);
		text.setString(toString(rocket_power * 100.0f, 0) + "%");
		text.setPosition(rocket.position + sf::Vector2f(delta, -off_y));
		target.draw(text, states);
		sub_text.setString("Booster Load");
		sub_text.setPosition(power_bar.position + sf::Vector2f(0.0f, power_bar.size.y + 2.0f));
		target.draw(sub_text, states);

		GaugeBar angle_bar(2.0f, rocket.position + sf::Vector2f(delta, -off_y + delta_y), sf::Vector2f(100.0f, height));
		angle_bar.setValue(rocket.thruster.getNormAngle() + 1.0f);
		angle_bar.render(target, states);
		text.setString(toString(rocket.thruster.getAvgAngle() * RAD_TO_DEG, 2) + "°");
		text.setPosition(rocket.position + sf::Vector2f(delta, -off_y + delta_y));
		target.draw(text, states);
		sub_text.setString("Booster Orientation");
		sub_text.setPosition(angle_bar.position + sf::Vector2f(0.0f, angle_bar.size.y + 2.0f));
		target.draw(sub_text, states);
	}

	static void drawPie(float radius, float angle, sf::Color color, sf::Vector2f position, sf::RenderTarget& target, sf::RenderStates states)
	{
		const uint32_t quality = 16;
		sf::VertexArray va(sf::TriangleFan, quality + 2);
		const float da = angle / float(quality);
		va[0].position = position;
		va[0].color = color;
		for (uint32_t i(0); i < quality + 1; ++i) {
			const float angle = i * da;
			va[i + 1].position = position + radius * sf::Vector2f(cos(angle), sin(angle));
			va[i + 1].color = color;
		}
		target.draw(va, states);
	}
};
