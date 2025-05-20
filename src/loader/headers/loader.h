#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <fstream>
#include <vector> 

#if defined(__linux__)
    #include <unistd.h>
    #include <limits.h>
    #include <libgen.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <limits.h> 
    #include <libgen.h>
#elif defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
    #ifndef PATH_MAX
        #define PATH_MAX MAX_PATH
    #endif
#endif

namespace loader {
std::string getExecutableDir();

std::string findTomlPath();

std::string findLibraryPath(const std::string& Default_LIB = "libm_std.so");

} 

#endif // LOADER_H