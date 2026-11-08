/// @module FACTION_SCRIPT_INTERFACE
/// Extensions to the game's faction object (Rome 2).

#include "../common/tw.h"
#include "tw_types.h"

using twdll::TW_Faction;

constexpr size_t FACTION_PTR = twdll::TW_PtrOffset<TW_Faction>::value;

namespace Props {
static twdll::Property Gold{&TW_Faction::gold, FACTION_PTR, "faction"};
}

/// Memory address of the faction object in hexadecimal format.
/// @function GetMemoryAddress
/// @treturn string memory address (e.g. `"0x12345678"`)
/// @usage local addr = faction:GetMemoryAddress()
static int GetMemoryAddress(lua_State* L) {
    return tw_mem_address(L, "faction", FACTION_PTR);
}

/// Amount of gold in the faction's treasury.
/// @function GetGold
/// @treturn integer current gold amount
/// @usage local gold = faction:GetGold()
static int GetGold(lua_State* L) {
    return Props::Gold.get(L);
}

/// Sets the amount of gold in the faction's treasury.
/// @function SetGold
/// @tparam integer value new gold amount
/// @treturn boolean `true` on success, `false` if the faction wrapper is invalid
/// @usage faction:SetGold(50000)
static int SetGold(lua_State* L) {
    void* obj = tw_get_obj(L, "faction", FACTION_PTR);
    if (!obj) {
        if (g_game_lua_pushboolean) l_pushboolean(L, 0);
        return g_game_lua_pushboolean ? 1 : 0;
    }
    Props::Gold.set(L);
    if (g_game_lua_pushboolean) {
        l_pushboolean(L, 1);
        return 1;
    }
    return 0;
}

static const luaL_Reg faction_methods[] = {
    {"GetMemoryAddress", GetMemoryAddress},
    {"GetGold",          GetGold},
    {"SetGold",          SetGold},
    {nullptr, nullptr}
};

void register_faction_methods(lua_State* L) {
    if (!g_game_luaL_newmetatable || !g_game_lua_getfield || !g_game_lua_type ||
        !g_game_lua_pushstring || !g_game_lua_pushcclosure || !g_game_lua_settable ||
        !g_game_lua_settop) {
        Log("[twdll] register_faction_methods: required Lua C API signatures are not resolved");
        return;
    }

    l_newmetatable(L, "FACTION_SCRIPT_INTERFACE");
    l_getfield(L, -1, "__index");
    if (l_type(L, -1) == LUA_TTABLE) {
        for (const luaL_Reg* f = faction_methods; f->name; ++f) {
            l_pushstring(L, f->name);
            l_pushcclosure(L, f->func, 0);
            l_settable(L, -3);
        }
        Log("[twdll] FACTION_SCRIPT_INTERFACE extended");
    } else {
        Log("[twdll] WARNING: FACTION_SCRIPT_INTERFACE __index not found");
    }
    l_pop(L, 2);
}
