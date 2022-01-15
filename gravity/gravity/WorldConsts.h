#pragma once

namespace gravity
{
	// Simulation step 
	static constexpr double TIME_DELTA{ 10.0 * 10.0 }; // seconds
//	static constexpr double TIME_DELTA{ 1 }; // seconds


	static constexpr double GRAVITATIONAL_CONSTANT = 6.67430e-11; // m*m*m/(kg*s*s)

	static constexpr double ONE_A_U{ 1.496e+11 }; // meters 
	static constexpr double JUPUTER_ORBIT_RADIUS{ 778.5e+9 };

	static constexpr double SUN_MASS{ 1.989e+30 };
	static constexpr double SUN_RADIUS{ 696340'000 };

	static constexpr double EARTH_MASS{ 5.972e+24 };
	static constexpr double EARTH_RADIUS{ 6500'000 };

	static constexpr double JUPYTER_MASS{ 1.898e+27 };
	static constexpr double JUPYTER_RADIUS{ 69911'000 };


	static constexpr double DECLARE_ESCAPED_AT_DISTANCE{ 10000 * ONE_A_U };
}