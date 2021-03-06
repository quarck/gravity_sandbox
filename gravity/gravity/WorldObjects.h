#pragma once

#include <algorithm>
#include <functional>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <sstream>
#include <array>
#include <iostream>

#include <list>
#include <unordered_set>

#include <ppl.h>

#include <intrin.h>
#pragma intrinsic(__rdtsc)

#include "vec3d.h"
#include "kahan.h"

#include "WorldConsts.h"

#include "ThreadGrid.h"



namespace gravity
{
	struct mass_body
	{
		acc3d location{};
		acc3d velocity{};

		acc3d gravity_acceleration{}; 

		double radius{};
		double mass{ 1.0 };
		double mass_G{ 1.0 * GRAVITATIONAL_CONSTANT };
		double temperature{ 300 };

		std::string label{};

		mass_body() = default;

		mass_body(
			const std::string& _label,
			double _mass, double _radius, double temp,
			double x, double y, double z,
			double vx, double vy, double vz
		)
			: label{ _label }
			, mass{ _mass }
			, mass_G{ _mass * GRAVITATIONAL_CONSTANT }
			, radius{ _radius * 1000.0 } // to meters
			, temperature{ temp }
			, location { x * 1000.0, y * 1000.0, z * 1000.0 } // to meters
			, velocity { vx * 1000.0, vy * 1000.0, vz * 1000.0 } // to meters /s
		{
		}

		static std::string get_csv_header()
		{
			std::ostringstream str;

			str << "iteration,"
				<< "epoch_millis,"
				<< "body_idx," 
				<< "label,"
				<< "mass,"
				<< "radius_km,"
				<< "temperature,"
				<< "location_x_km,"
				<< "location_y_km,"
				<< "location_z_km,"
				<< "velocity_x_kms,"
				<< "velocity_y_kms,"
				<< "velocity_z_kms";

			return str.str();
		}

		std::string to_csv_line(uint64_t iteration, uint64_t epoch_millis, int body_idx)
		{
			std::ostringstream str;
			
			str << std::setprecision(20);

			str << iteration << ","
				<< epoch_millis << ","
				<< body_idx << ","
				<< label << ","
				<< mass << ","
				<< (radius / 1000.0) << ","
				<< temperature << ","
				<< (location.value.x() / 1000.0) << ","
				<< (location.value.y() / 1000.0) << ","
				<< (location.value.z() / 1000.0) << ","
				<< (velocity.value.x() / 1000.0) << ","
				<< (velocity.value.y() / 1000.0) << ","
				<< (velocity.value.z() / 1000.0);

			return str.str();
		}

		bool from_csv_line(const std::string& line, uint64_t& epoch_millis)
		{
			std::istringstream str{ line };

			std::string iteration_str;
			std::string epoch_millis_str;
			std::string body_idx_str;

			std::string mass_str, radius_str, temperature_str,
				location_x_str, location_y_str, location_z_str,
				velocity_x_str, velocity_y_str, velocity_z_str;

			if (!std::getline(str, iteration_str, ',') ||
				!std::getline(str, epoch_millis_str, ',') ||
				!std::getline(str, body_idx_str, ',') ||
				!std::getline(str, label, ',') ||
				!std::getline(str, mass_str, ',') ||
				!std::getline(str, radius_str, ',') ||
				!std::getline(str, temperature_str, ',') ||
				!std::getline(str, location_x_str, ',') ||
				!std::getline(str, location_y_str, ',') ||
				!std::getline(str, location_z_str, ',') ||
				!std::getline(str, velocity_x_str, ',') ||
				!std::getline(str, velocity_y_str, ',') ||
				!std::getline(str, velocity_z_str, ',') 
				)
			{
				return false;
			}

			epoch_millis = std::stoull(epoch_millis_str);

			mass = std::stod(mass_str);
			mass_G = mass * GRAVITATIONAL_CONSTANT;
			radius = std::stod(radius_str) * 1000.0;
			temperature = std::stod(temperature_str);

			location = {
				std::stod(location_x_str) * 1000.0,
				std::stod(location_y_str) * 1000.0,
				std::stod(location_z_str) * 1000.0
			};

			velocity = {
				std::stod(velocity_x_str) * 1000.0,
				std::stod(velocity_y_str) * 1000.0,
				std::stod(velocity_z_str) * 1000.0
			};

			return true;
		}

