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

		vec3d location_compensation{};
		vec3d velocity_compensation{};

		vec3d spin_axis{};

		double spin_rate{};
		double radius{};
		double mass{ 1.0 };
		double temperature{ 300 };

		vec3d gravity_force{}; // current resulting total gravity vector after accounting for all the bodies in the system 
		vec3d gravity_force_compensation{};

		vec3d acceleration{}; // current acceleration, based on the current gravity force (and mass)

		vec3d p0_acceleration{}; // acceleration at the previos step 
		vec3d p1_acceleration{}; // acceleration at the previos step 

		vec3d p0_velocity{}; // velocity at the previous step
		vec3d p1_velocity{}; // velocity at the previous step


		inline void iterate_linear(const double time_delta) noexcept
		{
			p0_acceleration = acceleration;
			acceleration = gravity_force / mass;

			p0_velocity = velocity;
			velocity += (acceleration + p0_acceleration) / 2.0 * time_delta;

			location += (velocity + p0_velocity) / 2.0 * time_delta;
		}

		inline void iterate_linear_kahan(const double time_delta) noexcept
		{
			p0_acceleration = acceleration;
			acceleration = gravity_force / mass;

			p0_velocity = velocity;

			{
				auto input{ (acceleration + p0_acceleration) / 2.0 * time_delta  };

				auto y = input - velocity_compensation;
				auto t = velocity + y;						
				velocity_compensation = (t - velocity) - y;
				velocity = t;
			}

			{
				auto input{ (velocity + p0_velocity) / 2.0 * time_delta };

				auto y = input - location_compensation;
				auto t = location + y;
				location_compensation = (t - location) - y;
				location = t;
			}
		}

		inline void iterate_quadratic(const double time_delta) noexcept
		{
			p1_acceleration = p0_acceleration;
			p0_acceleration = acceleration;
			acceleration = gravity_force / mass;

			p1_velocity = p0_velocity;
			p0_velocity = velocity;

			velocity += (5*acceleration + 8*p0_acceleration - p1_acceleration) / 12.0 * time_delta;

			location += (5*velocity + 8*p0_velocity - p1_velocity) / 12.0 * time_delta;
		}

		inline void iterate_quadratic_kahan(const double time_delta) noexcept
		{
			p1_acceleration = p0_acceleration;
			p0_acceleration = acceleration;
			acceleration = gravity_force / mass;

			p1_velocity = p0_velocity;
			p0_velocity = velocity;

			{
				auto input{ (5 * acceleration + 8 * p0_acceleration - p1_acceleration) / 12.0 * time_delta };

				auto y = input - velocity_compensation;
				auto t = velocity + y;
				velocity_compensation = (t - velocity) - y;
				velocity = t;
			}

			{
				auto input{ (5 * velocity + 8 * p0_velocity - p1_velocity) / 12.0 * time_delta };

				auto y = input - location_compensation;
				auto t = location + y;
				location_compensation = (t - location) - y;
				location = t;
			}

		}

		inline void iterate(const double time_delta) noexcept
		{
			return iterate_quadratic_kahan(time_delta);
		}


		void save_to(std::ostream & stream)
		{
			location.save_to(stream);
			velocity.save_to(stream);
			spin_axis.save_to(stream);

			gravity_force.save_to(stream);
			acceleration.save_to(stream);
			p0_acceleration.save_to(stream);
			p1_acceleration.save_to(stream);
			p0_velocity.save_to(stream);
			p1_velocity.save_to(stream);

			stream.write(reinterpret_cast<const char*>(&spin_rate), sizeof(spin_rate));
			stream.write(reinterpret_cast<const char*>(&radius), sizeof(radius));
			stream.write(reinterpret_cast<const char*>(&mass), sizeof(mass));
			stream.write(reinterpret_cast<const char*>(&temperature), sizeof(temperature));
		}

		void load_from(std::istream & stream)
		{
			location.load_from(stream);
			velocity.load_from(stream);
			spin_axis.load_from(stream);

			gravity_force.load_from(stream);
			acceleration.load_from(stream);
			p0_acceleration.load_from(stream);
			p1_acceleration.load_from(stream);
			p0_velocity.load_from(stream);
			p1_velocity.load_from(stream);

			stream.read(reinterpret_cast<char*>(&spin_rate), sizeof(spin_rate));
			stream.read(reinterpret_cast<char*>(&radius), sizeof(radius));
			stream.read(reinterpret_cast<char*>(&mass), sizeof(mass));
			stream.read(reinterpret_cast<char*>(&temperature), sizeof(temperature));
		}
	};
}