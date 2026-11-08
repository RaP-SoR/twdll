#include "../common/tw.h"

extern const char* GAME_NAME;

/// @module twdll.core
/// Utility functions for the Rome 2 dummy build.
/// Gameplay methods are registered only after their signatures are confirmed.

#ifndef TWDLL_BUILD_SHA
#define TWDLL_BUILD_SHA "unknown"
#endif

/// Log a single string to `twdll.log`.
/// One string only: the multi-argument `tostring` path needs `lua_getfield`,
/// `lua_type`, `lua_pushvalue`, and `lua_settop`, which are not in the Rome 2
/// signature list yet.
/// @function Log
/// @tparam string message text to write
/// @usage
/// twdll.core.Log("Campaign script loaded")
static int script_Log(lua_State* L) {
    if (!g_game_luaL_checklstring) {
        Log("[twdll] Log: luaL_checklstring is not resolved");
        return 0;
    }
    const char* msg = l_checklstring(L, 1, nullptr);
    Log("%s", msg ? msg : "");
    return 0;
}

/// Returns the target game build name.
/// @function GameBuild
/// @treturn string game name, `"Rome2"` on this branch
/// @usage
/// local game_name = twdll.core.GameBuild()
static int script_GameBuild(lua_State* L) {
    if (!g_game_lua_pushstring) {
        Log("[twdll] GameBuild: lua_pushstring is not resolved");
        return 0;
    }
    l_pushstring(L, GAME_NAME);
    return 1;
}

/// Returns the git commit SHA hash from which the DLL was compiled.
/// @function GetBuildSha
/// @treturn string git commit SHA, or `"unknown"` when git was unavailable at build time
/// @usage
/// local sha = twdll.core.GetBuildSha()
static int script_GetBuildSha(lua_State* L) {
    if (!g_game_lua_pushstring) {
        Log("[twdll] GetBuildSha: lua_pushstring is not resolved");
        return 0;
    }
    l_pushstring(L, TWDLL_BUILD_SHA);
    return 1;
}

extern const luaL_Reg twdll_core[] = {
    {"Log",         script_Log},
    {"GameBuild",   script_GameBuild},
    {"GetBuildSha", script_GetBuildSha},
    {nullptr, nullptr}
};
