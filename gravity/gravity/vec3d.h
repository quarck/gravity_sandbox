#pragma once

#include <algorithm>
#include <functional>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <sstream>

namespace gravity
{
	struct vec3d
	{
		double x{};
		double y{};
		double z{};

		void save_to(std::ostream& stream) const
		{
			stream.write(reinterpret_cast<const char*>(&x), sizeof(x));
			stream.write(reinterpret_cast<const char*>(&y), sizeof(y));
			stream.write(reinterpret_cast<const char*>(&z), sizeof(z));
		}

		void load_from(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&x), sizeof(x));
			stream.read(reinterpret_cast<char*>(&y), sizeof(y));
			stream.read(reinterpret_cast<char*>(&z), sizeof(z));
		}

		inline double modulo() const noexcept
		{
			return std::sqrt(x * x + y * y + z * z);
		}

		inline static vec3d cross(const vec3d& lhs, const vec3d& rhs) noexcept
		{
			return {
				lhs.y * rhs.z - lhs.z * rhs.y,
				lhs.z * rhs.x - lhs.x * rhs.z, 
				lhs.x * rhs.y - lhs.y * rhs.x,
			};
		}

		inline static double dot(const vec3d& lhs, const vec3d& rhs) noexcept
		{
			return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
		}

		inline vec3d& operator-=(const vec3d& rhs) noexcept
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;
			return *this;
		}

		inline vec3d& operator+=(const vec3d& rhs) noexcept
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;
			return *this;
		}
	};

	inline vec3d operator-(const vec3d& lhs, const vec3d& rhs) noexcept
	{
		return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
	}

	inline vec3d operator+(const vec3d& lhs, const vec3d& rhs) noexcept
	{
		return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
	}

	inline vec3d operator*(const vec3d& lhs, double f) noexcept
	{
		return { lhs.x * f, lhs.y * f, lhs.z * f};
	}

	inline vec3d operator/(const vec3d& lhs, double f) noexcept
	{
		return { lhs.x / f, lhs.y / f, lhs.z / f };
	}

	inline vec3d operator*(double f, const vec3d& rhs) noexcept
	{
		return rhs * f;
	}
}