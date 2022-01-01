#pragma once

#include <cstdio>
#include <chrono>

class logger
{
	static inline FILE* _logfile{ nullptr };

	static inline std::chrono::steady_clock::time_point _start_time;

	static inline std::atomic_uint32_t _spin_lock{ 0 };

public: 
	static void open(const char* path)
	{
		if (_logfile != nullptr)
		{
			fclose(_logfile);
		}

		_start_time = std::chrono::high_resolution_clock::now();
		_logfile = fopen(path, "w");

		
		//std::chrono::duration<double> sinceLastUpdate = std::chrono::duration_cast<std::chrono::duration<double>>(now - lastUIUpdate);
	}

	template <typename ...Args>
	static void log(int thread_id, Args&... args)
	{
		if (_logfile == nullptr)
			return;

		auto now = std::chrono::high_resolution_clock::now();
		auto since_start = std::chrono::duration_cast<std::chrono::nanoseconds>(now - _start_time);

		
		for (;;)
		{
			uint32_t exp = 0;
			if (_spin_lock.compare_exchange_weak(exp, 1))
				break;
		}

		fprintf(_logfile, "%03d: T+%llu: ", thread_id, static_cast<uint64_t>(since_start.count()));
		fprintf(_logfile, args...);
		fprintf(_logfile, "\n");
		fflush(_logfile);

		_spin_lock = 0;
	}

};