		void save_to(std::ostream & stream)
		{
			location.save_to(stream);
			velocity.save_to(stream);

			gravity_acceleration.save_to(stream);

			stream.write(reinterpret_cast<const char*>(&radius), sizeof(radius));
			stream.write(reinterpret_cast<const char*>(&mass), sizeof(mass));
			stream.write(reinterpret_cast<const char*>(&temperature), sizeof(temperature));
		}

		void load_from(std::istream & stream)
		{
			location.load_from(stream);
			velocity.load_from(stream);

			gravity_acceleration.load_from(stream);
			
			stream.read(reinterpret_cast<char*>(&radius), sizeof(radius));
			stream.read(reinterpret_cast<char*>(&mass), sizeof(mass));
			stream.read(reinterpret_cast<char*>(&temperature), sizeof(temperature));

			mass_G = mass * GRAVITATIONAL_CONSTANT;
		}
	};

	enum class integration_method
	{
		linear,
		linear_kahan,
		quadratic,
		quadratic_kahan,
		cubic,
		cubic_kahan,
	};

	//
	// a "structure" in a cosmological term - a set of bodies connected by the gravitational attraction 
	//
	template <integration_method method>
	class gravity_struct
	{
	public:
		using mass_bodies = std::vector<mass_body>;

		static constexpr int NUM_GENERATIONS{ 4 };

	private:
		// 4 generations: 
		// T+1 - next
		// T+0 - current
		// T-1
		// T-2 
		//
		// current generation index is (_current_iteration % 4), and the other generations are moving accordingly
		//

		std::array<mass_bodies, NUM_GENERATIONS> _bodies_gens;

		std::list<std::unordered_set<int>> _collisions;
		std::mutex _collisions_mutex;

		uint64_t _report_every_n_iterations{ 0 };
		uint64_t _max_iterations{ 0 };
		uint64_t _current_iteration{ 0 };

		uint64_t _simulation_start_in_epoch_time_millis{ 0 };

		uint64_t _mt_ticks_per_n_iter{ 1 };
		uint64_t _st_ticks_per_n_iter{ 2 };

		static constexpr uint64_t PERFORMANCE_PROFILING_CYCLE{ 8192 };
		static constexpr uint32_t PERFORMANCE_PROFILING_N{ 8 };

		double _time_delta{ 0.1 };
		double _time_delta_times_1_2{ _time_delta / 2.0 };
		double _time_delta_times_1_12{ _time_delta / 12.0 };
		double _time_delta_times_1_24{ _time_delta / 24.0 };

		std::string _report_file{};
		std::string _report_centre{};

		bool _first_report{ true };

	private: 

		mass_bodies& get_generation(int gen) noexcept
		{
			return _bodies_gens[(_current_iteration + NUM_GENERATIONS + gen) % NUM_GENERATIONS];
		}

		const mass_bodies& get_generation(int gen) const noexcept
		{
			return _bodies_gens[(_current_iteration + NUM_GENERATIONS + gen) % NUM_GENERATIONS];
		}

