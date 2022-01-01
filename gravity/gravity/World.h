// TODO: ideas to try: 
// * caves 
// * R-G-B games
// * gravity 

#pragma once 

#define _USE_MATH_DEFINES
#include <math.h>


#include <algorithm>
#include <functional>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <unordered_set>
#include <sstream>

#include <ppl.h>

#include "vec3d.h"
#include "Random.h"
#include "ThreadGrid.h"

#include "Utils.h"

#include "WorldObjects.h"

#include "Log.h"

namespace gravity
{ 
    class World
    {
	public:
		static constexpr double GRAVITATIONAL_CONSTANT = 6.67430e-11; // m*m*m/(kg*s*s)

		static constexpr double TIME_DELTA{ 10 * 10 }; // seconds

		static constexpr double SQRT_2 = 1.4142135623730950488016887242097f; // unfortunately std::sqrt is not a constexpr function


		static constexpr double ONE_A_U{ 1.496e+11 }; // meters 
		static constexpr double JUPUTER_ORBIT_RADIUS{ 778.5e+9 };

		static constexpr double SUN_MASS{ 1.989e+30 };
		static constexpr double SUN_RADIUS{ 696340'000 };

		static constexpr double EARTH_MASS{ 5.972e+24 };
		static constexpr double EARTH_RADIUS{ 6500'000 };

		static constexpr double JUPYTER_MASS{ 1.898e+27 };
		static constexpr double JUPYTER_RADIUS{ 69911'000 };


	private:
        std::vector<mass_body> _objects;
		std::vector<vec3d> _gravity_field;

		std::list<std::unordered_set<int>> _collisions;
		std::mutex _collisions_mutex;

        Random _random{};

        std::string _workingFolder;
        bool _workingFolderCreated{ false };


	public:
        World(const std::string& workingFolder, int nWorkerThreads)
            :  _workingFolder(workingFolder)
        {	

			init_planets();
        }

		~World()
		{
		}


		static double orbital_velocity(double M, double R) noexcept 
		{
			return std::sqrt(GRAVITATIONAL_CONSTANT * M / R);
		}

		double populate_orbit(
			double sun_mass_adjusted,
			int num_planets,
			double mass,
			double radius,
			double orbit_radius,
			double orbit_direction
		)
		{
			double total_mass{ 0 };
			for (int i = 0; i < num_planets; ++i)
			{
				mass_body planet{};
				planet.mass = mass; // kg
				planet.radius = radius; // m
				planet.temperature = 300; // K

				total_mass += planet.mass;

				double loc_angle = M_PI * 2.0 / num_planets * i;
				double vec_angle = loc_angle + M_PI / 2.0;

				double V = orbital_velocity(sun_mass_adjusted, orbit_radius);

				planet.location.x() = orbit_radius * std::cos(loc_angle);
				planet.location.y() = orbit_radius * std::sin(loc_angle);
				planet.velocity.x() = orbit_direction * V * std::cos(vec_angle);
				planet.velocity.y() = orbit_direction * V * std::sin(vec_angle);

				_objects.push_back(planet);
			}

			return total_mass;
		}

		void init_planets()
		{
			mass_body sun{};
			sun.mass = SUN_MASS; // kg
			sun.radius = SUN_RADIUS; // m
			sun.temperature = 10000000;

			//mass_body neutron_star{};
			//neutron_star.mass = SUN_MASS / 2.0;
			//neutron_star.radius = EARTH_RADIUS / 1000;
			//neutron_star.temperature = 1000000;
			//neutron_star.location.x() = SUN_RADIUS * 10;
			//neutron_star.velocity.y() = orbital_velocity(sun.mass, neutron_star.location.x());


			//sun.velocity.y() = -neutron_star.velocity.y() * (neutron_star.mass / sun.mass);

			_objects.push_back(sun);
			//_objects.push_back(neutron_star);

			//mass_body jupyter{};
			//jupyter.mass = JUPYTER_MASS; // kg
			//jupyter.radius = JUPYTER_RADIUS; // m
			//jupyter.location.x = JUPUTER_ORBIT_RADIUS; // m
			//jupyter.velocity.y = orbital_velocity(sun.mass, JUPUTER_ORBIT_RADIUS);  // 13.06e+3; // m/s
			//jupyter.temperature = 273;
			//_objects.push_back(jupyter);


			//mass_body dupyter{};
			//dupyter.mass = JUPYTER_MASS; // kg
			//dupyter.radius = JUPYTER_RADIUS; // m
			//dupyter.location.x = ONE_A_U; // m
			//dupyter.velocity.y = orbital_velocity(sun.mass, ONE_A_U);
			//dupyter.temperature = 273;
			//_objects.push_back(dupyter);


			double sun_mass_adjusted = sun.mass;// +neutron_star.mass;

			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 20, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 0.85, -1.0);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 24, EARTH_MASS, EARTH_RADIUS, ONE_A_U, 1.0);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 28, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 1.15, -1.0);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 32, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 1.3, 1.0);

			//mass_body impactor{};
			//impactor.mass = EARTH_MASS / 10000; // kg 
			//impactor.radius = 3500'000; // m
			//impactor.temperature = 300; // K
			//impactor.location.x() = 0; // m
			//impactor.location.y() = -247.1e+9;
			//impactor.velocity.x() = -14.78e+3 * 1.291;
			//impactor.velocity.y() = 0; // m/s
			//_objects.push_back(impactor);

			_gravity_field.resize(_objects.size() * _objects.size());
		}

        const std::vector<mass_body>& get_objects() noexcept
        {
            return _objects;
        }

		void save_to(std::ostream& stream)
		{
            uint32_t len = static_cast<uint32_t>(_objects.size());
			stream.write(reinterpret_cast<const char*>(&len), sizeof(len));
            for (auto& obj : _objects)
            {
                obj.save_to(stream);
            }
		}

