#pragma once

#include <algorithm>
#include <functional>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <sstream>

#include <immintrin.h>

namespace gravity
{
	struct vec3d
	{
		__m256d v;

		vec3d() 
			: v{ _mm256_setzero_pd() }
		{ 
		}

		vec3d(const __m256d& _v)
			: v{ _v }
		{

		}

		vec3d(double _x, double _y, double _z)
			: v{ _mm256_set_pd(0, _z, _y, _x) }
		{
		}

		inline double x() const noexcept
		{
			return v.m256d_f64[0];
		}

		inline double y() const noexcept
		{
			return v.m256d_f64[1];
		}

		inline double z() const noexcept
		{
			return v.m256d_f64[2];
		}

		inline double& x() noexcept
		{
			return v.m256d_f64[0];
		}

		inline double& y() noexcept
		{
			return v.m256d_f64[1];
		}

		inline double& z() noexcept
		{
			return v.m256d_f64[2];
		}

		void save_to(std::ostream& stream) const
		{
			stream.write(reinterpret_cast<const char*>(&v), sizeof(v));
		}

		void load_from(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&v), sizeof(v));
		}

		inline double modulo() const noexcept
		{
			//return std::sqrt(x() * x() + y() * y() + z() * z());
			auto m = _mm256_mul_pd(v, v);
			return std::sqrt(m.m256d_f64[0] + m.m256d_f64[1] + m.m256d_f64[2]);
		}

		inline static vec3d cross(const vec3d& lhs, const vec3d& rhs) noexcept
		{
			// TODO: try to optimize this if you are ever going to use it.
			return {
				lhs.y() * rhs.z() - lhs.z() * rhs.y(),
				lhs.z() * rhs.x() - lhs.x() * rhs.z(), 
				lhs.x() * rhs.y() - lhs.y() * rhs.x(),
			};
		}

		inline static double dot(const vec3d& lhs, const vec3d& rhs) noexcept
		{
			auto m = _mm256_mul_pd(lhs.v, rhs.v);
			return m.m256d_f64[0] + m.m256d_f64[1] + m.m256d_f64[2];
		}

		inline vec3d& operator-=(const vec3d& rhs) noexcept
		{
			v = _mm256_sub_pd(v, rhs.v);
			return *this;
		}

		inline vec3d& operator+=(const vec3d& rhs) noexcept
		{
			v = _mm256_add_pd(v, rhs.v);
			return *this;
		}
	};

	inline vec3d operator-(const vec3d& lhs, const vec3d& rhs) noexcept
	{
		return { _mm256_sub_pd(lhs.v, rhs.v) };
	}

	inline vec3d operator+(const vec3d& lhs, const vec3d& rhs) noexcept
	{
		return { _mm256_add_pd(lhs.v, rhs.v) };
	}

	inline vec3d operator*(const vec3d& lhs, double f) noexcept
	{
		auto fmm = _mm256_set1_pd(f);
		return { _mm256_mul_pd(lhs.v, fmm) };
	}

	inline vec3d operator/(const vec3d& lhs, double f) noexcept
	{
		auto fmm = _mm256_set1_pd(f);
		return { _mm256_div_pd(lhs.v, fmm) };
	}

	inline vec3d operator*(double f, const vec3d& rhs) noexcept
	{
		auto fmm = _mm256_set1_pd(f);
		return { _mm256_mul_pd(rhs.v, fmm) };
	}

	inline vec3d operator-(const vec3d& lhs) noexcept
	{
		auto zero = _mm256_set1_pd(0.0);
		return {_mm256_sub_pd(zero, lhs.v) };
	}

	//
	// a wrapper for Kahan algorithm summation of vec3d objects
	//
	struct acc3d
	{
		vec3d value{};
		vec3d compensation{};

		acc3d()
		{
		}

		acc3d(double x, double y, double z)
			: value{ x, y, z }
		{
		}

		inline acc3d& operator+=(const vec3d& input) noexcept
		{
			auto y = input - compensation;
			auto t = value + y;
			compensation = (t - value) - y;
			value = t;
			return *this;
		}

		inline acc3d& operator-=(const vec3d& input) noexcept
		{
			return this->operator+=(-input);
		}

		void save_to(std::ostream& stream) const
		{
			value.save_to(stream);
			compensation.save_to(stream);
		}

		void load_from(std::istream& stream)
		{
			value.load_from(stream);
			compensation.load_from(stream);
		}
	};

	inline acc3d operator+(const acc3d& acc, const vec3d& input) noexcept
	{
		acc3d ret{ acc };
		ret += input;
		return ret;
	}

	inline acc3d operator-(const acc3d& acc, const vec3d& input) noexcept
	{
		acc3d ret{ acc };
		ret -= input;
		return ret;
	}
}