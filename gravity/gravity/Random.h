#pragma once

#include <stdexcept>
#include <random>
#include <chrono>

class Random
{
    std::default_random_engine generator;

    std::uniform_real_distribution<double> realDistribution{ 0.0, 1.0 };
	std::uniform_real_distribution<float> floatDistribution{ 0.0f, 1.0f };
    std::uniform_int_distribution<int> intDistribution{ 0, INT_MAX };

public:
    Random()
    {
        unsigned seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());
        generator.seed(seed);
    }

	template <typename T>
	T Next(const T& from, const T& to)
	{
		return static_cast<T>(static_cast<double>(from - to) * NextDouble() + static_cast<double>(from));
	}

	template <>
	float Next(const float& from, const float& to) noexcept
	{
		return static_cast<float>((from - to) * NextFloat() + from);
	}

	template <>
	double Next(const double& from, const double& to) noexcept
	{
		return (from - to) * NextDouble() + from;
	}

    double NextDouble() noexcept
    {
        return realDistribution(generator);
    }

	float NextFloat() noexcept
	{
		return floatDistribution(generator);
	}

    int Next() noexcept
    {
        return intDistribution(generator);
    }

    int Next(int max) noexcept
    {
        return Next() % max;
    }
};

