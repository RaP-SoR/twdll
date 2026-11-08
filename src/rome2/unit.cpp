/// @module UNIT_SCRIPT_INTERFACE
/// Extensions to the game's unit object (Rome 2).

#include "../common/tw.h"
#include "tw_types.h"

using twdll::TW_Unit;

constexpr size_t UNIT_PTR = twdll::TW_PtrOffset<TW_Unit>::value;

namespace Props {
static twdll::Property NumMen{&TW_Unit::num_men, UNIT_PTR, "unit"};
static twdll::Property MaxNumMen{&TW_Unit::max_num_men, UNIT_PTR, "unit"};
static twdll::Property ActionPoints{&TW_Unit::action_points, UNIT_PTR, "unit"};
}

static int push_set_result(lua_State* L, bool ok) {
    if (!g_game_lua_pushboolean) return 0;
    l_pushboolean(L, ok ? 1 : 0);
    return 1;
}

/// Memory address of the unit object in hexadecimal format.
/// @function GetMemoryAddress
/// @treturn string memory address (e.g. `"0x12345678"`)
/// @usage local addr = unit:GetMemoryAddress()
static int GetMemoryAddress(lua_State* L) {
    return tw_mem_address(L, "unit", UNIT_PTR);
}

/// Current number of surviving soldiers in the unit.
/// @function GetNumMen
/// @treturn integer current number of men
/// @usage local men = unit:GetNumMen()
static int GetNumMen(lua_State* L) {
    return Props::NumMen.get(L);
}

/// Sets the number of surviving soldiers in the unit.
/// @function SetNumMen
/// @tparam integer value new number of men
/// @treturn boolean `true` on success, `false` if the unit wrapper is invalid
/// @usage unit:SetNumMen(unit:GetMaxNumMen())
static int SetNumMen(lua_State* L) {
    if (!tw_get_obj(L, "unit", UNIT_PTR)) return push_set_result(L, false);
    Props::NumMen.set(L);
    return push_set_result(L, true);
}

/// Maximum soldier capacity (full strength count) for this unit.
/// @function GetMaxNumMen
/// @treturn integer maximum number of men
/// @usage local max_men = unit:GetMaxNumMen()
static int GetMaxNumMen(lua_State* L) {
    return Props::MaxNumMen.get(L);
}

/// Sets the maximum soldier capacity for this unit.
/// @function SetMaxNumMen
/// @tparam integer value new maximum number of men
/// @treturn boolean `true` on success, `false` if the unit wrapper is invalid
/// @usage unit:SetMaxNumMen(160)
static int SetMaxNumMen(lua_State* L) {
    if (!tw_get_obj(L, "unit", UNIT_PTR)) return push_set_result(L, false);
    Props::MaxNumMen.set(L);
    return push_set_result(L, true);
}

/// Remaining movement action points for this unit.
/// @function GetActionPoints
/// @treturn integer action points
/// @usage local ap = unit:GetActionPoints()
static int GetActionPoints(lua_State* L) {
    return Props::ActionPoints.get(L);
}

/// Sets the movement action points for this unit.
/// @function SetActionPoints
/// @tparam integer value new action points
/// @treturn boolean `true` on success, `false` if the unit wrapper is invalid
/// @usage unit:SetActionPoints(100)
static int SetActionPoints(lua_State* L) {
    if (!tw_get_obj(L, "unit", UNIT_PTR)) return push_set_result(L, false);
    Props::ActionPoints.set(L);
    return push_set_result(L, true);
}

static const luaL_Reg unit_methods[] = {
    {"GetMemoryAddress", GetMemoryAddress},
    {"GetNumMen",        GetNumMen},
    {"SetNumMen",        SetNumMen},
    {"GetMaxNumMen",     GetMaxNumMen},
    {"SetMaxNumMen",     SetMaxNumMen},
    {"GetActionPoints",  GetActionPoints},
    {"SetActionPoints",  SetActionPoints},
    {nullptr, nullptr}
};

void register_unit_methods(lua_State* L) {
    if (!g_game_luaL_newmetatable || !g_game_lua_getfield || !g_game_lua_type ||
        !g_game_lua_pushstring || !g_game_lua_pushcclosure || !g_game_lua_settable ||
        !g_game_lua_settop) {
        Log("[twdll] register_unit_methods: required Lua C API signatures are not resolved");
        return;
    }

    l_newmetatable(L, "UNIT_SCRIPT_INTERFACE");
    l_getfield(L, -1, "__index");
    if (l_type(L, -1) == LUA_TTABLE) {
        for (const luaL_Reg* f = unit_methods; f->name; ++f) {
            l_pushstring(L, f->name);
            l_pushcclosure(L, f->func, 0);
            l_settable(L, -3);
        }
        Log("[twdll] UNIT_SCRIPT_INTERFACE extended");
    } else {
        Log("[twdll] WARNING: UNIT_SCRIPT_INTERFACE __index not found");
    }
    l_pop(L, 2);
}
