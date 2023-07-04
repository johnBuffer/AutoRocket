#pragma once

#include <swarm.hpp>

#include "selector.hpp"
#include "rocket.hpp"
#include "objective.hpp"


struct Stadium
{
	struct Iteration
	{
		float time;
		float best_fitness;

		void reset()
		{
			time = 0.0f;
			best_fitness = 0.0f;
		}
	};

	uint32_t population_size;
	Selector<Rocket> selector;
	uint32_t targets_count;
	std::vector<sf::Vector2f> targets;
	std::vector<Objective> objectives;
	sf::Vector2f area_size;
	Iteration current_iteration;
	swrm::Swarm swarm;

	Stadium(uint32_t population, sf::Vector2f size)
		: population_size(population)
		, selector(population)
		, targets_count(8)
		, targets(targets_count)
		, objectives(population)
		, area_size(size)
		, swarm(4)
	{
		initializeTargets();
	}

	void loadDnaFromFile(const std::string& filename)
	{
		const uint64_t bytes_count = Network::getParametersCount(architecture) * 4;
		const uint64_t dna_count = DnaLoader::getDnaCount(filename, bytes_count);
		for (uint64_t i(0); i < dna_count && i < population_size; ++i) {
			const DNA dna = DnaLoader::loadDnaFrom(filename, bytes_count, i);
			selector.getCurrentPopulation()[i].loadDNA(dna);
		}
	}

	void initializeTargets()
	{
		// Initialize targets
		const float border_x = 360.0f;
		const float border_y = 290.0f;
		for (uint32_t i(0); i < targets_count - 1; ++i) {
			targets[i] = sf::Vector2f(border_x + getRandUnder(area_size.x - 2.0f * border_x), border_y + getRandUnder(area_size.y - 2.0f * border_y));
		}

		targets[targets_count-1] = sf::Vector2f(area_size.x * 0.5f, 900.0f);
	}

	void initializeUnits()
	{
		// Initialize targets
		auto& rockets = selector.getCurrentPopulation();
		uint32_t i = 0;
		for (Rocket& r : rockets) {
			r.index = i++;
			Objective& objective = objectives[r.index];
			r.position = sf::Vector2f(area_size.x * 0.5f, area_size.y * 0.75f);
			objective.reset();
			objective.points = getLength(r.position - targets[0]);
			r.reset();
		}
	}

	bool checkAlive(const Rocket& rocket, float tolerance) const
	{
		const sf::Vector2f tolerance_margin = sf::Vector2f(tolerance, tolerance);
		const bool in_window = sf::FloatRect(-tolerance_margin, area_size + tolerance_margin).contains(rocket.position);
		return in_window && sin(rocket.angle) > 0.0f;
	}

	uint32_t getAliveCount() const
	{
		uint32_t result = 0;
		const auto& rockets = selector.getCurrentPopulation();
		for (const Rocket& r : rockets) {
			result += r.alive;
		}

		return result;
	}

	void updateUnit(uint64_t i, float dt, bool update_smoke)
	{
		Rocket& r = selector.getCurrentPopulation()[i];
		if (!r.alive) {
			// It's too late for it
			return;
		}

		const float target_radius = 8.0f;
		const float max_dist = 500.0f;
		const float tolerance_margin = 50.0f;
		
		Objective& objective = objectives[r.index];
		sf::Vector2f to_target = objective.getTarget(targets) - r.position;
		const float to_target_dist = getLength(to_target);
		to_target.x /= std::max(to_target_dist, max_dist);
		to_target.y /= std::max(to_target_dist, max_dist);

		if (objective.target_id == targets_count - 1) {
			objective.time_in = 0.0f;
			if (to_target_dist < 1.0f && std::abs(r.angle - HalfPI) < 0.01f) {
				r.stop = true;
			}
		}

		const std::vector<float> inputs = {
			to_target.x,
			to_target.y,
			r.velocity.x * dt,
			r.velocity.y * dt,
			static_cast<float>(cos(r.angle)),
			static_cast<float>(sin(r.angle)),
			r.angular_velocity * dt
		};
		// The actual update
		if (!r.stop) {
			r.execute(inputs);
		}
		r.update(dt, update_smoke);
		r.alive = checkAlive(r, tolerance_margin);
		// Fitness stuffs
		const float jerk_malus = std::abs(r.last_power - r.thruster.power);
		const float move_malus = 0.1f * getLength(r.velocity * dt);
		r.fitness += 10.0f * jerk_malus / (1.0f + to_target_dist);
		// We don't want weirdos
		const float score_factor = std::pow(sin(r.angle), 2.0f);
		const float target_reward_coef = score_factor * 10.0f;
		const float target_time = 1.0f;
		if (to_target_dist < target_radius) {
			objective.addTimeIn(dt);
			if (objective.time_in > target_time) {
				r.fitness += target_reward_coef * objective.points / (1.0f + objective.time_out + to_target_dist);
				//r.fitness += objective.time_in;
				objective.nextTarget(targets);
				objective.points = getLength(r.position - objective.getTarget(targets));
			}
		}
		else {
			objective.addTimeOut(dt);
		}

		checkBestFitness(r.fitness);
	}

	void checkBestFitness(float fitness)
	{
		current_iteration.best_fitness = std::max(fitness, current_iteration.best_fitness);
	}

	void update(float dt, bool update_smoke)
	{
		const uint64_t population_size = selector.getCurrentPopulation().size();
		auto group_update = swarm.execute([&](uint32_t thread_id, uint32_t max_thread) {
			const uint64_t thread_width = population_size / max_thread;
			for (uint64_t i(thread_id * thread_width); i < (thread_id + 1) * thread_width; ++i) {
				updateUnit(i, dt, update_smoke);
			}
		});
		group_update.waitExecutionDone();
		current_iteration.time += dt;
	}

	void initializeIteration()
	{
		initializeTargets();
		initializeUnits();
		current_iteration.reset();
	}

	void nextIteration()
	{
		selector.nextGeneration();
	}
};
