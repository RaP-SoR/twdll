/// @module MILITARY_FORCE_SCRIPT_INTERFACE
/// Extensions to the game's military force object (Rome 2).

#include "../common/tw.h"
#include "tw_types.h"

using twdll::TW_MilitaryForce;

constexpr size_t MIL_FORCE_PTR = twdll::TW_PtrOffset<TW_MilitaryForce>::value;

namespace Props {
static twdll::Property RecruitmentQueueSize{
    &TW_MilitaryForce::recruitment_queue_size, MIL_FORCE_PTR, "military_force"};
}

static int push_set_result(lua_State* L, bool ok) {
    if (!g_game_lua_pushboolean) return 0;
    l_pushboolean(L, ok ? 1 : 0);
    return 1;
}

/// Memory address of the military force object in hexadecimal format.
/// @function GetMemoryAddress
/// @treturn string memory address (e.g. `"0x12345678"`)
/// @usage local addr = force:GetMemoryAddress()
static int GetMemoryAddress(lua_State* L) {
    return tw_mem_address(L, "military_force", MIL_FORCE_PTR);
}

/// Number of units currently waiting in the recruitment queue.
/// @function GetRecruitmentQueueSize
/// @treturn integer queue size
/// @usage local n = force:GetRecruitmentQueueSize()
static int GetRecruitmentQueueSize(lua_State* L) {
    return Props::RecruitmentQueueSize.get(L);
}

/// Sets the recruitment queue size field on the military force.
/// @function SetRecruitmentQueueSize
/// @tparam integer value new queue size
/// @treturn boolean `true` on success, `false` if the force wrapper is invalid
/// @usage force:SetRecruitmentQueueSize(0)
static int SetRecruitmentQueueSize(lua_State* L) {
    if (!tw_get_obj(L, "military_force", MIL_FORCE_PTR)) return push_set_result(L, false);
    Props::RecruitmentQueueSize.set(L);
    return push_set_result(L, true);
}

static const luaL_Reg military_force_methods[] = {
    {"GetMemoryAddress",        GetMemoryAddress},
    {"GetRecruitmentQueueSize", GetRecruitmentQueueSize},
    {"SetRecruitmentQueueSize", SetRecruitmentQueueSize},
    {nullptr, nullptr}
};

void register_military_force_methods(lua_State* L) {
    if (!g_game_luaL_newmetatable || !g_game_lua_getfield || !g_game_lua_type ||
        !g_game_lua_pushstring || !g_game_lua_pushcclosure || !g_game_lua_settable ||
        !g_game_lua_settop) {
        Log("[twdll] register_military_force_methods: required Lua C API signatures are not resolved");
        return;
    }

    l_newmetatable(L, "MILITARY_FORCE_SCRIPT_INTERFACE");
    l_getfield(L, -1, "__index");
    if (l_type(L, -1) == LUA_TTABLE) {
        for (const luaL_Reg* f = military_force_methods; f->name; ++f) {
            l_pushstring(L, f->name);
            l_pushcclosure(L, f->func, 0);
            l_settable(L, -3);
        }
        Log("[twdll] MILITARY_FORCE_SCRIPT_INTERFACE extended");
    } else {
        Log("[twdll] WARNING: MILITARY_FORCE_SCRIPT_INTERFACE __index not found");
    }
    l_pop(L, 2);
}
