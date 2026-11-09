/// @module SLOT_SCRIPT_INTERFACE
/// Extensions to the game's settlement slot object (Rome 2).

#include "../common/tw.h"
#include "tw_types.h"

using twdll::TW_RegionSlot;

constexpr size_t SLOT_PTR = twdll::TW_PtrOffset<TW_RegionSlot>::value;

/// Memory address of the slot object in hexadecimal format.
/// @function GetMemoryAddress
/// @treturn string memory address (e.g. `"0x12345678"`)
/// @usage local addr = slot:GetMemoryAddress()
static int GetMemoryAddress(lua_State* L) {
    return tw_mem_address(L, "slot", SLOT_PTR);
}

static const luaL_Reg slot_methods[] = {
    {"GetMemoryAddress", GetMemoryAddress},
    {nullptr, nullptr}
};

void register_slot_methods(lua_State* L) {
    if (!g_game_luaL_newmetatable || !g_game_lua_getfield || !g_game_lua_type ||
        !g_game_lua_pushstring || !g_game_lua_pushcclosure || !g_game_lua_settable ||
        !g_game_lua_settop) {
        Log("[twdll] register_slot_methods: required Lua C API signatures are not resolved");
        return;
    }

    l_newmetatable(L, "SLOT_SCRIPT_INTERFACE");
    l_getfield(L, -1, "__index");
    if (l_type(L, -1) == LUA_TTABLE) {
        for (const luaL_Reg* f = slot_methods; f->name; ++f) {
            l_pushstring(L, f->name);
            l_pushcclosure(L, f->func, 0);
            l_settable(L, -3);
        }
        Log("[twdll] SLOT_SCRIPT_INTERFACE extended");
    } else {
        Log("[twdll] WARNING: SLOT_SCRIPT_INTERFACE __index not found");
    }
    l_pop(L, 2);
}
