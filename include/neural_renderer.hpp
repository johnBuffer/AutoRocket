#pragma once

#include "neural_network.hpp"
#include "utils.hpp"


struct GLayer
{
	std::vector<sf::Vector2f> neurons_positions;
};


struct NeuralRenderer
{
	void render(sf::RenderTarget& target, Network& network, sf::RenderStates states)
	{
		updateLayers(network);

		const uint64_t layers_count = layers.size();
		for (uint64_t i(1); i <layers_count; ++i) {
			const GLayer& curr_layer = layers[i];
			const GLayer& prev_layer = layers[i - 1];
			uint32_t neuron_id = 0;
			for (const sf::Vector2f& neuron_pos : curr_layer.neurons_positions) {
				uint32_t weight_id = 0;
				for (const sf::Vector2f& prev_neuron_pos : prev_layer.neurons_positions) {
					const float link_weight = network.layers[i - 1].weights[neuron_id][weight_id];
					const float link_value = link_weight * (i == 1 ? network.last_input : network.layers[i - 2].values)[weight_id];
					const float color_value = std::pow(link_value + 1.0f, 3.0f);
					const sf::Vector3f link_color = sf::Vector3f(128, 128, 128) + sf::Vector3f(color_value, color_value, color_value);
					const float link_width = 2.0f;// 2.0f * log2(1.0f + std::abs(link_value));
					target.draw(getLine(neuron_pos, prev_neuron_pos, link_width, toColor(link_color)), states);
					++weight_id;
				}
				++neuron_id;
			}
		}

		uint32_t layer_id = 0;
		for (GLayer& layer : layers) {
			uint32_t neuron_id = 0;
			for (const sf::Vector2f& pos : layer.neurons_positions) {
				const sf::Vector3f base_color(120, 120, 120);
				const sf::Vector3f activated_color(255, 255, 255);
				
				sf::Color neuron_color;
				float current_neuron_radius = neuron_radius;
				float intensity = 0.0f;

				if (!layer_id) {
					intensity = std::min(1.0f, std::abs(network.last_input[neuron_id]));
				}
				else if (layer_id == layers_count - 1) {
					intensity = std::abs(network.layers.back().values[neuron_id]);
				}
				else {
					intensity = std::abs(network.layers[layer_id-1].values[neuron_id]);
					current_neuron_radius = neuron_radius * 0.8f;
				}

				sf::Vector3f color_vec = intensity * activated_color + std::pow(1.0f - intensity, 3.0f) * base_color;
				neuron_color = toColor(color_vec);

				sf::CircleShape neuron_shape(current_neuron_radius);
				neuron_shape.setOrigin(current_neuron_radius, current_neuron_radius);
				neuron_shape.setPosition(pos);
				neuron_shape.setFillColor(sf::Color::Black);
				neuron_shape.setOutlineColor(sf::Color::White);
				neuron_shape.setOutlineThickness(1.0f);
				target.draw(neuron_shape, states);
				neuron_shape.setFillColor(neuron_color);
				neuron_shape.setScale(0.9f, 0.9f);
				neuron_shape.setOutlineThickness(0.0f);
				target.draw(neuron_shape, states);
				++neuron_id;
			}
			++layer_id;
		}
	}

	float getLayerHeight(uint64_t neurons_count)
	{
		return neurons_count * (2.0f * neuron_radius + neuron_spacing);
	}

	float getWidth(uint64_t layers_count)
	{
		return (layers_count - 1) * (2.0f * neuron_radius + layer_spacing) + neuron_radius;
	}

	sf::Vector2f getSize(uint64_t layers_count, uint64_t max_neurons_on_layer)
	{
		return sf::Vector2f(getWidth(layers_count), getLayerHeight(max_neurons_on_layer));
	}

	void updateLayers(Network& network)
	{
		layers.clear();
		// Find network height
		float max_layer_height = getLayerHeight(network.input_size);
		for (const Layer& layer : network.layers) {
			const float layer_height = getLayerHeight(layer.getNeuronsCount());
			if (layer_height > max_layer_height) {
				max_layer_height = layer_height;
			}
		}

		float layer_x = position.x;
		float layer_height = getLayerHeight(network.input_size);
		float neuron_y = position.y + 0.5f * (max_layer_height - layer_height) + neuron_radius;
		// Draw inputs
		layers.emplace_back();
		for (uint64_t i(0); i < network.input_size; ++i) {
			layers.back().neurons_positions.push_back(sf::Vector2f(layer_x, neuron_y));
			neuron_y += 2.0f * neuron_radius + neuron_spacing;
		}
		layer_x += 2.0f * neuron_radius + layer_spacing;
		// Draw layers
		for (const Layer& layer : network.layers) {
			layers.emplace_back();
			layer_height = getLayerHeight(layer.getNeuronsCount());
			neuron_y = position.y + 0.5f * (max_layer_height - layer_height) + neuron_radius;
			for (uint32_t i(0); i < layer.getNeuronsCount(); ++i) {
				layers.back().neurons_positions.push_back(sf::Vector2f(layer_x, neuron_y));
				neuron_y += 2.0f * neuron_radius + neuron_spacing;
			}

			layer_x += 2.0f * neuron_radius + layer_spacing;
		}
	}

	float neuron_radius = 20.0f;
	float neuron_spacing = 8.0f;
	float layer_spacing = 60.0f;
	sf::Vector2f position;
	std::vector<GLayer> layers;
};
