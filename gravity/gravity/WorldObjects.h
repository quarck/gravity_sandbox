#pragma once

#include <algorithm>
#include <functional>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <sstream>

#include "vec3d.h"

namespace gravity
{
	struct mass_body
	{
		vec3d location{};
		vec3d velocity{};
		vec3d spin_axis{};

		double spin_rate{};
		double radius{};
		double mass{ 1.0 };
		double temperature{ 300 };

		vec3d gravity_force{}; // current resulting total gravity vector after accounting for all the bodies in the system 
		vec3d acceleration{}; // current acceleration, based on the current gravity force (and mass)

		vec3d p0_acceleration{}; // acceleration at the previos step 
		vec3d p0_velocity{}; // velocity at the previous step

		void iterate(const double time_delta) noexcept 
		{
			p0_acceleration = acceleration;
			acceleration = gravity_force / mass;

			p0_velocity = velocity;
			velocity += (acceleration + p0_acceleration) / 2.0 * time_delta;

			location += (velocity + p0_velocity) / 2.0 * time_delta;
		}

		void save_to(std::ostream & stream)
		{
			location.save_to(stream);
			velocity.save_to(stream);
			spin_axis.save_to(stream);
			stream.write(reinterpret_cast<const char*>(&spin_rate), sizeof(spin_rate));
			stream.write(reinterpret_cast<const char*>(&radius), sizeof(radius));
			stream.write(reinterpret_cast<const char*>(&mass), sizeof(mass));
		}

		void load_from(std::istream & stream)
		{
			location.load_from(stream);
			velocity.load_from(stream);
			spin_axis.load_from(stream);
			stream.read(reinterpret_cast<char*>(&spin_rate), sizeof(spin_rate));
			stream.read(reinterpret_cast<char*>(&radius), sizeof(radius));
			stream.read(reinterpret_cast<char*>(&mass), sizeof(mass));
		}
	};
}