		void load_from(std::istream& stream)
		{
            uint32_t len;
			stream.read(reinterpret_cast<char*>(&len), sizeof(len));
            _objects.resize(len);
            for (uint32_t i = 0; i < len; ++i)
            {
                _objects[i] = {};
                _objects[i].load_from(stream);
            }
		}

		void register_collisions(int i, int j)
		{
			std::lock_guard l{ _collisions_mutex };

			bool found = false;
			for (auto& c : _collisions)
			{
				if (c.count(i) > 0 || c.count(j) > 0)
				{
					found = true;
					c.insert(i);
					c.insert(j);
				}
			}

			if (!found)
			{
				_collisions.push_back(std::unordered_set{ i, j });
			}
		}



		void iterate_gravity_forces(int i, int j) noexcept
		{
			if (i == j)
				return;

			auto& a{ _objects[i] };
			auto& b{ _objects[j] };

			auto r_ba = b.location - a.location;
			auto r_modulo = r_ba.modulo();

			if (r_modulo > a.radius + b.radius)
			{
				auto F_ab = r_ba * (GRAVITATIONAL_CONSTANT * a.mass * b.mass / std::pow(r_modulo, 3.0));

				{
					auto y = F_ab - a.gravity_force_compensation;
					auto t = a.gravity_force + y;
					a.gravity_force_compensation = (t - a.gravity_force) - y;
					a.gravity_force = t;
				}

				{
					auto y = (-1.0)*F_ab - b.gravity_force_compensation;
					auto t = b.gravity_force + y;
					b.gravity_force_compensation = (t - b.gravity_force) - y;
					b.gravity_force = t;
				}

				if (r_modulo < a.radius * 10)
				{
					a.temperature = std::max(a.temperature, 1000.0); // tidal forces stirr the mantel, floor is lava in the whole planet now
				}

				if (r_modulo < b.radius * 10)
				{
					b.temperature = std::max(b.temperature, 1000.0); // tidal forces stirr the mantel, floor is lava in the whole planet now
				}
			}
			else
			{
				register_collisions(i, j);
			}
		}		

		void iterate_collision_merges() noexcept
		{
			std::lock_guard l{ _collisions_mutex };

			if (_collisions.empty())
				return; 

			std::vector<bool> idx_to_remove(_objects.size());

			for (auto& c : _collisions)
			{
				vec3d mass_location{};
				vec3d mass_velocity{};
				vec3d mass_accel{}; // for p0 only

				double total_mass{};

				double total_vol_times_N{};

				double max_temp{0};

				int dst_idx = *c.begin();
				for (auto& idx : c)
				{
					if (idx != dst_idx)
					{
						idx_to_remove[idx] = true;
					}

					auto& o{ _objects[idx] };
					mass_location += o.location * o.mass;
					mass_velocity += o.velocity * o.mass;
					mass_accel += o.acceleration * o.mass;

					total_mass += o.mass;

					total_vol_times_N += std::pow(o.radius, 3.0);

					max_temp = std::max(max_temp, o.temperature);
				}

				auto& dst{ _objects[dst_idx] };

				dst.mass = total_mass;
				dst.radius = std::pow(total_vol_times_N, 1.0 / 3.0);

				dst.location = mass_location / total_mass;
				dst.velocity = mass_velocity / total_mass;
				dst.p0_velocity = dst.velocity;
				dst.p0_acceleration = mass_accel / total_mass;
				dst.temperature = std::max(max_temp, 3000.0);
			}

			_collisions.clear();

			for (int idx = static_cast<int>(_objects.size()) - 1; idx >= 0; idx--)
			{
				if (idx_to_remove[idx])
				{
					_objects.erase(_objects.begin() + idx);
				}
			}
		}
	
		void iterate_forces() noexcept
		{
			// Calculate inter-object gravities
			for (int i = 0; i < _objects.size(); ++i)
			{
				for (int j = 0; j < i; ++j)
				{
					iterate_gravity_forces(i, j);
				}
			}
		}

		void iterate_moves() noexcept
		{
			// Apply the forces into accelerations & movements
			for (int i = 0; i < _objects.size(); ++ i)
			{
				auto& o = _objects[i];
				//auto* src = &_gravity_field[i * _objects.size()];

				//vec3d sum = {};
				//vec3d compensation = {};

				//// Kahan summation algorithm
				//for (int j = 0; j < _objects.size(); ++j)
				//{
				//	//sum += src[j];
				//	//o.gravity_force += src[j];

				//	auto y = src[j] - compensation;			// c is zero the first time around.
				//	auto t = sum + y;						// Alas, sum is big, y small, so low-order digits of y are lost.
				//	compensation = (t - sum) - y;           // (t - sum) cancels the high-order part of y; subtracting y recovers negative (low part of y)
				//	sum = t;								// Algebraically, c should always be zero. Beware overly-aggressive optimizing compilers!
				//}
				//o.gravity_force = sum;
				o.iterate(TIME_DELTA);
			}

			//concurrency::parallel_for_each(_objects.begin(), _objects.end(), [&](auto& o) { o.iterate(TIME_DELTA); });
		}

		void set_pause(bool pause)
		{

		}
	
	public:
		void Iterate(int64_t step)  noexcept
		{
			if (step == 0)
				WorldInitialize();

			// Wipe the previous gravity calculations 
			for (auto& o : _objects)
			{
				o.gravity_force = { 0.0, 0.0, 0.0 };
			}

			iterate_forces();
			iterate_moves();
			iterate_collision_merges();
        }

	private:


        void WorldInitialize()  noexcept
        {
        }
    };
}