		[[noreturn]] void on_bodies_vector_mismatch() noexcept 
		{
			std::cerr << "Internal error: inconsistency in size of _bodies_gens vectors" << std::endl;
			std::terminate();
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


		void check_generations_size_consistency()
		{
			for (int i = 1; i < NUM_GENERATIONS; ++i)
			{
				if (this->_bodies_gens[0].size() != this->_bodies_gens[i].size())
				{
					on_bodies_vector_mismatch();
				}
			}
		}

		void remove_at(int idx)
		{
			for (auto& generation : _bodies_gens)
			{
				generation.erase(generation.begin() + idx);
			}
		}

		void remove_at(std::vector<bool>& indexes)
		{
			for (int idx = static_cast<int>(_bodies_gens[0].size()) - 1; idx >= 0; --idx)
			{
				if (indexes[idx])
				{
					remove_at(idx);
				}
			}
		}

		void iterate_collision_merges() noexcept
		{
			std::lock_guard l{ _collisions_mutex };

			if (_collisions.empty())
				return;

			check_generations_size_consistency();

			const auto num_bodies{ _bodies_gens[0].size() };

			std::vector<bool> idx_to_remove(num_bodies);

			auto& curr_gen = get_generation(0);
			auto& next_gen = get_generation(1);
			auto& prev0_gen = get_generation(-1);
			auto& prev1_gen = get_generation(-2);

			for (auto& collision : _collisions)
			{
				acc3d mass_location{};	// to calculate the resulting centre of mass 
				acc3d mass_velocity{}; // to calculate the resulting momentum of motion 
				acc3d force{};

				std::string resulting_label;

				acc<double> total_mass{};

				acc<double> total_vol_times_N{};

				double max_temp{ 0 };

				int dst_idx = *collision.begin();
				for (auto& idx : collision)
				{
					if (idx != dst_idx)
					{
						idx_to_remove[idx] = true;
					}

					auto& body{ curr_gen[idx] };

					mass_location  +=  body.location.value * body.mass;
					mass_velocity  += body.velocity.value * body.mass;
					force += body.gravity_acceleration.value * body.mass;

					total_mass += body.mass;

					total_vol_times_N += std::pow(body.radius, 3.0);
					max_temp = std::max(max_temp, body.temperature);

					if (!resulting_label.empty())
						resulting_label += "+";
					resulting_label += !body.label.empty() ? body.label : std::to_string(idx);
				}

				auto& c_dst{ curr_gen[dst_idx] };
				auto& n_dst{ next_gen[dst_idx] };
				auto& p0_dst{ prev0_gen[dst_idx] };
				auto& p1_dst{ prev1_gen[dst_idx] };

				c_dst.mass = total_mass.value;
				c_dst.mass_G = c_dst.mass * GRAVITATIONAL_CONSTANT;
				c_dst.radius = std::pow(total_vol_times_N.value, 1.0 / 3.0);
				c_dst.location.value = mass_location.value / total_mass.value;
				c_dst.velocity.value = mass_velocity.value / total_mass.value;
				c_dst.gravity_acceleration = acc3d{ force.value / total_mass.value };
				c_dst.temperature = std::max(max_temp, 3000.0); // boiling planet's guts 
				c_dst.label = resulting_label;

				// TODO: add labels here for labelled objects
				n_dst = c_dst;
				p0_dst = c_dst;
				p1_dst = c_dst;
			}

			_collisions.clear();
			remove_at(idx_to_remove);
		}

		void check_for_escaped_bodies() noexcept
		{
			check_generations_size_consistency();

			const auto num_bodies{ _bodies_gens[0].size() };

			std::vector<bool> idx_to_remove(num_bodies);

			auto& curr_gen = get_generation(0);

			for (int i = 0; i < curr_gen.size(); ++ i)
			{
				const auto& b = curr_gen[i];

				if (b.location.value.modulo() > DECLARE_ESCAPED_AT_DISTANCE)
				{
					idx_to_remove[i] = true;
				}
			}

			remove_at(idx_to_remove);
		}


		void iterate_gravity_forces(
			mass_bodies& prev1_gen,
			mass_bodies& prev0_gen,
			mass_bodies& current_gen,
			mass_bodies& next_gen
		) noexcept
		{
			if (current_gen.size() != next_gen.size())
			{
				on_bodies_vector_mismatch();
			}

			for (auto& o : next_gen)
			{
				o.gravity_acceleration = { 0.0, 0.0, 0.0 };
			}

			for (int i = 0; i < current_gen.size(); ++i)
			{
				const auto& curr_a{ current_gen[i] };
				auto& next_a{ next_gen[i] };

				for (int j = i+1; j < current_gen.size(); ++j)
				{
					const auto& curr_b{ current_gen[j] };
					auto& next_b{ next_gen[j] };

					auto r_ba = curr_b.location.value - curr_a.location.value;
					auto r_modulo = r_ba.modulo();

					if (r_modulo > curr_a.radius + curr_b.radius)
					{
						auto r_mod_pow_3 = std::pow(r_modulo, 3.0);

						next_a.gravity_acceleration += r_ba * (curr_b.mass_G / r_mod_pow_3);
						next_b.gravity_acceleration += -r_ba * (curr_a.mass_G / r_mod_pow_3);

						if (r_modulo < curr_a.radius * 10)
						{
							next_a.temperature = std::max(curr_a.temperature, 1000.0); // tidal forces stirr the mantel, floor is lava in the whole planet now
						}

						if (r_modulo < curr_b.radius * 10)
						{
							next_b.temperature = std::max(curr_b.temperature, 1000.0); // tidal forces stirr the mantel, floor is lava in the whole planet now
						}
					}
					else
					{
						register_collisions(i, j);
					}
				}
			}

			if (_current_iteration == 0)
			{
				prev1_gen = next_gen;
				prev0_gen = next_gen;
				current_gen = next_gen;
			}

			for (int i = 0; i < current_gen.size(); ++i)
			{
				iterate_move(prev1_gen[i], prev0_gen[i], current_gen[i], next_gen[i]);
			}
		}
		
		void iterate_gravity_forces_mt(
			const mass_bodies& prev1_gen,
			const mass_bodies& prev0_gen,
			const mass_bodies& current_gen,
			mass_bodies& next_gen,
			int i
		) noexcept
		{
			const auto& curr_a{ current_gen[i] };
			auto& next_a{ next_gen[i] };

			next_a.gravity_acceleration = { 0.0, 0.0, 0.0 };

			for (int j = 0; j < current_gen.size(); ++j)
			{
				if (i == j)
				{
					continue;
				}

				const auto& curr_b{ current_gen[j] };

				auto r_ba = curr_b.location.value - curr_a.location.value;
				auto r_modulo = r_ba.modulo();

				if (r_modulo > curr_a.radius + curr_b.radius)
				{
					next_a.gravity_acceleration += r_ba * (curr_b.mass_G / std::pow(r_modulo, 3.0));

					if (r_modulo < curr_a.radius * 10)
					{
						next_a.temperature = std::max(curr_a.temperature, 1000.0); // tidal forces stirr the mantel, floor is lava in the whole planet now
					}
				}
				else
				{
					register_collisions(i, j);
				}
			}

			iterate_move(prev1_gen[i], prev0_gen[i], curr_a, next_a);
		}

		inline void iterate_linear(const mass_body& current, mass_body& next) noexcept
		{
			next.velocity.value = current.velocity.value + next.gravity_acceleration.value * _time_delta;
			next.location.value = current.location.value + next.velocity.value * _time_delta;
		}

		inline void iterate_linear_kahan(const mass_body& current, mass_body& next) noexcept
		{
			next.velocity = current.velocity + next.gravity_acceleration.value * _time_delta;
			next.location = current.location + next.velocity.value * _time_delta;
		}

		//
		// This method uses the current value - v_0, previous v_1 and the one before v_2 to 
		// interpolate the curve into quadratic equation, then using exact integration on the range from (t-time_delta/2, t+time_delta/2) we
		// find the value of velocity / location change based on a quadratic approximation to the actual function
		// 
		inline void iterate_quadratic(const mass_body& prev0, const mass_body& current, mass_body& next) noexcept
		{
			next.velocity.value = current.velocity.value
				+ (
					25 * next.gravity_acceleration.value
					- 2 * current.gravity_acceleration.value
					+ prev0.gravity_acceleration.value
				) * _time_delta_times_1_24;
			next.location.value = current.location.value + 
				(
					25 * next.velocity.value + 
					- 2 * current.velocity.value 
					+ prev0.velocity.value
				) * _time_delta_times_1_24;
		}

		//
		// Same as above, but using Kahan accumulators for velocity / location 
		//
		inline void iterate_quadratic_kahan(const mass_body& prev0, const mass_body& current, mass_body& next) noexcept
		{
			next.velocity = current.velocity
				+ (
					25 * next.gravity_acceleration.value 
					- 2 * current.gravity_acceleration.value 
					+ prev0.gravity_acceleration.value
				) * _time_delta_times_1_24;
			next.location = current.location +
				(
					25 * next.velocity.value +
					-2 * current.velocity.value
					+ prev0.velocity.value
				) * _time_delta_times_1_24;
		}

		//
		// This method is similar to quadratic, but using more history points and interpolates into cubic function 
		// 
		inline void iterate_cubic(const mass_body& prev1, const mass_body& prev0, const mass_body& current, mass_body& next) noexcept
		{
			next.velocity.value = current.velocity.value +
				(
					26 * next.gravity_acceleration.value
					+ (-5) * current.gravity_acceleration.value
					+ 4 * prev0.gravity_acceleration.value
					+ (-1)* prev1.gravity_acceleration.value
				) * _time_delta_times_1_24;

			next.location.value = current.location.value +
				(
					26 * next.velocity.value 
					+ (-5) * current.velocity.value 
					+ 4 * prev0.velocity.value
					+ (-1) * prev1.velocity.value
				) * _time_delta_times_1_24;
		}

		//
		// This method is similar to quadratic, but using more history points and interpolates into cubic function 
		// 
		inline void iterate_cubic_kahan(const mass_body& prev1, const mass_body& prev0, const mass_body& current, mass_body& next) noexcept
		{
			next.velocity = current.velocity +
				(
					26 * next.gravity_acceleration.value
					+ (-5) * current.gravity_acceleration.value
					+ 4 * prev0.gravity_acceleration.value
					+ (-1) * prev1.gravity_acceleration.value
				) * _time_delta_times_1_24;

			next.location = current.location +
				(
					26 * next.velocity.value
					+ (-5) * current.velocity.value
					+ 4 * prev0.velocity.value
					+ (-1) * prev1.velocity.value
				) * _time_delta_times_1_24;
		}


		void iterate_move(const mass_body& prev1, const mass_body& prev0, const mass_body& current, mass_body& next) noexcept
		{
			if constexpr (method == integration_method::linear)
			{
				iterate_linear(current, next);
			}
			else if constexpr (method == integration_method::linear_kahan)
			{
				iterate_linear_kahan(current, next);
			}
			else if constexpr (method == integration_method::quadratic)
			{
				iterate_quadratic(prev0, current, next);
			}
			else if constexpr (method == integration_method::quadratic_kahan)
			{
				iterate_quadratic_kahan(prev0, current, next);
			}
			else if constexpr (method == integration_method::cubic)
			{
				iterate_cubic(prev1, prev0, current, next);
			}
			else if constexpr (method == integration_method::cubic_kahan)
			{
				iterate_cubic_kahan(prev1, prev0, current, next);
			}
			else
			{
				static_assert(false, "Invalid integration method");
			}
		}

		void iterate_forces_and_moves() noexcept
		{
			auto p1_idx = (_current_iteration + NUM_GENERATIONS - 2) % NUM_GENERATIONS;
			auto p0_idx = (p1_idx + 1) % NUM_GENERATIONS;
			auto c_idx = (p0_idx + 1) % NUM_GENERATIONS;
			auto n_idx = (c_idx + 1) % NUM_GENERATIONS;

			auto& prev1_gen = _bodies_gens[p1_idx];
			auto& prev0_gen = _bodies_gens[p0_idx];
			auto& curr_gen = _bodies_gens[c_idx];
			auto& next_gen = _bodies_gens[n_idx];
			
			bool use_mt = (_mt_ticks_per_n_iter < _st_ticks_per_n_iter);

			auto sub_iter{ _current_iteration % PERFORMANCE_PROFILING_CYCLE };
			bool profiling_iter{ sub_iter < PERFORMANCE_PROFILING_N * 2 };

			if (profiling_iter)
			{
				if (sub_iter == 0)
				{
					_st_ticks_per_n_iter = 0;
					_mt_ticks_per_n_iter = 0;
				}
				use_mt = sub_iter < PERFORMANCE_PROFILING_N;
			}			

			uint64_t start = __rdtsc();
			if (!use_mt || _current_iteration == 0)
			{
				iterate_gravity_forces(prev1_gen, prev0_gen, curr_gen, next_gen);

				if (profiling_iter)
					_st_ticks_per_n_iter += __rdtsc() - start;
			}
			else
			{
				concurrency::parallel_for(0, static_cast<int>(curr_gen.size()),
					[&](int i)
					{
						iterate_gravity_forces_mt(prev1_gen, prev0_gen, curr_gen, next_gen, i);
					});

				if (profiling_iter)
					_mt_ticks_per_n_iter += __rdtsc() - start;
			}

		}

	public: 

		gravity_struct()
		{

		}

		void set_simulation_start_in_epoch_time_millis(uint64_t value)
		{
			_simulation_start_in_epoch_time_millis = value;
		}

		void register_body(const mass_body& body)
		{
			auto body_copy{ body };
			body_copy.mass_G = body_copy.mass * GRAVITATIONAL_CONSTANT;
			for (auto& gen : _bodies_gens)
			{
				gen.push_back(body_copy);
			}
		}

		//// 
		//// Calculates the resulting speed of the centre of mass of the struct, and shifts into the frame of reference where
		//// centre of mass of the struct remains stationary. 
		//// also shifts the centre of the view to the centre of mass
		////
		//void align_observers_frame_of_reference()
		//{
		//	acc3d mass_velocity{};
		//	acc3d mass_location{};	
		//	acc<double> total_mass{};

		//	for (auto& body : _bodies_gens[0])
		//	{				
		//		mass_location += body.location.value * body.mass;
		//		mass_velocity += body.velocity.value * body.mass;

		//		total_mass += body.mass;
		//	}

		//	auto& centre_of_mass = mass_location.value / total_mass.value;
		//	auto& centre_of_mass_velocity = mass_velocity.value / total_mass.value;

		//	for (auto& gen : _bodies_gens)
		//	{
		//		for (auto& body : gen)
		//		{
		//			body.location.value -= centre_of_mass;
		//			body.velocity.value -= centre_of_mass_velocity;
		//		}
		//	}
		//}

		const std::vector<mass_body>& get_bodies() const noexcept
		{
			return get_generation(0);
		}

		void set_time_delta(double time_delta)
		{
			_time_delta = time_delta;

			_time_delta_times_1_2 = _time_delta / 2.0;
			_time_delta_times_1_12 = _time_delta / 12.0;
			_time_delta_times_1_24 = _time_delta / 24.0;
		}

		bool load_from_csv(std::string input_file)
		{
			std::ifstream istrm(input_file);

			std::string csv_header;

			if (!std::getline(istrm, csv_header))
				return false;

			if (csv_header != mass_body::get_csv_header())
				return false;

			uint64_t prev_epoch_time{ 0 };
			uint64_t epoch_time{ 0 };
			bool show_warning{ false };

			std::string line;
			while (std::getline(istrm, line))
			{
				mass_body body;
				if (!body.from_csv_line(line, epoch_time))
					return false;

				if (prev_epoch_time != 0 && prev_epoch_time != epoch_time)
				{
					show_warning = true;
				}

				prev_epoch_time = epoch_time;
				register_body(body);
			}

			_simulation_start_in_epoch_time_millis = epoch_time;

			if (show_warning)
			{
				MessageBox(
					NULL,
					L"Warning: epoch times are inconsistent for objects in the input csv",
					L"Warning",
					MB_OK | MB_ICONHAND);
			}

			return true;
		}

		void set_output_csv(std::string output_file)
		{
			_report_file = output_file;
		}

		void set_report_centre(std::string report_centre)
		{
			_report_centre = report_centre;
		}

		void set_report_every(uint64_t report_every)
		{
			_report_every_n_iterations = report_every;
		}

		void set_max_iterations(uint64_t max_iterations)
		{
			_max_iterations = max_iterations;
		}

		bool iterate() noexcept
		{
			iterate_forces_and_moves();
			iterate_collision_merges();

			if (_current_iteration % (16 * 1024) == 0)
				check_for_escaped_bodies();

			_current_iteration++;

			if ((_report_every_n_iterations != 0 && (_current_iteration % _report_every_n_iterations) == 0) || 
				(_current_iteration >= _max_iterations))
			{
				generate_report();
			}

			return _current_iteration < _max_iterations;
		}

		int64_t current_iteration() const noexcept
		{
			return _current_iteration;
		}

		uint64_t current_time_epoch_millis() const noexcept
		{
			return _simulation_start_in_epoch_time_millis + static_cast<uint64_t>(std::round(_current_iteration * _time_delta * 1000.0));
		}

		void generate_report()
		{
			if (_report_file.empty())
			{
				return;
			}

			std::ofstream ostrm(_report_file, std::ios::app);

			if (_first_report)
			{
				_first_report = false;
				mass_body mb;
				ostrm << mb.get_csv_header() << "\n";
			}

			auto& current_gen = get_generation(0);

			vec3d_pd loc_centre{ 0.0, 0.0, 0.0 };
			vec3d_pd vel_centre{ 0.0, 0.0, 0.0 };

			for (const auto& body: current_gen)
			{
				if (body.label == _report_centre)
				{
					loc_centre = body.location.value;
					vel_centre = body.velocity.value;
					break;
				}
			}

			uint64_t current_epoch_time{ current_time_epoch_millis() };

			for (int idx = 0; idx < current_gen.size(); ++idx)
			{
				mass_body body_copy = current_gen[idx];
				
				body_copy.location.value -= loc_centre;
				body_copy.velocity.value -= vel_centre;

				auto line = body_copy.to_csv_line(_current_iteration, current_epoch_time, idx);
				ostrm << line << "\n";
			}

			ostrm.flush();
		}

		void save_to(std::ostream& stream)
		{
			stream.write(reinterpret_cast<const char*>(&_current_iteration), sizeof(_current_iteration));
			stream.write(reinterpret_cast<const char*>(&_simulation_start_in_epoch_time_millis), sizeof(_simulation_start_in_epoch_time_millis));
			stream.write(reinterpret_cast<const char*>(&_time_delta), sizeof(_time_delta));

			uint32_t len = static_cast<uint32_t>(_bodies_gens[0].size());
			stream.write(reinterpret_cast<const char*>(&len), sizeof(len));

			for (auto& gen : _bodies_gens)
			{
				for (auto& b : gen)
				{
					b.save_to(stream);
				}
			}
		}

		void load_from(std::istream& stream)
		{
			stream.read(reinterpret_cast<char*>(&_current_iteration), sizeof(_current_iteration));
			stream.read(reinterpret_cast<char*>(&_simulation_start_in_epoch_time_millis), sizeof(_simulation_start_in_epoch_time_millis));
			stream.read(reinterpret_cast<char*>(&_time_delta), sizeof(_time_delta));

			_time_delta_times_1_2 = _time_delta / 2.0;
			_time_delta_times_1_12 = _time_delta / 12.0;
			_time_delta_times_1_24 = _time_delta / 24.0;

			uint32_t len;
			stream.read(reinterpret_cast<char*>(&len), sizeof(len));

			for (auto& gen : _bodies_gens)
			{
				gen.resize(len);
				for (uint32_t i = 0; i < len; ++i)
				{
					gen[i] = {};
					gen[i].load_from(stream);
				}
			}
		}
	};
}