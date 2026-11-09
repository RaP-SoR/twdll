/// @module twdll.campaign_ui
/// Campaign UI singleton accessor for Total War: Rome 2.

#include "../common/tw.h"
#include "../common/campaign_hooks.h"
#include "../common/signature_scanner.h"
#include "tw_types.h"

#include <MinHook.h>
#include <cstdio>
#include <windows.h>

using twdll::TW_CampaignUi;

static TW_CampaignUi* g_campaign_ui = nullptr;
static void* orig_campaign_ui_ctor = nullptr;
static uintptr_t campaign_ui_ctor_addr = 0;

static void LogCampaignUiHook(void* ptr) {
    g_campaign_ui = static_cast<TW_CampaignUi*>(ptr);
    Log("[twdll] CAMPAIGN_UI ctor hooked — g_campaign_ui = 0x%08X", reinterpret_cast<uintptr_t>(ptr));
}

__declspec(naked) static void HookedCampaignUiCtor() {
    __asm {
        pushad
        push ecx
        call LogCampaignUiHook
        add esp, 4
        popad
        jmp dword ptr [orig_campaign_ui_ctor]
    }
}

void install_campaign_ui_hook(uintptr_t base, size_t size) {
    const char* anchor = "data/ui/campaign ui/mp_timer";
    const char* label = "CAMPAIGN_UI";

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
                                   reinterpret_cast<void*>(HookedCampaignUiCtor),
                                   reinterpret_cast<void**>(&orig_campaign_ui_ctor));
    if (mhs != MH_OK) {
        Log("[twdll] [%s] MH_CreateHook failed (%d)", label, mhs);
        return;
    }
    campaign_ui_ctor_addr = ctor_addr;

    mhs = MH_EnableHook(reinterpret_cast<void*>(ctor_addr));
    if (mhs != MH_OK) {
        Log("[twdll] [%s] MH_EnableHook failed (%d)", label, mhs);
        return;
    }

    Log("[twdll] [%s] hook installed OK at 0x%08X", label, static_cast<unsigned int>(ctor_addr));
}

void uninstall_campaign_ui_hook() {
    if (campaign_ui_ctor_addr) {
        MH_DisableHook(reinterpret_cast<void*>(campaign_ui_ctor_addr));
        MH_RemoveHook(reinterpret_cast<void*>(campaign_ui_ctor_addr));
        campaign_ui_ctor_addr = 0;
    }
    g_campaign_ui = nullptr;
    orig_campaign_ui_ctor = nullptr;
}

/// Memory address of the CAMPAIGN_UI singleton as a hexadecimal string.
/// @function GetMemoryAddress
/// @treturn string|nil memory address (e.g. `"0x12345678"`), or nil if not yet captured
/// @usage local addr = twdll.campaign_ui.GetMemoryAddress()
static int GetMemoryAddress(lua_State* L) {
    if (!g_campaign_ui) {
        if (g_game_lua_pushnil) l_pushnil(L);
        return g_game_lua_pushnil ? 1 : 0;
    }
    char buf[20];
    snprintf(buf, sizeof(buf), "0x%08X", reinterpret_cast<unsigned int>(g_campaign_ui));
    l_pushstring(L, buf);
    return 1;
}

extern const luaL_Reg campaign_ui_functions[] = {
    {"GetMemoryAddress", GetMemoryAddress},
    {nullptr, nullptr}
};
