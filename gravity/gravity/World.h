// TODO: 

// report periodically - the total kinetic + potential energy of the system in respect to the sun's frame of reference 

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

#include "vec3d.h"
#include "Random.h"
#include "ThreadGrid.h"

#include "Utils.h"

#include "WorldConsts.h"
#include "WorldObjects.h"

#include "Log.h"

namespace gravity
{ 
    class World
    {
		gravity_struct _objects;
        Random _random{};

	public:
        World()
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
			double orbit_direction, 
			double mass_variation, 
			double location_variation_rad, 
			int num_subset = 0
		)
		{
			auto up_to = num_subset > 0 ? num_subset : num_planets;

			double total_mass{ 0 };
			for (int i = 0; i < up_to; ++i)
			{
				mass_body planet{};

				planet.mass = mass +_random.Next(-mass_variation / 2.0, mass_variation / 2.0); // kg
				planet.radius = radius; // m
				planet.temperature = 300; // K

				total_mass += planet.mass;

				double loc_angle = M_PI * 2.0 / num_planets * i +_random.Next(-location_variation_rad / 2.0, location_variation_rad / 2.0);
				double vec_angle = loc_angle + M_PI / 2.0;

				double V = orbital_velocity(sun_mass_adjusted, orbit_radius);

				planet.location.value.x() = orbit_radius * std::cos(loc_angle);
				planet.location.value.y() = orbit_radius * std::sin(loc_angle);
				planet.location.value.z() = 0;
				planet.velocity.value.x() = orbit_direction * V * std::cos(vec_angle);
				planet.velocity.value.y() = orbit_direction * V * std::sin(vec_angle);
				planet.velocity.value.z() = 0;

				_objects.register_body(planet);
			}

			return total_mass;
		}

		void init_planets()
		{
			// Motivation: validate this: 
			// https://planetplanet.net/2017/05/03/the-ultimate-engineered-solar-system/
			// (and: https://habr.com/ru/post/598687/)

			mass_body sun{};
			sun.mass = SUN_MASS; // kg
			sun.radius = SUN_RADIUS; // m
			sun.temperature = 10000000;
			_objects.register_body(sun);

			//mass_body jupyter{};
			//jupyter.mass = JUPYTER_MASS; // kg
			//jupyter.radius = JUPYTER_RADIUS; // m
			//jupyter.location.x() = JUPUTER_ORBIT_RADIUS; // m
			//jupyter.velocity.y() = orbital_velocity(sun.mass, JUPUTER_ORBIT_RADIUS);  // 13.06e+3; // m/s
			//jupyter.temperature = 273;
			//_objects.push_back(jupyter);

			//{
			//	mass_body dupyter{};
			//	dupyter.mass = JUPYTER_MASS * 32; // kg
			//	dupyter.radius = JUPYTER_RADIUS; // m
			//	dupyter.location.value.x() = ONE_A_U * 185.0 / 189.0; // m
			//	dupyter.velocity.value.y() = orbital_velocity(sun.mass, dupyter.location.value.x()) * 1.000001;
			//	dupyter.temperature = 273;
			//	_objects.register_body(dupyter);
			//}

			double sun_mass_adjusted = sun.mass;// +neutron_star.mass;

			auto mass_var = EARTH_MASS * 0.9;
			auto loc_var = M_PI / 4.0;

			double d_odd = 1.0;
			double d_evn = 1.0;

			//sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 170.0 / 189.0, d_odd, mass_var, loc_var);
			//sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 185.0 / 189.0, d_evn, mass_var, loc_var);
			//sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 200.0 / 189.0, d_odd, mass_var, loc_var);
			//sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 216.0 / 189.0, d_evn, mass_var, loc_var);
			//sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 234.0 / 189.0, d_odd, mass_var, loc_var);
			//sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 254.0 / 189.0, d_evn, mass_var, loc_var);
			//sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 275.0 / 189.0, d_odd, mass_var, loc_var);
			//sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 298.0 / 189.0, d_evn, mass_var, loc_var);

			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52-28, EARTH_MASS, EARTH_RADIUS, ONE_A_U *0.95, d_odd, mass_var, loc_var);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52-18, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 0.97, d_evn, mass_var, loc_var);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52-10, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 1.03, d_odd, mass_var, loc_var);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52-4, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 1.05, d_evn, mass_var, loc_var);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52+4, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 1.09, d_odd, mass_var, loc_var);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52+10, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 1.12, d_evn, mass_var, loc_var);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52+18, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 1.15, d_odd, mass_var, loc_var);
			sun_mass_adjusted += populate_orbit(sun_mass_adjusted, 52+28, EARTH_MASS, EARTH_RADIUS, ONE_A_U * 1.2, d_evn, mass_var, loc_var);

			//mass_body impactor{};
			//impactor.mass = EARTH_MASS / 50 / 50 / 50; // kg 1/50th of the size (130km in radius), 1/125000th of the mass 
			//impactor.radius = 3500'000.0; // m
			//impactor.temperature = 900; // K
			//impactor.location.value.x() = -3.5e+11; // hand picked to produce an impact on the first turn of the orbit
			//impactor.location.value.y() = 9.191e+10;
			//impactor.velocity.value.x() = 40e+3; // 40km/s
			//impactor.velocity.value.y() = 1e+3; // m/s
			//_objects.register_body(impactor);

			_objects.align_observers_frame_of_reference();
		}

        const std::vector<mass_body>& get_objects() const noexcept
        {
            return _objects.get_bodies();
        }

		void save_to(std::ostream& stream)
		{
			_objects.save_to(stream);
		}

		void load_from(std::istream& stream)
		{
			_objects.load_from(stream);
		}
	
	public:
		void iterate()  noexcept
		{
			_objects.iterate();
        }

		int64_t current_iteration() const noexcept
		{
			return _objects.current_iteration();
		}

		void align_observers_frame_of_reference() noexcept
		{
			_objects.align_observers_frame_of_reference();
		}
    };
}
