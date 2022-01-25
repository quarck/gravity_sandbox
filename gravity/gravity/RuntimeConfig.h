#pragma once

#include <charconv>
#include <shellapi.h>
#include <limits>
#include <string>
#include <vector>

#include "WorldObjects.h"

namespace gravity
{
    class runtime_config
    {
        double _time_delta{ 1 };

        uint64_t _report_every_n{ 1000 };
        uint64_t _max_n{ std::numeric_limits<uint64_t>::max() };

        int _num_worker_threads{ 1 }; // would be more more flexible in the future.. 
        
        std::string _input_file{};
        std::string _output_file{};

        bool _auto_start{ false };

        integration_method method{ integration_method::cubic_kahan };

    public:

        runtime_config()
        {
#ifndef _DEBUG
            SYSTEM_INFO sysinfo;
            ::GetSystemInfo(&sysinfo);
            _num_worker_threads = sysinfo.dwNumberOfProcessors ;
#endif
        }

        static std::string wcs2mbs(std::wstring w_string)
        {
            const wchar_t* wcs_ind_string = w_string.c_str();
            int buffer_size = (w_string.size() + 10) * 8 + 1024;

            std::vector<char> buffer(buffer_size);

            mbstate_t       mbstate;
            ::memset((void*)&mbstate, 0, sizeof(mbstate));

            size_t  converted;
            errno_t err = wcsrtombs_s(&converted, buffer.data(), buffer_size, &wcs_ind_string, buffer_size, &mbstate);

            if (err == EILSEQ)
                return "";
            return std::string(buffer.data(), converted);
        }

        static LPWSTR get_usage()
        {
            return
                L"Usage:\r\n"
                L"gravity.exe [--input <input_file.csv>] [--output <output.csv>] [options]\r\n"
                L"options are:\r\n"
                L"  --time-delta <time_delta_seconds>\r\n" L"    default is 1.0, supports float values\r\n"
                L"  --report-every <simulated_seconds>\r\n" L"    report into <output.csv> every given simulated period\r\n"
                L"  --duration <simulated_seconds>\r\n" L"    automatically stop the simulation after simulating this much\r\n"
                L"  --auto-start\r\n" L"    start unpaused\r\n"
                L"  --method <integration_method>\r\n" L"    Use specific integration method\r\n"
                L"    Supported integration methods\r\n"
                L"    0 - naive\r\n" L"         v += f(t) * dt\r\n"
                L"    1 - naive_kahan\r\n" L"         same using Kahan sum\r\n"
                L"    2 - linear\r\n" L"         v += (f(t) + f(t-1)) / 2.0 * dt\r\n"
                L"    3 - linear_kahan\r\n" L"         same using Kahan sum\r\n"
                L"    4 - quadratic\r\n" L"         v += (5*f(t) + 8*f(t-1) - f(t-2)) / 12.0 * dt\r\n"
                L"    5 - quadratic_kahan\r\n" L"         same using Kahan sum\r\n"
                L"    6 - cubic\r\n" L"         v += (-f(t) + 13*f(t-1) + 13*f(t-2) - f(t-3)) / 24.0 * dt\r\n"
                L"    7 - cubic_kahan [DEFAULT]\r\n" L"         same using Kahan sum\r\n"
                ;
        }

        bool parse_command_line(LPWSTR lpszCmdLine)
        {
            if (wcscmp(lpszCmdLine, L"") == 0)
            {
                return true;
            }

            int argc;
            LPWSTR* argv = CommandLineToArgvW(lpszCmdLine, &argc);

            uint64_t report_every_n_seconds = 1000;
            uint64_t duration = 0; // infinite 

            for (int idx = 0; idx < argc; idx++)
            {
                if (wcscmp(argv[idx], L"--input") == 0 && (idx + 1) < argc)
                {
                    _input_file = wcs2mbs(argv[idx + 1]);
                    idx++;
                }
                else if (wcscmp(argv[idx], L"--output") == 0 && (idx + 1) < argc)
                {
                    _output_file = wcs2mbs(argv[idx + 1]);
                    idx++;
                }
                else if (wcscmp(argv[idx], L"--time-delta") == 0 && (idx + 1) < argc)
                {
                    _time_delta = std::stod(std::wstring{ argv[idx + 1] });
                    idx++;
                }
                else if (wcscmp(argv[idx], L"--report-every") == 0 && (idx + 1) < argc)
                {
                    report_every_n_seconds = std::stoull(std::wstring{ argv[idx + 1] });
                    idx++;
                }
                else if (wcscmp(argv[idx], L"--duration") == 0 && (idx + 1) < argc)
                {
                    duration =  std::stoull(std::wstring{ argv[idx + 1] });
                    idx++;
                }
                else if (wcscmp(argv[idx], L"--auto-start") == 0)
                {
                    _auto_start = true;
                }
                else if (wcscmp(argv[idx], L"--method") == 0 && (idx + 1) < argc)
                {
                    int m = std::stoi(std::wstring{ argv[idx + 1] });
                    idx++;

                    if (m < static_cast<int>(integration_method::naive) ||
                        m > static_cast<int>(integration_method::cubic_kahan))
                    {
                        return false;
                    }

                    method = static_cast<integration_method>(m);
                }
                else
                {
                    return false;
                }
            }

            _report_every_n = static_cast<uint64_t>(std::round(static_cast<double>(report_every_n_seconds) / _time_delta));

            if (duration != 0)
            {
                _max_n = static_cast<uint64_t>(std::round(static_cast<double>(duration) / _time_delta));
            }

            return true;
        }

        inline integration_method get_integration_method() const noexcept
        {
            return method;
        }

        inline int num_worker_thrads() const noexcept 
        {
            return _num_worker_threads;
        }

        inline double time_delta() const noexcept
        {
            return _time_delta;
        }       

        inline uint64_t report_every_n() const noexcept
        {
            return _report_every_n;
        }

        inline uint64_t max_n() const noexcept
        {
            return _max_n;
        }

        inline const std::string& input_file() const noexcept
        {
            return _input_file;
        }

        inline const std::string& output_file() const noexcept
        {
            return _output_file;
        }

        inline bool auto_star() const noexcept
        {
            return _auto_start;
        }
    };

}