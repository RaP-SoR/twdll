/// @module SLOT_SCRIPT_INTERFACE
/// Extensions to the game's settlement slot object (Rome 2).

#include "../common/tw.h"
#include "tw_types.h"

#include <cstdio>

using twdll::TW_RegionSlot;

constexpr size_t SLOT_PTR = twdll::TW_PtrOffset<TW_RegionSlot>::value;

/// Memory address of the slot object in hexadecimal format.
/// @function GetMemoryAddress
/// @treturn string memory address (e.g. `"0x12345678"`)
/// @usage local addr = slot:GetMemoryAddress()
static int GetMemoryAddress(lua_State* L) {
    return tw_mem_address(L, "slot", SLOT_PTR);
}

/// Diagnostic: read a 32-bit signed integer at a byte offset from the slot object.
/// Temporary helper for layout discovery — not a stable gameplay API.
/// @function GetIntAtOffset
/// @tparam integer offset byte offset from the start of the slot object
/// @treturn integer|nil value at the offset, or nil if the wrapper is invalid
/// @usage local v = slot:GetIntAtOffset(0x8)
static int GetIntAtOffset(lua_State* L) {
    return tw_get_int_at(L, "slot", SLOT_PTR);
}

/// Diagnostic: write a 32-bit signed integer at a byte offset on the slot object.
/// Temporary helper for layout discovery — not a stable gameplay API.
/// @function SetIntAtOffset
/// @tparam integer offset byte offset from the start of the slot object
/// @tparam integer value value to write
/// @usage slot:SetIntAtOffset(0xA0, 3)
static int SetIntAtOffset(lua_State* L) {
    return tw_set_int_at(L, "slot", SLOT_PTR);
}

/// Diagnostic: log a run of 32-bit integers starting at `start_offset`.
/// Writes one `[DIAG][slot]` line to `twdll.log`. Clamps `count` to 1..64.
/// @function DumpInts
/// @tparam integer start_offset byte offset (should be 4-byte aligned)
/// @tparam integer count number of ints to dump
/// @treturn boolean `true` if the dump was written
/// @usage slot:DumpInts(0x0, 32)
static int DumpInts(lua_State* L) {
    void* obj = tw_get_obj(L, "slot", SLOT_PTR);
    if (!g_game_lua_pushboolean) return 0;
    if (!obj) {
        l_pushboolean(L, 0);
        return 1;
    }

    const int start = static_cast<int>(l_tointeger(L, 2));
    int count = static_cast<int>(l_tointeger(L, 3));
    if (start < 0) {
        l_pushboolean(L, 0);
        return 1;
    }
    if (count < 1) count = 1;
    if (count > 64) count = 64;

    char line[1024];
    int used = snprintf(line, sizeof(line), "[DIAG][slot] start=0x%X count=%d:", start, count);
    for (int i = 0; i < count && used > 0 && used < static_cast<int>(sizeof(line)) - 24; ++i) {
        const int value = tw_read<int>(obj, static_cast<size_t>(start + i * 4));
        used += snprintf(line + used, sizeof(line) - static_cast<size_t>(used),
                         " [0x%X]=%d", start + i * 4, value);
    }
    Log("%s", line);
    l_pushboolean(L, 1);
    return 1;
}

static const luaL_Reg slot_methods[] = {
    {"GetMemoryAddress", GetMemoryAddress},
    {"GetIntAtOffset",   GetIntAtOffset},
    {"SetIntAtOffset",   SetIntAtOffset},
    {"DumpInts",         DumpInts},
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
