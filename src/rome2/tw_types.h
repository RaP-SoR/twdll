#pragma once
// tw_types.h — Engine object layouts for Total War: Rome 2 (32-bit).
// Do NOT share with attila/ — offsets differ between games.

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "../common/lua_api.h"

namespace twdll {

#pragma pack(push, 1)

// EMPIRECAMPAIGN::FACTION — gold/treasury at +0x6C4 (verified vs faction:treasury()).
struct TW_Faction {
    char pad_00[0x6C4];
    int  gold; // 0x6C4
};

// EMPIRECAMPAIGN::UNIT — men / AP layout matches Attila + legacy Rome 2 mapping.
// Live-verify against vanilla unit strength / AP accessors before treating as stable.
struct TW_Unit {
    char pad_00[0x44];
    int  num_men;       // 0x44
    int  max_num_men;   // 0x48
    char pad_4C[0x18];
    int  action_points; // 0x64
};

// EMPIRECAMPAIGN::CHARACTER — AP at +0x14; ambition/gravitas legacy Rome 2 mapping.
struct TW_Character {
    char pad_00[0x14];
    int  action_points; // 0x14
    char pad_18[0x540];
    int  ambition;      // 0x558
    int  gravitas;      // 0x55C
};

// EMPIRECAMPAIGN::MILITARY_FORCE — recruitment queue size at +0x45C (legacy Rome 2).
struct TW_MilitaryForce {
    char pad_00[0x45C];
    int  recruitment_queue_size; // 0x45C
};

// EMPIRECAMPAIGN::REGION — address-only for now (no verified field offsets yet).
struct TW_Region {
    char pad_00[0x4];
};

// EMPIRECAMPAIGN::REGION_SLOT — address-only for now (no verified field offsets yet).
struct TW_RegionSlot {
    char pad_00[0x4];
};

#pragma pack(pop)

#define TW_ASSERT_OFFSET(S, F, O) \
    static_assert(offsetof(S, F) == (O), #S " Rome2: " #F " expected at " #O)

TW_ASSERT_OFFSET(TW_Faction, gold, 0x6C4);
TW_ASSERT_OFFSET(TW_Unit, num_men, 0x44);
TW_ASSERT_OFFSET(TW_Unit, max_num_men, 0x48);
TW_ASSERT_OFFSET(TW_Unit, action_points, 0x64);
TW_ASSERT_OFFSET(TW_Character, action_points, 0x14);
TW_ASSERT_OFFSET(TW_Character, ambition, 0x558);
TW_ASSERT_OFFSET(TW_Character, gravitas, 0x55C);
TW_ASSERT_OFFSET(TW_MilitaryForce, recruitment_queue_size, 0x45C);

// Per-type pointer offset inside the Lua userdata wrapper.
template <typename T>
struct TW_PtrOffset {
    static_assert(sizeof(T) == 0,
                  "TW_PtrOffset not specialized — add TW_PTR_OFFSET(T, offset) in tw_types.h");
};

#define TW_PTR_OFFSET(T, O) \
    template <>             \
    struct TW_PtrOffset<T> { static constexpr size_t value = (O); }

TW_PTR_OFFSET(TW_Faction, 0x8);
TW_PTR_OFFSET(TW_Unit, 0x8);
TW_PTR_OFFSET(TW_Character, 0x8);
TW_PTR_OFFSET(TW_MilitaryForce, 0x8);
TW_PTR_OFFSET(TW_Region, 0x8);
TW_PTR_OFFSET(TW_RegionSlot, 0x8);

template <typename T>
T* tw_unwrap(lua_State* L, int slot) {
    void** ud = static_cast<void**>(l_touserdata(L, slot));
    if (!ud || !*ud) return nullptr;
    return *reinterpret_cast<T**>(static_cast<char*>(*ud) + TW_PtrOffset<T>::value);
}

} // namespace twdll
