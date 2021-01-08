#pragma once
#include "ai_unit.hpp"
#include "smoke.hpp"
#include <list>
#include "moving_average.hpp"


std::vector<uint64_t> architecture{ 7, 9, 9, 2 };

struct Rocket : AiUnit
{
	struct Thruster
	{
		float max_angle;
		float angle;
		float target_angle;

		float max_power;
		float power;

		MovingAverage avg_power;
		MovingAverage avg_angle;

		Thruster()
			: max_power(3000.0f)
			, max_angle(HalfPI)
			, avg_power(60)
			, avg_angle(60)
		{}

		void update(float dt)
		{
			const float angle_speed = 1.0f;
			angle += angle_speed * dt * (target_angle - angle);
			avg_angle.addValue(angle);
		}

		float getNormAngle() const
		{
			return angle / max_angle;
		}

		void setAngle(float a)
		{
			target_angle = a * max_angle;
		}

		void setPower(float p)
		{
			power = p;
			avg_power.addValue(p);
		}

		float getPower() const
		{
			return power * max_power;
		}

		float getAvgPowerRatio() const
		{
			return avg_power.get();
		}

		float getAvgAngle() const
		{
			return avg_angle.get();
		}

		void reset()
		{
			power = 0.0f;
			angle = 0.0f;
			target_angle = 0.0f;
		}
	};

	Thruster thruster;
	sf::Vector2f position;
	sf::Vector2f velocity;
	float angle;
	float angular_velocity;
	float height;
	uint32_t index;
	float last_power;
	std::list<Smoke> smoke;
	float time;
	bool stop;
	bool take_off;

	Rocket()
		: AiUnit(architecture)
		, height(120.0f)
		, last_power(0.0f)
	{
		reset();
	}

	void reset() 
	{
		velocity = sf::Vector2f(0.0f, 0.0f);
		angle = HalfPI;
		angular_velocity = 0.0f;
		thruster.reset();
		alive = true;
		fitness = 0.0f;
		time = 0.0f;
		stop = false;
	}

	sf::Vector2f getThrust() const
	{
		const float thruster_angle = angle + thruster.angle;
		return -thruster.getPower() * sf::Vector2f(cos(thruster_angle), sin(thruster_angle));
	}

	static float cross(sf::Vector2f v1, sf::Vector2f v2)
	{
		return v1.x * v2.y - v1.y * v2.x;
	}

	float getTorque() const
	{
		const sf::Vector2f v(cos(thruster.angle), sin(thruster.angle));
		const float inertia_coef = 0.8f;
		return thruster.getPower() / (0.5f * height) * cross(v, sf::Vector2f(1.0f, 0.0f));
	}

	void update(float dt, bool update_smoke)
	{
		thruster.update(dt);
		const sf::Vector2f gravity(0.0f, 1000.0f);
		velocity += (gravity + getThrust()) * dt;
		position += velocity * dt;

		angular_velocity += getTorque() * dt;
		angle += angular_velocity * dt;
	
		if (update_smoke) {
			const sf::Vector2f rocket_dir(cos(angle), sin(angle));
			const float smoke_vert_offset = 40.0f;
			const float smoke_duration = 0.5f;
			const float smoke_speed_coef = 0.25f;
			const float power_ratio = 4.0f * thruster.getAvgPowerRatio();
			if (power_ratio > 0.15f) {
				const float power = thruster.max_power * power_ratio;
				const float thruster_angle = angle + thruster.angle;
				const sf::Vector2f thruster_direction(cos(thruster_angle), sin(thruster_angle));
				const sf::Vector2f thruster_pos = position + rocket_dir * height * 0.5f + thruster_direction * smoke_vert_offset * power_ratio;

				smoke.push_back(Smoke(thruster_pos, thruster_direction, smoke_speed_coef * power, 0.15f + 0.5f * power_ratio, smoke_duration * power_ratio));
			}

			for (Smoke& s : smoke) {
				s.update(dt);
			}

			smoke.remove_if([this](const Smoke& s) { return s.done(); });
		}

		time += dt;
	}

	void process(const std::vector<float>& outputs) override
	{
		last_power = thruster.power;
		thruster.setPower(0.5f * (outputs[0] + 1.0f));
		thruster.setAngle(outputs[1]);
	}
};
