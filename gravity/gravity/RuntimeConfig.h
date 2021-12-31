#pragma once

namespace gravity
{

    class RuntimeConfig
    {
        int numWorkerThreads; // would be more more flexible in the future.. 
    public:

        RuntimeConfig()
        {
#ifdef _DEBUG
			numWorkerThreads = 1;
#else 
            SYSTEM_INFO sysinfo;
            ::GetSystemInfo(&sysinfo);
            numWorkerThreads = sysinfo.dwNumberOfProcessors ;
#endif
        }

        int GetNumWorkerThreads() const noexcept 
        {
            return numWorkerThreads; 
        }
    };

}