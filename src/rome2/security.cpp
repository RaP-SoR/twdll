#include "rome2/security.h"

#include <windows.h>
#include <algorithm>
#include <cctype>
#include <string>

bool is_valid_game_host() {
    char exe_path[MAX_PATH] = {};
    if (GetModuleFileNameA(NULL, exe_path, MAX_PATH) == 0) {
        return false;
    }

    std::string path_lower(exe_path);
    std::transform(path_lower.begin(), path_lower.end(), path_lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (path_lower.find("rome2.exe") == std::string::npos) {
        return false;
    }

    if (GetModuleHandleA("empire.retail.dll") == NULL) {
        return false;
    }

    return true;
}
