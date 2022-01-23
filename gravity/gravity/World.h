//
// Note: use this NASA api in order to obtain the real coordinates for the planets / other solar system bodies: 
//  https://ssd-api.jpl.nasa.gov/doc/horizons.html
// 
// Particular query example to query Mars' (object 499) location relative to the Sun (@sun), using the 
// Cartesian coordinates in International Celestial Reference Frame: 
// 
// https://ssd.jpl.nasa.gov/api/horizons.api?format=text&COMMAND='499'&OBJ_DATA='YES'&MAKE_EPHEM='YES'&EPHEM_TYPE='VECTORS'&CENTER='@sun'&START_TIME='2021-12-01'&STOP_TIME='2022-01-21'&STEP_SIZE='15%20d'&OUT_UNITS='KM-S'&REF_SYSTEM='ICRF'&VEC_TABLE='2'
// 
// Other bodies: 
//  199  Mercury
//  299  Venus                                                                
//  399  Earth
//      301 Moon 
//  499  Mars 
//      401 Phobos
//      402 Deimos
//  599  Jupiter 
//      501 Io
//      502 Europa
//      ...
//      550 Herse (2003J17)
//      ...
//  699  Saturn
//      601 Mimas 
//      602 Enceladus 
//      ...
//  799  Uranus
//      701 ..
//  899  Neptune
//      801 ..
//  999  Pluto
//      901 Charon


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
			_objects.set_simulation_start_in_epoch_time_millis(1638316800LLU * 1000); // 2021-12-01 00:00:00 UTC 

			double X, Y, Z, VX, VY, VZ; // some ugly hack to make the numbers below a bit more readable 

			_objects.register_body({"The Sun", 1988500e24, 696000, 1000000, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 });

			_objects.register_body({ "Mercury", 3.302e23, 2440, 400,  
				X = -2.167664834454452E+07, Y = -6.619159190648106E+07, Z = -3.420692631296203E+06,
				VX = 3.650927987393379E+01, VY = -1.273914346337067E+01, VZ = -4.389920191190015E+00 });

			_objects.register_body({ "Venus", 48.685e23, 6051.84, 400,  
				X = 7.576313873684648E+07, Y = 7.711607191532642E+07, Z = -3.313487956947327E+06,
				VX = -2.508851408159857E+01, VY = 2.439292937054329E+01, VZ = 1.782524647980090E+00 });

			_objects.register_body({ "Earth", 5.97219e24, 6371.01, 30,
				 X = 5.358615709453598E+07, Y = 1.374511007334921E+08, Z = -7.098000273063779E+03,
				 VX = -2.824425323200066E+01, VY = 1.071888568481009E+01, VZ = 5.551504930916273E-040 });
			_objects.register_body({ "Moon", 7.349e22, 1737.53, 30,
				  X = 5.324727782955997E+07, Y = 1.373107538738163E+08, Z = 1.427581423602998E+04,
				  VX = -2.778205246706115E+01, VY = 9.758075795843698E+00, VZ = -7.964689569136452E-02 });

			_objects.register_body({ "Mars", 6.4171e23, 3389.92, 30,
			   X = -1.800625404850776E+08, Y = -1.519509644922584E+08, Z = 1.232371251878612E+06,
			   VX = 1.653193518248757E+01, VY = -1.644419258786372E+01, VZ = -7.501597892699268E-01 });
			_objects.register_body({ "Phobos", 1.08e20, 12, 30,
				X = -1.800633484449605E+08, Y = -1.519415763992660E+08, Z = 1.233504578332104E+06,
				VX = 1.463815474957840E+01, VY = -1.669734314209153E+01, VZ = 1.490280944102649E-01 });
			_objects.register_body({ "Deimos", 1.80e20, 7, 30,
				X = -1.800725291760565E+08, Y = -1.519306722879699E+08, Z = 1.238569708517231E+06,
				VX = 1.544205807892980E+01, VY = -1.711382987552074E+01, VZ = -3.136605973651667E-01 });

			_objects.register_body({ "Jupiter", 189818.722e22, 71492, 30,
				X = 6.838721286912214E+08, Y = -3.024806468423285E+08, Z = -1.404409810935293E+07,
				VX = 5.133912400306891E+00, VY = 1.257833242624969E+01, VZ = -1.670642775002857E-01});
			// TODO: add major jupiter moons 

			_objects.register_body({ "Saturn", 5.6834e26, 58232, 30,
				X = 1.024053765137041E+09, Y = -1.075128773787984E+09, Z = -2.206167213916075E+07,
				VX = 6.463717901383691E+00, VY = 6.654665277163426E+00, VZ = -3.733248248209207E-01 });

			// TODO: add major saturn moons 

			_objects.register_body({ "Uranus", 86.813e24, 25362, 30,
				X = 2.166402843059769E+09, Y = 2.003850686154429E+09, Z = -2.063057646324039E+07,
				VX = -4.670082502937194E+00, VY = 4.694611299968432E+00, VZ = 7.781814338959481E-02 });
			// same

			_objects.register_body({ "Neptune", 102.409e24, 24624, 30,
				X = 4.431140574776667E+09, Y = -6.264926905311370E+08, Z = -8.922589280170983E+07,
				VX = 7.301938467449688E-01, VY = 5.427938915323065E+00, VZ = -1.284171598559747E-01 });
			// same

			_objects.register_body({ "Pluto", 1.307e22, 1188.3, 30,
				X = 2.249475791696351E+09, Y = -4.628093902163340E+09, Z = -1.551654004126823E+08,
				VX = 5.022668067623437E+00, VY = 1.200137128287457E+00, VZ = -1.599130583011552E+00 });
			_objects.register_body({ "Charon", 1.53e21, 606, 30,
				X = 2.249461717304943E+09, Y = -4.628102738753292E+09, Z = -1.551550140308864E+08,
				VX = 5.057298851302190E+00, VY = 1.342620937103221E+00, VZ = -1.430982763083136E+00 });

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
		bool iterate()  noexcept
		{
			return _objects.iterate();
        }

		int64_t current_iteration() const noexcept
		{
			return _objects.current_iteration();
		}

		uint64_t current_time_epoch_millis() const noexcept
		{
			return _objects.current_time_epoch_millis();
		}

		void align_observers_frame_of_reference() noexcept
		{
			_objects.align_observers_frame_of_reference();
		}

		void set_time_delta(double time_delta)
		{
			_objects.set_time_delta(time_delta);
		}

		bool load_from_csv(std::string input_file)
		{
			if (!input_file.empty())
				return _objects.load_from_csv(input_file);
			else
				init_planets();
			return true;
		}

		void set_output_csv(std::string output_file)
		{
			_objects.set_output_csv(output_file);
		}

		void set_report_every(uint64_t report_every)
		{
			_objects.set_report_every(report_every);
		}

		void set_max_iterations(uint64_t max_iterations)
		{
			_objects.set_max_iterations(max_iterations);
		}
    };
}
