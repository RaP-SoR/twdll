#pragma once
// tw_types.h — Engine object layouts for Total War: Rome 2 (32-bit).
// Do NOT share with attila/ — offsets differ between games.

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "../common/lua_api.h"

namespace twdll {

#pragma pack(push, 1)

// EMPIRECAMPAIGN::FACTION — gold/treasury candidate at +0x6C4 (legacy Rome 2 mapping).
// Verified in-game against faction:treasury() before treating as stable.
struct TW_Faction {
    char pad_00[0x6C4];
    int  gold; // 0x6C4
};

#pragma pack(pop)

#define TW_ASSERT_OFFSET(S, F, O) \
    static_assert(offsetof(S, F) == (O), #S " Rome2: " #F " expected at " #O)

TW_ASSERT_OFFSET(TW_Faction, gold, 0x6C4);

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

template <typename T>
T* tw_unwrap(lua_State* L, int slot) {
    void** ud = static_cast<void**>(l_touserdata(L, slot));
    if (!ud || !*ud) return nullptr;
    return *reinterpret_cast<T**>(static_cast<char*>(*ud) + TW_PtrOffset<T>::value);
}

} // namespace twdll
