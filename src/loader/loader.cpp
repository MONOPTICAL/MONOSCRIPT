#include "headers/loader.h" 
#include <iostream>

namespace loader {

std::string getExecutableDir() {
    std::vector<char> path_buf;
    path_buf.resize(PATH_MAX + 1, '\0');

#if defined(_WIN32) || defined(_WIN64)
    if (GetModuleFileNameA(NULL, path_buf.data(), static_cast<DWORD>(path_buf.size() -1)) == 0) {
        // Ошибка
        return "";
    }
    std::string exe_path_str(path_buf.data());
    size_t last_slash = exe_path_str.find_last_of("\\/");
    if (std::string::npos != last_slash) {
        return exe_path_str.substr(0, last_slash);
    }
#elif defined(__linux__)
    ssize_t len = ::readlink("/proc/self/exe", path_buf.data(), path_buf.size() - 1);
    if (len != -1) {
        path_buf[len] = '\0'; 
        std::string exe_path_str(path_buf.data());
        size_t last_slash_idx = exe_path_str.rfind('/');
        if (std::string::npos != last_slash_idx) {
            return exe_path_str.substr(0, last_slash_idx);
        }
    }
#elif defined(__APPLE__)
    uint32_t size = static_cast<uint32_t>(path_buf.size());
    if (_NSGetExecutablePath(path_buf.data(), &size) == 0) {
        std::vector<char> real_path_buf(PATH_MAX + 1, '\0');
        if (realpath(path_buf.data(), real_path_buf.data()) != NULL) {
            std::string exe_path_str(real_path_buf.data());
            size_t last_slash_idx = exe_path_str.rfind('/');
            if (std::string::npos != last_slash_idx) {
                return exe_path_str.substr(0, last_slash_idx);
            }
        }
    } else {
        path_buf.resize(size + 1, '\0');
        if (_NSGetExecutablePath(path_buf.data(), &size) == 0) {
            std::vector<char> real_path_buf(PATH_MAX + 1, '\0');
             if (realpath(path_buf.data(), real_path_buf.data()) != NULL) {
                std::string exe_path_str(real_path_buf.data());
                size_t last_slash_idx = exe_path_str.rfind('/');
                if (std::string::npos != last_slash_idx) {
                    return exe_path_str.substr(0, last_slash_idx);
                }
            }
        }
    }
#else
    return "."; 
#endif
    return "";
}


std::string findTomlPath() {
    std::string resolved_toml_path;
    bool found = false;

#ifdef STDLIB_TOML_PATH
    std::string path_from_def = STDLIB_TOML_PATH;
    std::ifstream file_def(path_from_def);
    if (file_def.good()) {
        resolved_toml_path = path_from_def;
        found = true;
    }
#endif

    if (!found) {
        std::string exe_dir = getExecutableDir();
        if (!exe_dir.empty()) {
            std::string path_near_exe = exe_dir + "/mono.toml";
            std::ifstream file_near_exe(path_near_exe);
            if (file_near_exe.good()) {
                resolved_toml_path = path_near_exe;
                found = true;
            }
        }
    }

    if (!found) {
        std::cerr << "Warning: mono.toml not found." << std::endl; // Или более серьезная ошибка
    }
    return resolved_toml_path; 
}


std::string findLibraryPath(const std::string& Default_LIB) {

#ifdef STDLIB_SO_PATH
    std::string path_from_def = STDLIB_SO_PATH;

    std::ifstream lib_file_def(path_from_def);
    if (lib_file_def.good()) {
        return path_from_def;
    }
#endif
    return Default_LIB;
}

}