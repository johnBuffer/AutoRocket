#include <SFML/Graphics.hpp>
#include <vector>
#include <list>
#include <event_manager.hpp>
#include <iostream>

#include "dna.hpp"
#include "selector.hpp"
#include "number_generator.hpp"
#include "graph.hpp"
#include "rocket.hpp"
#include "stadium.hpp"
#include "rocket_renderer.hpp"
#include "neural_renderer.hpp"


int main()
{
	NumberGenerator<>::initialize();

	const uint32_t win_width = 1600;
	const uint32_t win_height = 900;
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;
	sf::RenderWindow window(sf::VideoMode(win_width, win_height), "SpaceY", sf::Style::Default, settings);
	window.setVerticalSyncEnabled(false);
	window.setFramerateLimit(144);

	bool slow_motion = false;
	const float base_dt = 0.007f;
	float dt = base_dt;

	sfev::EventManager event_manager(window);
	event_manager.addEventCallback(sf::Event::Closed, [&](sfev::CstEv ev) { window.close(); });
	event_manager.addKeyPressedCallback(sf::Keyboard::Escape, [&](sfev::CstEv ev) { window.close(); });

	// The drones colors
	std::vector<sf::Color> colors({ sf::Color(36, 123, 160),
									sf::Color(161, 88, 86),
									sf::Color(249, 160, 97),
									sf::Color(80, 81, 79), 
		                            sf::Color(121, 85, 83),
									sf::Color(242, 95, 92),
									sf::Color(255, 224, 102),
									sf::Color(146, 174, 131),
									sf::Color(74, 158, 170),
									sf::Color(112, 193, 179) });

	sf::Vector2f mouse_target;

	bool show_just_one = true;
	bool full_speed = false;
	bool manual_control = false;
	bool draw_neural = true;
	bool draw_rockets = true;
	bool draw_fitness = true;

	event_manager.addKeyPressedCallback(sf::Keyboard::E, [&](sfev::CstEv ev) { full_speed = !full_speed; window.setFramerateLimit(!full_speed * 144); });
	event_manager.addKeyPressedCallback(sf::Keyboard::M, [&](sfev::CstEv ev) { manual_control = !manual_control; });
	event_manager.addKeyPressedCallback(sf::Keyboard::S, [&](sfev::CstEv ev) { show_just_one = !show_just_one; });
	event_manager.addKeyPressedCallback(sf::Keyboard::N, [&](sfev::CstEv ev) { draw_neural = !draw_neural; });
	event_manager.addKeyPressedCallback(sf::Keyboard::D, [&](sfev::CstEv ev) { draw_rockets = !draw_rockets; });
	event_manager.addKeyPressedCallback(sf::Keyboard::F, [&](sfev::CstEv ev) { draw_fitness = !draw_fitness; });

	const float GUI_MARGIN = 10.0f;
	Graphic fitness_graph(1000, sf::Vector2f(700, 120), sf::Vector2f(GUI_MARGIN, win_height - 120 - GUI_MARGIN));
	fitness_graph.color = sf::Color(96, 211, 148);

	sf::Font font;
	font.loadFromFile("../res/font.ttf");
	sf::Text generation_text;
	sf::Text best_score_text;
	generation_text.setFont(font);
	generation_text.setCharacterSize(42);
	generation_text.setFillColor(sf::Color::White);
	generation_text.setPosition(GUI_MARGIN * 2.0f, GUI_MARGIN);

	best_score_text = generation_text;
	best_score_text.setCharacterSize(32);
	best_score_text.setPosition(4.0f * GUI_MARGIN, 64);
	
	const uint32_t pop_size = 2000;
	Stadium stadium(pop_size, sf::Vector2f(win_width, win_height));

	RocketRenderer rocket_renderer;
	NeuralRenderer neural_renderer;
	const sf::Vector2f size = neural_renderer.getSize(4, 9);
	neural_renderer.position = sf::Vector2f(50.0f, win_height) - sf::Vector2f(0.0f, GUI_MARGIN + size.y);

	sf::RenderStates states;
	states.transform.scale(1.2f, 1.2f);
	states.transform.translate(-160.0f, -90.0f);
	float time = 0.0f;

	while (window.isOpen()) {
		event_manager.processEvents();

		// Initialize rockets
		std::vector<Rocket>& population = stadium.selector.getCurrentPopulation();
		stadium.initializeIteration();

		time = 0.0f;

		while (stadium.getAliveCount() && window.isOpen() && stadium.current_iteration.time < 90.0f) {
			event_manager.processEvents();

			if (manual_control) {
				const sf::Vector2i mouse_position = sf::Mouse::getPosition(window);
				mouse_target.x = static_cast<float>(mouse_position.x);
				mouse_target.y = static_cast<float>(mouse_position.y);
			}

			stadium.update(dt, !full_speed);

			fitness_graph.setLastValue(stadium.current_iteration.best_fitness);
			generation_text.setString("Generation " + toString(stadium.selector.current_iteration));
			best_score_text.setString("Score " + toString(stadium.current_iteration.best_fitness));

			// Render
			window.clear();

			uint32_t current_drone_i = 0;
			if (draw_rockets) {
				for (Rocket& r : population) {
					if (r.alive) {
						rocket_renderer.render(r, window, states, !full_speed && show_just_one);
						if (show_just_one) {
							current_drone_i = r.index;
							break;
						}
					}
				}
			}
			
			if (show_just_one) {
				const float target_radius = 10.0f;
				sf::CircleShape target_c(target_radius);
				target_c.setFillColor(sf::Color(255, 128, 0));
				target_c.setOrigin(target_radius, target_radius);
				const Objective& obj = stadium.objectives[current_drone_i];
				target_c.setPosition(stadium.targets[obj.target_id]);
				if (obj.target_id < stadium.targets_count - 1) {
					window.draw(target_c, states);
				}

				if (!full_speed) {
					RocketRenderer::drawPie(target_radius - 3.0f, (obj.time_in / 3.0f) * 2.0f * PI, sf::Color(75, 75, 75), stadium.targets[obj.target_id], window, states);
					neural_renderer.render(window, stadium.selector.getCurrentPopulation()[current_drone_i].network, sf::RenderStates());
				}
			}

			if (draw_fitness) {
				fitness_graph.render(window);
			}

			window.display();
			time += dt;
		}
		
		fitness_graph.next();
		stadium.nextIteration();
	}

	return 0;
}