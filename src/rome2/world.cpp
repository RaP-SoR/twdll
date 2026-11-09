/// @module twdll.world
/// Campaign WORLD singleton accessor for Total War: Rome 2.

#include "../common/tw.h"
#include "../common/campaign_hooks.h"
#include "../common/signature_scanner.h"
#include "tw_types.h"

#include <MinHook.h>
#include <cstdio>
#include <windows.h>

using twdll::TW_World;

static TW_World* g_world = nullptr;
static void* orig_world_ctor = nullptr;
static uintptr_t world_ctor_addr = 0;

static void LogWorldHook(void* ptr) {
    g_world = static_cast<TW_World*>(ptr);
    Log("[twdll] WORLD ctor hooked — g_world = 0x%08X", reinterpret_cast<uintptr_t>(ptr));
}

__declspec(naked) static void HookedWorldCtor() {
    __asm {
        pushad
        push ecx
        call LogWorldHook
        add esp, 4
        popad
        jmp dword ptr [orig_world_ctor]
    }
}

void install_world_hook(uintptr_t base, size_t size) {
    const char* anchor = "FACTION_ARRAY";
    const char* label = "WORLD";

    Log("[twdll] Processing anchor '%s' for %s ctor", anchor, label);

    uintptr_t str_addr = Scanner::FindString(base, size, anchor);
    if (!str_addr) {
        Log("[twdll] [%s] anchor string not found", label);
        return;
    }

    uintptr_t push_addr = Scanner::FindPushRef(base, size, str_addr);
    if (!push_addr) {
        Log("[twdll] [%s] push ref not found", label);
        return;
    }

    uintptr_t ctor_addr = Scanner::FindPrologue(push_addr);
    if (!ctor_addr) {
        Log("[twdll] [%s] prologue not found", label);
        return;
    }

    MH_STATUS mhs = MH_CreateHook(reinterpret_cast<void*>(ctor_addr),
                                   reinterpret_cast<void*>(HookedWorldCtor),
                                   reinterpret_cast<void**>(&orig_world_ctor));
    if (mhs != MH_OK) {
        Log("[twdll] [%s] MH_CreateHook failed (%d)", label, mhs);
        return;
    }
    world_ctor_addr = ctor_addr;

    mhs = MH_EnableHook(reinterpret_cast<void*>(ctor_addr));
    if (mhs != MH_OK) {
        Log("[twdll] [%s] MH_EnableHook failed (%d)", label, mhs);
        return;
    }

    Log("[twdll] [%s] hook installed OK at 0x%08X", label, static_cast<unsigned int>(ctor_addr));
}

void uninstall_world_hook() {
    if (world_ctor_addr) {
        MH_DisableHook(reinterpret_cast<void*>(world_ctor_addr));
        MH_RemoveHook(reinterpret_cast<void*>(world_ctor_addr));
        world_ctor_addr = 0;
    }
    g_world = nullptr;
    orig_world_ctor = nullptr;
}

/// Memory address of the WORLD singleton as a hexadecimal string.
/// @function GetMemoryAddress
/// @treturn string|nil memory address (e.g. `"0x12345678"`), or nil if not yet captured
/// @usage local addr = twdll.world.GetMemoryAddress()
static int GetMemoryAddress(lua_State* L) {
    if (!g_world) {
        if (g_game_lua_pushnil) l_pushnil(L);
        return g_game_lua_pushnil ? 1 : 0;
    }
    char buf[20];
    snprintf(buf, sizeof(buf), "0x%08X", reinterpret_cast<unsigned int>(g_world));
    l_pushstring(L, buf);
    return 1;
}

static twdll::GlobalGetter<int, TW_World> FactionCount{&TW_World::faction_count, &g_world};

/// Total number of factions in the current campaign world.
/// @function GetFactionCount
/// @treturn integer|nil faction count, or nil if WORLD is not yet captured
/// @usage local n = twdll.world.GetFactionCount()
static int GetFactionCount(lua_State* L) {
    return FactionCount.get(L);
}

/// Captures the WORLD singleton from a vanilla world script-interface userdata.
/// Use when the ctor hook missed construction (e.g. DLL loaded after campaign start).
/// @function Capture
/// @tparam userdata world vanilla `model():world()` object
/// @treturn boolean `true` if the pointer was captured
/// @usage twdll.world.Capture(cm:model():world())
static int Capture(lua_State* L) {
    if (!g_game_lua_touserdata || !g_game_lua_pushboolean) return 0;
    void** ud = static_cast<void**>(l_touserdata(L, 1));
    if (!ud || !*ud) {
        l_pushboolean(L, 0);
        return 1;
    }
    auto* obj = *reinterpret_cast<TW_World**>(static_cast<char*>(*ud) + 0x8);
    if (!obj) {
        l_pushboolean(L, 0);
        return 1;
    }
    g_world = obj;
    Log("[twdll] WORLD captured from userdata — g_world = 0x%08X", reinterpret_cast<uintptr_t>(obj));
    l_pushboolean(L, 1);
    return 1;
}

extern const luaL_Reg world_functions[] = {
    {"GetMemoryAddress", GetMemoryAddress},
    {"GetFactionCount",  GetFactionCount},
    {"Capture",          Capture},
    {nullptr, nullptr}
};
