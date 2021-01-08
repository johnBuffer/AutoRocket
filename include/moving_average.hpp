#pragma once
#include <cstdint>
#include <vector>

struct MovingAverage
{
	uint32_t over;
	std::vector<float> values;
	uint32_t index;
	float sum;

	MovingAverage(uint32_t count)
		: over(count)
		, values(count, 0.0f)
		, index(0)
		, sum(0.0f)
	{}

	void addValue(float f)
	{
		if (index >= over) {
			sum -= values[index%over];
		}
		values[index%over] = f;
		sum += f;
		++index;
	}

	float get() const {
		return sum / float(over);
	}
};