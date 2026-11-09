/// @module CHARACTER_SCRIPT_INTERFACE
/// Extensions to the game's character object (Rome 2).

#include "../common/tw.h"
#include "tw_types.h"

using twdll::TW_Character;

constexpr size_t CHAR_PTR = twdll::TW_PtrOffset<TW_Character>::value;

namespace Props {
static twdll::Property ActionPoints{&TW_Character::action_points, CHAR_PTR, "character"};
static twdll::Property Ambition{&TW_Character::ambition, CHAR_PTR, "character"};
static twdll::Property Gravitas{&TW_Character::gravitas, CHAR_PTR, "character"};
}

static int push_set_result(lua_State* L, bool ok) {
    if (!g_game_lua_pushboolean) return 0;
    l_pushboolean(L, ok ? 1 : 0);
    return 1;
}

/// Memory address of the character object in hexadecimal format.
/// @function GetMemoryAddress
/// @treturn string memory address (e.g. `"0x12345678"`)
/// @usage local addr = character:GetMemoryAddress()
static int GetMemoryAddress(lua_State* L) {
    return tw_mem_address(L, "character", CHAR_PTR);
}

/// Remaining campaign movement action points for this character.
/// @function GetActionPoints
/// @treturn integer action points
/// @usage local ap = character:GetActionPoints()
static int GetActionPoints(lua_State* L) {
    return Props::ActionPoints.get(L);
}

/// Sets the campaign movement action points for this character.
/// @function SetActionPoints
/// @tparam integer value new action points
/// @treturn boolean `true` on success, `false` if the character wrapper is invalid
/// @usage character:SetActionPoints(100)
static int SetActionPoints(lua_State* L) {
    if (!tw_get_obj(L, "character", CHAR_PTR)) return push_set_result(L, false);
    Props::ActionPoints.set(L);
    return push_set_result(L, true);
}

/// Ambition level of the character.
/// @function GetAmbition
/// @treturn integer ambition level
/// @usage local ambition = character:GetAmbition()
static int GetAmbition(lua_State* L) {
    return Props::Ambition.get(L);
}

/// Sets the ambition level of the character.
/// @function SetAmbition
/// @tparam integer value new ambition level
/// @treturn boolean `true` on success, `false` if the character wrapper is invalid
/// @usage character:SetAmbition(50)
static int SetAmbition(lua_State* L) {
    if (!tw_get_obj(L, "character", CHAR_PTR)) return push_set_result(L, false);
    Props::Ambition.set(L);
    return push_set_result(L, true);
}

/// Gravitas (political standing) of the character.
/// @function GetGravitas
/// @treturn integer gravitas
/// @usage local gravitas = character:GetGravitas()
static int GetGravitas(lua_State* L) {
    return Props::Gravitas.get(L);
}

/// Sets the gravitas (political standing) of the character.
/// @function SetGravitas
/// @tparam integer value new gravitas
/// @treturn boolean `true` on success, `false` if the character wrapper is invalid
/// @usage character:SetGravitas(40)
static int SetGravitas(lua_State* L) {
    if (!tw_get_obj(L, "character", CHAR_PTR)) return push_set_result(L, false);
    Props::Gravitas.set(L);
    return push_set_result(L, true);
}

static const luaL_Reg character_methods[] = {
    {"GetMemoryAddress", GetMemoryAddress},
    {"GetActionPoints",  GetActionPoints},
    {"SetActionPoints",  SetActionPoints},
    {"GetAmbition",      GetAmbition},
    {"SetAmbition",      SetAmbition},
    {"GetGravitas",      GetGravitas},
    {"SetGravitas",      SetGravitas},
    {nullptr, nullptr}
};

void register_character_methods(lua_State* L) {
    if (!g_game_luaL_newmetatable || !g_game_lua_getfield || !g_game_lua_type ||
        !g_game_lua_pushstring || !g_game_lua_pushcclosure || !g_game_lua_settable ||
        !g_game_lua_settop) {
        Log("[twdll] register_character_methods: required Lua C API signatures are not resolved");
        return;
    }

    l_newmetatable(L, "CHARACTER_SCRIPT_INTERFACE");
    l_getfield(L, -1, "__index");
    if (l_type(L, -1) == LUA_TTABLE) {
        for (const luaL_Reg* f = character_methods; f->name; ++f) {
            l_pushstring(L, f->name);
            l_pushcclosure(L, f->func, 0);
            l_settable(L, -3);
        }
        Log("[twdll] CHARACTER_SCRIPT_INTERFACE extended");
    } else {
        Log("[twdll] WARNING: CHARACTER_SCRIPT_INTERFACE __index not found");
    }
    l_pop(L, 2);
}
