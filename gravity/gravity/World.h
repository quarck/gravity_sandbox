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
			_objects.register_body({ "Io", 8931900e16, 1821.49, 30,
				X = 6.834917902827756E+08, Y = -3.026595705531766E+08, Z = -1.405637860602486E+07,
				VX = 1.259070104859374E+01, VY = -3.124519485827986E+00, VZ = -6.163496998839197E-01 });
			_objects.register_body({ "Europa", 4799800e16, 1560.8, 30,
				X = 6.834119304468290E+08, Y = -3.029764491775714E+08, Z = -1.407309409436847E+07,
				VX = 1.508979221346110E+01, VY = 3.285138263455496E+00, VZ = -2.938644431332047E-01 });
			_objects.register_body({ "Ganymede", 14819000e16, 2631.2, 30,
				X = 6.829700380052605E+08, Y = -3.019006470123901E+08, Z = -1.403442225508019E+07,
				VX = -7.337187217445831E-01, VY = 3.449873825163968E+00, VZ = -5.955918412231824E-01 });
			_objects.register_body({ "Callisto", 10759000e16, 2410.3, 30,
				X = 6.849249581284509E+08, Y = -3.040332547654980E+08, Z = -1.407901783269997E+07,
				VX = 1.191669266239755E+01, VY = 1.723570447211361E+01, VZ = 7.090279128984012E-02 });

			_objects.register_body({ "Saturn", 5.6834e26, 58232, 30,
				X = 1.024053765137041E+09, Y = -1.075128773787984E+09, Z = -2.206167213916075E+07,
				VX = 6.463717901383691E+00, VY = 6.654665277163426E+00, VZ = -3.733248248209207E-01 });
			_objects.register_body({ "Mimas", 3.75e19, 198.8, 30,
				X = 1.024061569514297E+09, Y = -1.074967199686903E+09, Z = -2.215107703973424E+07,
				VX = -7.849060920010291E+00, VY = 7.889466459387394E+00, VZ = 3.817537891840139E-02 });
			_objects.register_body({ "Enceladus", 10.805e19, 252.3, 30,
				X = 1.023905220959770E+09, Y = -1.075287520619336E+09, Z = -2.196412787925708E+07,
				VX = 1.623556350987166E+01, VY = -7.694152882365580E-01, VZ = 2.569202269747525E+00 });
			_objects.register_body({ "Tethys", 61.76e19, 536.3, 30,
				X = 1.024347454375713E+09, Y = -1.075126353886991E+09, Z = -2.208566802130806E+07,
				VX = 5.949162186789012E+00, VY = 1.667582608915526E+01, VZ = -5.680160053328846E+00 });
			_objects.register_body({ "Dione", 109.572e19, 562.5, 30,
				X = 1.023677372396493E+09, Y = -1.075106702772919E+09, Z = -2.203654950615901E+07,
				VX = 6.273963860424415E+00, VY = -2.207801980011506E+00, VZ = 4.289347461558116E+00 });
			_objects.register_body({ "Rhea", 230.9e19, 764.5, 30,
				X = 1.023639682532061E+09, Y = -1.074825419418305E+09, Z = -2.218324751626205E+07,
				VX = 1.253760284754933E+00, VY = 9.372996797301310E-01, VZ = 3.096741005227620E+00 });
			_objects.register_body({ "Titan", 13455.3e19, 2575.5, 30,
				X = 1.023136477714555E+09, Y = -1.074337054407217E+09, Z = -2.237847106385607E+07,
				VX = 2.738681118489664E+00, VY = 3.300594873075072E+00, VZ = 1.727168731391114E+00 });
			_objects.register_body({ "Iapetus", 180.59e19, 734.5, 30,
				X = 1.020467407011307E+09, Y = -1.075166028661907E+09, Z = -2.132825212854689E+07,
				VX = 6.621413286549470E+00, VY = 3.558477452746217E+00, VZ = 3.112450066270458E-01 });
			_objects.register_body({ "Phoebe", 0.8289e19, 106.6, 30,
				X = 1.036422191619940E+09, Y = -1.068105946502307E+09, Z = -2.357338946881741E+07,
				VX = 7.023637467860023E+00, VY = 5.224998990586824E+00, VZ = -4.459519938544869E-01 });

			_objects.register_body({ "Uranus", 86.813e24, 25362, 30,
				X = 2.166402843059769E+09, Y = 2.003850686154429E+09, Z = -2.063057646324039E+07,
				VX = -4.670082502937194E+00, VY = 4.694611299968432E+00, VZ = 7.781814338959481E-02 });
			_objects.register_body({ "Miranda", 0.659e20, 235, 30,
				X = 2.166310266894187E+09, Y = 2.003849637890084E+09, Z = -2.072139420120418E+07,
				VX = -9.170537947822899E+00, VY = 6.597350560512488E+00, VZ = 4.645603916998672E+00 });
			_objects.register_body({ "Ariel", 13.53e20, 579, 30,
				X = 2.166585050207122E+09, Y = 2.003817146301778E+09, Z = -2.058492043887448E+07,
				VX = -3.524209825336937E+00, VY = 3.710862049955818E+00, VZ = -5.223615162170813E+00 });
			_objects.register_body({ "Umbriel", 11.72e20, 584.7, 30,
				X = 2.166313922345208E+09, Y = 2.003903836444845E+09, Z = -2.038679240410388E+07,
				VX = -3.694929400822213E-01, VY = 3.995802560775008E+00, VZ = 1.805252844419170E+00 });
			_objects.register_body({ "Titania", 35.27e20, 788.9, 30,
				X = 2.166297108950608E+09, Y = 2.003931997899418E+09, Z = -2.021473715733314E+07,
				VX = -1.223310387933968E+00, VY = 4.078031174089427E+00, VZ = 1.075296184144253E+00 });
			_objects.register_body({ "Oberon", 30.14e20, 761.4, 30,
				X = 2.166969509460758E+09, Y = 2.003722144638722E+09, Z = -2.066695986005986E+07,
				VX = -4.959165384355544E+00, VY = 4.313649246810850E+00, VZ = -3.044055651160569E+00 });

			_objects.register_body({ "Neptune", 102.409e24, 24624, 30,
				X = 4.431140574776667E+09, Y = -6.264926905311370E+08, Z = -8.922589280170983E+07,
				VX = 7.301938467449688E-01, VY = 5.427938915323065E+00, VZ = -1.284171598559747E-01 });
			_objects.register_body({ "Triton",  2139000e16, 1352.6, 30,
				X = 4.431005555431501E+09, Y = -6.263127113797166E+08, Z = -8.895160380449900E+07,
				VX = 4.105334670785398E+00, VY = 8.229355809460420E+00, VZ = -3.051893170109001E-01 });
			_objects.register_body({ "Nereid", 2700e16, 170, 30,
				X = 4.428939523737038E+09, Y = -6.204323927818954E+08, Z = -8.894537849520865E+07,
				VX = 3.709513388874054E-01, VY = 4.566507252781917E+00, VZ = -2.069986586383070E-01 });
			_objects.register_body({ "Proteus",  4400e16, 208, 30,
				X = 4.431245538462396E+09, Y = -6.265099013832287E+08, Z = -8.927607632156128E+07,
				VX = 2.750421406918723E+00, VY = 1.256445792215055E+01, VZ = 1.643010925226763E+00 });
			_objects.register_body({ "Larissa",  420e16, 100, 30,
				X = 4.431085627109832E+09, Y = -6.265413034732841E+08, Z = -8.922050736469600E+07,
				VX = 6.084925934337120E+00, VY = -1.150078488926583E+00, VZ = -4.712007971350823E+00 });

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
