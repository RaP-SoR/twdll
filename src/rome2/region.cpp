/// @module REGION_SCRIPT_INTERFACE
/// Extensions to the game's region object (Rome 2).

#include "../common/tw.h"
#include "tw_types.h"

using twdll::TW_Region;

constexpr size_t REGION_PTR = twdll::TW_PtrOffset<TW_Region>::value;

/// Memory address of the region object in hexadecimal format.
/// @function GetMemoryAddress
/// @treturn string memory address (e.g. `"0x12345678"`)
/// @usage local addr = region:GetMemoryAddress()
static int GetMemoryAddress(lua_State* L) {
    return tw_mem_address(L, "region", REGION_PTR);
}

static const luaL_Reg region_methods[] = {
    {"GetMemoryAddress", GetMemoryAddress},
    {nullptr, nullptr}
};

void register_region_methods(lua_State* L) {
    if (!g_game_luaL_newmetatable || !g_game_lua_getfield || !g_game_lua_type ||
        !g_game_lua_pushstring || !g_game_lua_pushcclosure || !g_game_lua_settable ||
        !g_game_lua_settop) {
        Log("[twdll] register_region_methods: required Lua C API signatures are not resolved");
        return;
    }

    l_newmetatable(L, "REGION_SCRIPT_INTERFACE");
    l_getfield(L, -1, "__index");
    if (l_type(L, -1) == LUA_TTABLE) {
        for (const luaL_Reg* f = region_methods; f->name; ++f) {
            l_pushstring(L, f->name);
            l_pushcclosure(L, f->func, 0);
            l_settable(L, -3);
        }
        Log("[twdll] REGION_SCRIPT_INTERFACE extended");
    } else {
        Log("[twdll] WARNING: REGION_SCRIPT_INTERFACE __index not found");
    }
    l_pop(L, 2);
}
