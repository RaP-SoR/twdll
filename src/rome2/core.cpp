#include "../common/tw.h"

#include <string>

extern const char* GAME_NAME;

/// @module twdll.core
/// Utility functions for the Rome 2 build.
/// Gameplay methods are registered only after their signatures are confirmed.

#ifndef TWDLL_BUILD_SHA
#define TWDLL_BUILD_SHA "unknown"
#endif

/// Log a message or arbitrary values to `twdll.log`.
/// Accepts any number of arguments of any type, converting them via Lua tostring.
/// @function Log
/// @tparam any ... values to log
/// @usage
/// twdll.core.Log("Campaign initialized. Turn:", 1, "Faction:", faction:name())
static int script_Log(lua_State* L) {
    if (!g_game_lua_type || !g_game_lua_getfield || !g_game_lua_pushvalue ||
        !g_game_lua_pcall || !g_game_luaL_checklstring || !g_game_lua_settop) {
        Log("[twdll] Log: required Lua C API signatures are not resolved");
        return 0;
    }

    std::string full_msg;
    for (int i = 1; l_type(L, i) != LUA_TNONE; ++i) {
        if (i > 1) full_msg += "\t";
        l_getfield(L, LUA_GLOBALSINDEX, "tostring");
        l_pushvalue(L, i);
        if (l_pcall(L, 1, 1, 0) == 0) {
            if (const char* s = l_checklstring(L, -1, nullptr))
                full_msg += s;
            l_pop(L, 1);
        } else {
            l_pop(L, 1);
        }
    }
    Log("%s", full_msg.c_str());
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

// Script-interface metatable names used by the CA Lua binding layer.
// Probe is read-only: looks up the registry entry without creating a new metatable.
static const char* k_script_interfaces[] = {
    "FACTION_SCRIPT_INTERFACE",
    "CHARACTER_SCRIPT_INTERFACE",
    "UNIT_SCRIPT_INTERFACE",
    "MILITARY_FORCE_SCRIPT_INTERFACE",
    "REGION_SCRIPT_INTERFACE",
    "SLOT_SCRIPT_INTERFACE",
    "ARTSET_SCRIPT_INTERFACE",
    "CAMPAIGN_POLITICAL_PARTY_SCRIPT_INTERFACE",
    "POLITICAL_PARTY_LIST_SCRIPT_INTERFACE",
    "RELIGION_SCRIPT_INTERFACE",
    "RELIGION_LIST_SCRIPT_INTERFACE",
    nullptr
};

static bool probe_script_interface(lua_State* L, const char* name) {
    l_getfield(L, LUA_REGISTRYINDEX, name);
    if (l_type(L, -1) != LUA_TTABLE) {
        Log("[twdll] ProbeScriptInterfaces: %s missing (no registry metatable)", name);
        l_pop(L, 1);
        return false;
    }

    l_getfield(L, -1, "__index");
    const bool ok = (l_type(L, -1) == LUA_TTABLE);
    if (ok) {
        Log("[twdll] ProbeScriptInterfaces: %s OK (__index table)", name);
    } else {
        Log("[twdll] ProbeScriptInterfaces: %s present but __index is not a table (type=%d)",
            name, l_type(L, -1));
    }
    l_pop(L, 2);
    return ok;
}

/// Probe whether known CA script-interface metatables exist in the Lua registry.
/// Read-only: does not create metatables or attach methods. Call after the campaign
/// script environment is up (metatable registration is done by the engine).
/// @function ProbeScriptInterfaces
/// @treturn table map of interface name -> boolean (`true` if metatable and `__index` table exist)
/// @usage
/// local hits = twdll.core.ProbeScriptInterfaces()
/// if hits.FACTION_SCRIPT_INTERFACE then twdll.core.Log("faction metatable ready") end
static int script_ProbeScriptInterfaces(lua_State* L) {
    if (!g_game_lua_createtable || !g_game_lua_getfield || !g_game_lua_type ||
        !g_game_lua_settop || !g_game_lua_pushstring || !g_game_lua_pushboolean ||
        !g_game_lua_setfield) {
        Log("[twdll] ProbeScriptInterfaces: required Lua C API signatures are not resolved");
        return 0;
    }

    l_createtable(L, 0, 11);
    for (const char* const* name = k_script_interfaces; *name; ++name) {
        const bool ok = probe_script_interface(L, *name);
        l_pushboolean(L, ok ? 1 : 0);
        l_setfield(L, -2, *name);
    }
    return 1;
}

extern const luaL_Reg twdll_core[] = {
    {"Log",                   script_Log},
    {"GameBuild",             script_GameBuild},
    {"GetBuildSha",           script_GetBuildSha},
    {"ProbeScriptInterfaces", script_ProbeScriptInterfaces},
    {nullptr, nullptr}
};
