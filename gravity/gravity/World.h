﻿// TODO: ideas to try: 
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

		static constexpr double TIME_DELTA{ 10 }; // seconds

		static constexpr double SQRT_2 = 1.4142135623730950488016887242097f; // unfortunately std::sqrt is not a constexpr function


		static constexpr double EARTH_SUN_DISTANCE{ 147.1e+9 }; // meters 
		static constexpr double EARTH_ORBITAL_VELOCITY{ 29.78e+3 }; // m/s

	private:
        std::vector<mass_body> _objects;
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


		void init_planets()
		{
			mass_body sun{};
			sun.mass = 1.989e+30; // kg
			sun.radius = 696340'000; // m
			sun.temperature = 10000000;
			_objects.push_back(sun);

			mass_body jupyter{};
			jupyter.mass = 1.898e+27; // kg
			jupyter.radius = 69911'000; // m
			jupyter.location.x = 778.5e+9; // m
			jupyter.velocity.y = 13.06e+3; // m/s
			jupyter.temperature = 273;
			_objects.push_back(jupyter);


			//mass_body dupyter{};
			//dupyter.mass = 1.898e+27; // kg
			//dupyter.radius = 69911'000; // m
			//dupyter.location.x = EARTH_SUN_DISTANCE; // m
			//dupyter.velocity.y = EARTH_ORBITAL_VELOCITY; // m/s
			//dupyter.temperature = 273;
			//_objects.push_back(dupyter);


			for (int i = 0; i < 26; ++i)
			{
				mass_body earth{};
				earth.mass = 5.972e+24; // kg
				earth.radius = 6500'000; // m
				earth.temperature = 300; // K

				double loc_angle = M_PI * 2.0 / 26 * i;
				double vec_angle = loc_angle + M_PI / 2.0;

				earth.location.x = EARTH_SUN_DISTANCE * std::cos(loc_angle);
				earth.location.y = EARTH_SUN_DISTANCE * std::sin(loc_angle);
				earth.velocity.x = EARTH_ORBITAL_VELOCITY * std::cos(vec_angle);
				earth.velocity.y = EARTH_ORBITAL_VELOCITY * std::sin(vec_angle);

				//if (i < 1)
				//{
				//	earth.velocity.x *= 0.99;
				//	earth.velocity.y *= 0.99;

				//	earth.temperature = 1000; // K
				//}
				_objects.push_back(earth);
			}


			mass_body mini_earth{};
			mini_earth.mass = 5.972e+20; // kg  // 1/10000th of the earth mass 
			mini_earth.radius = 3500'000; // m
			mini_earth.temperature = 300; // K
			mini_earth.location.x = 0; // m
			mini_earth.location.y = -247.1e+9;
			mini_earth.velocity.x = -14.78e+3 * 1.2993;
			mini_earth.velocity.y = 0; // m/s
			_objects.push_back(mini_earth); 
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
				a.gravity_force += F_ab;
				b.gravity_force -= F_ab;

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
				}

				auto& dst{ _objects[dst_idx] };

				dst.mass = total_mass;
				dst.radius = std::pow(total_vol_times_N, 1.0 / 3.0);

				dst.location = mass_location / total_mass;
				dst.velocity = mass_velocity / total_mass;
				dst.p0_velocity = dst.velocity;
				dst.p0_acceleration = mass_accel / total_mass;
				dst.temperature = 3000.0;
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
			for (auto& o: _objects)
			{
				o.iterate(TIME_DELTA);
			}
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
