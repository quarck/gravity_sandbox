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

namespace gravity
{ 
    class World
    {
	private:
		static constexpr double GRAVITATIONAL_CONSTANT = 6.67430e-11; // m*m*m/(kg*s*s)

		static constexpr double TIME_DELTA{ 10 }; // seconds

		static constexpr double SQRT_2 = 1.4142135623730950488016887242097f; // unfortunately std::sqrt is not a constexpr function


		static constexpr double EARTH_SUN_DISTANCE{ 147.1e+9 }; // meters 
		static constexpr double EARTH_ORBITAL_VELOCITY{ 29.78e+3 }; // m/s

        int _numWorkerThreads;

        std::vector<mass_body> _objects;
		std::list<std::unordered_set<int>> _collisions;
		std::mutex _collisions_mutex;

        Random _random{};

        std::string _workingFolder;
        bool _workingFolderCreated{ false };

        ThreadGrid _grid;

	public:
        World(
            const std::string& workingFolder,
            int nWorkerThreads
        )
            : _grid(nWorkerThreads)
            , _workingFolder(workingFolder)
            , _numWorkerThreads(nWorkerThreads)
        {
			
			mass_body sun{};
			sun.mass = 1.989e+30 * 1; // kg
			sun.radius = 696340'000; // m
			sun.temperature = 10000000;
			_objects.push_back(sun);


			for (int i = 0; i < 17; ++i)
			{
				mass_body earth{};
				earth.mass = 5.972e+24; // kg
				earth.radius = 6500'000; // m
				earth.temperature = 300; // K

				double loc_angle = M_PI * 2.0 / 17 * i;
				double vec_angle = loc_angle + M_PI / 2.0;

				earth.location.x = EARTH_SUN_DISTANCE * std::cos(loc_angle); 
				earth.location.y = EARTH_SUN_DISTANCE * std::sin(loc_angle);
				earth.velocity.x = EARTH_ORBITAL_VELOCITY * std::cos(vec_angle);
				earth.velocity.y = EARTH_ORBITAL_VELOCITY * std::sin(vec_angle);

				if (i < 1)
				{
					earth.velocity.x *= 0.9999;
					earth.velocity.y *= 0.9999;

					earth.temperature = 1000; // K
				}
				_objects.push_back(earth);
			}

			
			//mass_body mini_earth{};
			//mini_earth.mass = 5.972e+22; // kg
			//mini_earth.radius = 3500'000; // m
			//mini_earth.temperature = 300; // K
			//mini_earth.location.x = 0; // m
			//mini_earth.location.y = -247.1e+9;
			//mini_earth.velocity.x = 14.78e+3;
			//mini_earth.velocity.y = 0; // m/s
			//_objects.push_back(mini_earth); 
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
	
	public:
        void Iterate(long step)  noexcept
        {
            if (step == 0)
                WorldInitialize();

			// Wipe the previous gravity calculations 
			for (auto& o : _objects)
			{
				o.gravity_force = {0.0, 0.0, 0.0};
			}

			if (false)
			{
				concurrency::parallel_for(
					0,
					static_cast<int>(_objects.size()),
					[&](int i) 
					{
						auto& a{_objects[i]};

						for (int j = 0; j < _objects.size(); ++j)
						{
							if (i == j)
								continue;

							auto& b{ _objects[j] };

							auto r_ba = b.location - a.location;
							auto r_modulo = r_ba.modulo();

							if (r_modulo > a.radius + b.radius)
							{
								auto F_ab = r_ba * (GRAVITATIONAL_CONSTANT * a.mass * b.mass / std::pow(r_modulo, 3.0));
								a.gravity_force += F_ab;
								//	b.gravity_force -= F_ab;
							}
							else
							{
								std::lock_guard<std::mutex> l(_collisions_mutex);

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
						}

						a.iterate(TIME_DELTA);
					}					
				);
			}
			else
			{
				// Calculate inter-object gravities
				for (int i = 0; i < _objects.size(); ++i)
				{
					for (int j = 0; j < i; ++j)
					{
						if (j == i)
							continue;

						auto& a{ _objects[i] };
						auto& b{ _objects[j] };

						auto r_ba = b.location - a.location;
						auto r_modulo = r_ba.modulo();

						if (r_modulo > a.radius + b.radius)
						{
							auto F_ab = r_ba * (GRAVITATIONAL_CONSTANT * a.mass * b.mass / std::pow(r_modulo, 3.0));
							a.gravity_force += F_ab;
							b.gravity_force -= F_ab;
						}
						else
						{
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
					}
				}

				for (auto& o : _objects)
				{
					o.iterate(TIME_DELTA);
				}
			}

			if (!_collisions.empty())
			{
				std::vector<bool> idx_to_remove( _objects.size() );

				for (auto& c : _collisions)
				{
					vec3d mass_location{};
					vec3d mass_velocity{};
					vec3d mass_accel{}; // for p0 only

					double total_mass{};

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
					}

					auto& dst{ _objects[dst_idx]};

					dst.mass = total_mass;
					dst.radius = std::pow(total_mass, 1.0 / 3.0) * 0.1;

					dst.location = mass_location / total_mass;
					dst.velocity = mass_velocity / total_mass;
					dst.p0_velocity = dst.velocity;
					dst.p0_acceleration = mass_accel / total_mass;
					dst.temperature = 3000.0;
				}

				_collisions.clear();

				for (int idx = _objects.size() - 1; idx >= 0; idx--)
				{
					if (idx_to_remove[idx])
					{
						_objects.erase(_objects.begin() + idx);
					}
				}
			}
        }

	private:


        void WorldInitialize()  noexcept
        {
        }
    };
}
