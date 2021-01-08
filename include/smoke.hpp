#pragma once

#include <SFML/Graphics.hpp>
#include "utils.hpp"


struct Smoke
{
	sf::Vector2f position;
	sf::Vector2f direction;
	float speed;
	float angle;
	float scale;
	float max_lifetime;
	float lifetime;
	float angle_var;
	bool hit;

	Smoke(sf::Vector2f pos, sf::Vector2f dir, float speed_, float scale_, float duration)
		: position(pos)
		, direction(dir)
		, speed(speed_)
		, angle(getFastRandUnder(2.0f * PI))
		, scale(0.25f + scale_ * (0.25f + getFastRandUnder(1.0f)))
		, max_lifetime(duration)
		, lifetime(0.0f)
		, angle_var(1.0f * (getFastRandUnder(2.0f) - 1.0f))
		, hit(false)
	{
	}

	float getRatio() const
	{
		return lifetime / max_lifetime;
	}

	bool done() const
	{
		return lifetime >= max_lifetime;
	}

	void update(float dt)
	{
		const float ratio = getRatio();
		const float inv_ratio = (1.0f - ratio);
		lifetime += dt;
		angle += angle_var * dt;
		scale *= 1.0f + 2.5f * dt;
		position += (speed * direction * dt) / scale;

		if (!hit && position.y > 990.0f) {
			hit = true;
			const float old_dir_x = direction.x;
			const float new_dir_x = getFastRandUnder(1.0f) > 0.5f ? -1.0f : 1.0f;
			position.y = 990.0f;
			direction.x = new_dir_x * direction.y;
			direction.y = -getFastRandUnder(scale * 0.5f);
		}
	}
};
