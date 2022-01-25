#pragma once

#include "vec3d.h"

namespace gravity 
{

//
// a wrapper for Kahan algorithm summation of vec3d_pd objects
//
template <typename TValue>
struct acc
{
	TValue value{};
	TValue compensation{};

	acc()
	{
	}

	acc(const TValue& v)
		: value{ v }
	{
	}

	acc(const acc<TValue>& a) = default;

	inline acc<TValue>& operator+=(const TValue& input) noexcept
	{
		auto y = input - compensation;
		auto t = value + y;
		compensation = (t - value) - y;
		value = t;
		return *this;
	}

	void save_to(std::ostream& stream) const
	{
		stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
		stream.write(reinterpret_cast<const char*>(&compensation), sizeof(compensation));
	}

	void load_from(std::istream& stream)
	{
		stream.read(reinterpret_cast<char*>(&value), sizeof(value));
		stream.read(reinterpret_cast<char*>(&compensation), sizeof(compensation));
	}
};

template <typename TValue>
inline acc<TValue> operator+(const acc<TValue>& a, const TValue& input) noexcept
{
	acc<TValue> ret{ a };
	ret += input;
	return ret;
}

struct acc3d : public acc<vec3d_pd>
{
	acc3d(double x, double y, double z)
	{
		this->acc<vec3d_pd>::value = { x, y, z };
		this->acc<vec3d_pd>::compensation = { 0.0, 0.0, 0.0 };
	}

	explicit acc3d(const vec3d_pd& vec)
	{
		this->acc<vec3d_pd>::value = vec;
		this->acc<vec3d_pd>::compensation = { 0.0, 0.0, 0.0 };
	}

	acc3d() = default;
	acc3d(const acc3d& other) = default;
};

inline acc3d operator+(const acc3d& a, const vec3d_pd& input) noexcept
{
	acc3d ret{ a };
	ret += input;
	return ret;
}

}