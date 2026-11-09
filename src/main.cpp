#include <windows.h>
#include "common/tw.h"
#include "common/game_api.h"
#include "rome2/security.h"

extern const luaL_Reg twdll_core[];
extern void register_faction_methods(lua_State* L);
extern void register_unit_methods(lua_State* L);
extern void register_character_methods(lua_State* L);

static bool g_is_initialized = false;

static const char* dll_reason_name(DWORD reason) {
    switch (reason) {
        case DLL_PROCESS_DETACH: return "DLL_PROCESS_DETACH";
        case DLL_PROCESS_ATTACH: return "DLL_PROCESS_ATTACH";
        case DLL_THREAD_ATTACH:  return "DLL_THREAD_ATTACH";
        case DLL_THREAD_DETACH:  return "DLL_THREAD_DETACH";
        default:                 return "UNKNOWN";
    }
}

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD reason, LPVOID) {
    Log("DllMain() called, reason=%u (%s)", reason, dll_reason_name(reason));
    if (reason == DLL_PROCESS_ATTACH) {
        Log("DllMain() PROCESS_ATTACH: disabling thread attach/detach calls");
        DisableThreadLibraryCalls(hModule);
    }
    return TRUE;
}

extern "C" __declspec(dllexport) int luaopen_twdll(lua_State *L) {
    if (!is_valid_game_host()) {
        Log("[twdll] Execution blocked: host process is not Rome2.exe or missing empire.retail.dll");
        return 0;
    }

    Log("[twdll] luaopen_twdll: called");

    if (!g_is_initialized) {
        Log("[twdll] First load: scanning Lua C API and game module");
        initialize_lua_api();
        initialize_game_api();
        g_is_initialized = true;
    }

    if (!g_game_lua_createtable || !g_game_luaL_register || !g_game_lua_setfield) {
        Log("[twdll] luaopen_twdll: required Lua C API signatures are missing, module not registered");
        return 0;
    }

    l_createtable(L, 0, 1);
    l_register(L, "twdll", twdll_core);
    l_setfield(L, -2, "core");
    register_faction_methods(L);
    register_unit_methods(L);
    register_character_methods(L);

    Log("[twdll] luaopen_twdll: registered twdll.core, faction, unit, and character methods");
    return 1;
}
