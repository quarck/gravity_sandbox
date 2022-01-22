#pragma once

#include <charconv>
#include <shellapi.h>
#include <limits>
#include <string>
#include <vector>

namespace gravity
{
    class runtime_config
    {
        double _time_delta{ 0.1 };

        uint64_t _report_every_n{ 1000 };
        uint64_t _max_n{ std::numeric_limits<uint64_t>::max() };

        int _num_worker_threads{ 1 }; // would be more more flexible in the future.. 
        
        std::string _input_file{};
        std::string _output_file{};

        bool _auto_start{ false };

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

        bool parse_command_line(LPWSTR lpszCmdLine)
        {
            int argc;
            LPWSTR* argv = CommandLineToArgvW(lpszCmdLine, &argc);

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
                else if (wcscmp(argv[idx], L"--report-every-n") == 0 && (idx + 1) < argc)
                {
                    _report_every_n = std::stoull(std::wstring{ argv[idx + 1] });
                    idx++;
                }
                else if (wcscmp(argv[idx], L"--max-n") == 0 && (idx + 1) < argc)
                {
                    _max_n =  std::stoull(std::wstring{ argv[idx + 1] });
                    idx++;
                }
                else if (wcscmp(argv[idx], L"--auto-start") == 0)
                {
                    _auto_start = true;
                }
                else
                {
                    return false;
                }
            }

            return true;
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