-- Rome 2 smoke suite.
-- Load-time: core + metatable probe (game_interface may still be nil).
-- UICreated: faction gold path (needs campaign model / game_interface).

local function log(msg)
    if type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.Log) == "function" then
        twdll.core.Log(msg)
    else
        local f = io.open("twdll.log", "a")
        if f then
            f:write(tostring(msg) .. "\n")
            f:close()
        end
    end
end

local passed = 0
local failed = 0

local function assert_true(name, condition)
    if condition then
        passed = passed + 1
        log(string.format("[TEST]   assert '%s': OK", name))
        return true
    end
    failed = failed + 1
    log(string.format("[TEST]   assert '%s': FAILED", name))
    return false
end

local function summarize(phase)
    log(string.format("[TEST] %s PASSED: %d FAILED: %d SKIPPED: 0", phase, passed, failed))
end

log("[TEST] Rome 2 smoke (load)")
assert_true("twdll table", type(twdll) == "table")
assert_true("twdll.core table", type(twdll) == "table" and type(twdll.core) == "table")
assert_true("twdll.core.Log", type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.Log) == "function")
assert_true("twdll.core.GameBuild", type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.GameBuild) == "function")
assert_true("twdll.core.GetBuildSha", type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.GetBuildSha) == "function")

local game_build = nil
if type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.GameBuild) == "function" then
    game_build = twdll.core.GameBuild()
end
assert_true("GameBuild == Rome2", game_build == "Rome2")
log("[TEST] GameBuild=" .. tostring(game_build))

local sha = nil
if type(twdll) == "table" and type(twdll.core) == "table" and type(twdll.core.GetBuildSha) == "function" then
    sha = twdll.core.GetBuildSha()
end
assert_true("GetBuildSha non-empty", type(sha) == "string" and #sha > 0)
log("[TEST] GetBuildSha=" .. tostring(sha))

local multi_ok = pcall(function()
    twdll.core.Log("[TEST] multi-arg Log", 42, true, nil)
end)
assert_true("multi-arg Log", multi_ok)

assert_true("ProbeScriptInterfaces", type(twdll.core.ProbeScriptInterfaces) == "function")
local hits = nil
local probe_ok, probe_err = pcall(function()
    hits = twdll.core.ProbeScriptInterfaces()
end)
assert_true("ProbeScriptInterfaces call", probe_ok)
if not probe_ok then
    log("[TEST] ProbeScriptInterfaces error: " .. tostring(probe_err))
end
assert_true("ProbeScriptInterfaces table", type(hits) == "table")
if type(hits) == "table" then
    assert_true("FACTION_SCRIPT_INTERFACE", hits.FACTION_SCRIPT_INTERFACE == true)
    assert_true("CHARACTER_SCRIPT_INTERFACE", hits.CHARACTER_SCRIPT_INTERFACE == true)
    assert_true("UNIT_SCRIPT_INTERFACE", hits.UNIT_SCRIPT_INTERFACE == true)
    assert_true("MILITARY_FORCE_SCRIPT_INTERFACE", hits.MILITARY_FORCE_SCRIPT_INTERFACE == true)
end

summarize("LOAD")

-- Rome 2 exposes the campaign script interface on EpisodicScripting.game_interface,
-- not as a global `game_interface` (that name is Attila/WH-style).
local function get_game_interface()
    local scripting = rawget(_G, "EpisodicScripting")
    if type(scripting) ~= "table" then
        local ok_req, mod = pcall(require, "lua_scripts.EpisodicScripting")
        if ok_req then scripting = mod end
    end
    if type(scripting) == "table" and scripting.game_interface then
        return scripting.game_interface
    end
    return rawget(_G, "game_interface")
end

local function run_faction_tests()
    log("[TEST] Rome 2 smoke (UICreated / faction)")

    local faction = nil
    local ok_faction, err_faction = pcall(function()
        local gi = get_game_interface()
        if not gi then
            error("game_interface is nil")
        end
        faction = gi:model():world():faction_by_key("rom_rome")
    end)
    assert_true("resolve rom_rome faction", ok_faction and faction ~= nil)
    if not ok_faction then
        log("[TEST] faction resolve error: " .. tostring(err_faction))
        summarize("FACTION")
        return
    end

    local addr = faction:GetMemoryAddress()
    assert_true("GetMemoryAddress string", type(addr) == "string" and addr:match("^0x"))
    log("[TEST] faction addr=" .. tostring(addr))

    local gold = faction:GetGold()
    assert_true("GetGold number", type(gold) == "number")
    log("[TEST] GetGold=" .. tostring(gold))

    local vanilla = nil
    pcall(function()
        if type(faction.treasury) == "function" then
            vanilla = faction:treasury()
        end
    end)
    if vanilla ~= nil then
        log("[TEST] vanilla treasury()=" .. tostring(vanilla))
        assert_true("GetGold matches treasury()", gold == vanilla)
    else
        log("[TEST] vanilla treasury() unavailable — skipped equality check")
    end

    local original = gold
    local set_ok = faction:SetGold(original + 7)
    assert_true("SetGold returns true", set_ok == true)
    local after = faction:GetGold()
    assert_true("SetGold wrote value", after == original + 7)
    faction:SetGold(original)
    assert_true("SetGold restored", faction:GetGold() == original)

    summarize("FACTION")
    return faction
end

local function run_unit_tests(faction)
    log("[TEST] Rome 2 smoke (UICreated / unit)")
    if not faction then
        log("[TEST] unit suite skipped — no faction")
        summarize("UNIT")
        return
    end

    local unit = nil
    local ok_unit, err_unit = pcall(function()
        local forces = faction:military_force_list()
        if not forces or forces:num_items() <= 0 then
            error("military_force_list empty")
        end
        local force = forces:item_at(0)
        local units = force:unit_list()
        if not units or units:num_items() <= 0 then
            error("unit_list empty")
        end
        unit = units:item_at(0)
    end)
    if not ok_unit then
        log("[TEST] unit resolve error: " .. tostring(err_unit))
    end
    assert_true("resolve unit", ok_unit and unit ~= nil)
    if not unit then
        summarize("UNIT")
        return
    end

    local addr = unit:GetMemoryAddress()
    assert_true("unit GetMemoryAddress string", type(addr) == "string" and addr:match("^0x"))
    log("[TEST] unit addr=" .. tostring(addr))

    local men = unit:GetNumMen()
    local max_men = unit:GetMaxNumMen()
    local ap = unit:GetActionPoints()
    assert_true("GetNumMen number", type(men) == "number")
    assert_true("GetMaxNumMen number", type(max_men) == "number")
    assert_true("GetActionPoints number", type(ap) == "number")
    assert_true("men <= max_men", men <= max_men)
    log(string.format("[TEST] men=%s max=%s ap=%s", tostring(men), tostring(max_men), tostring(ap)))

    local vanilla_men = nil
    pcall(function()
        if type(unit.number_of_men_in_unit) == "function" then
            vanilla_men = unit:number_of_men_in_unit()
        elseif type(unit.strength) == "function" then
            -- strength is often a percentage; prefer number_of_men_in_unit when present
        end
    end)
    if vanilla_men ~= nil then
        log("[TEST] vanilla number_of_men_in_unit()=" .. tostring(vanilla_men))
        assert_true("GetNumMen matches vanilla", men == vanilla_men)
    else
        log("[TEST] vanilla men accessor unavailable — skipped equality check")
    end

    local original_men = men
    local set_men_ok = unit:SetNumMen(math.max(1, original_men - 1))
    assert_true("SetNumMen returns true", set_men_ok == true)
    assert_true("SetNumMen wrote value", unit:GetNumMen() == math.max(1, original_men - 1))
    unit:SetNumMen(original_men)
    assert_true("SetNumMen restored", unit:GetNumMen() == original_men)

    local original_ap = ap
    local set_ap_ok = unit:SetActionPoints(original_ap + 5)
    assert_true("SetActionPoints returns true", set_ap_ok == true)
    assert_true("SetActionPoints wrote value", unit:GetActionPoints() == original_ap + 5)
    unit:SetActionPoints(original_ap)
    assert_true("SetActionPoints restored", unit:GetActionPoints() == original_ap)

    summarize("UNIT")
end

local function run_character_tests(faction)
    log("[TEST] Rome 2 smoke (UICreated / character)")
    if not faction then
        log("[TEST] character suite skipped — no faction")
        summarize("CHARACTER")
        return
    end

    local character = nil
    local ok_char, err_char = pcall(function()
        local chars = faction:character_list()
        if not chars or chars:num_items() <= 0 then
            error("character_list empty")
        end
        character = chars:item_at(0)
    end)
    if not ok_char then
        log("[TEST] character resolve error: " .. tostring(err_char))
    end
    assert_true("resolve character", ok_char and character ~= nil)
    if not character then
        summarize("CHARACTER")
        return
    end

    local addr = character:GetMemoryAddress()
    assert_true("character GetMemoryAddress string", type(addr) == "string" and addr:match("^0x"))
    log("[TEST] character addr=" .. tostring(addr))

    local ap = character:GetActionPoints()
    local ambition = character:GetAmbition()
    local gravitas = character:GetGravitas()
    assert_true("GetActionPoints number", type(ap) == "number")
    assert_true("GetAmbition number", type(ambition) == "number")
    assert_true("GetGravitas number", type(gravitas) == "number")
    log(string.format("[TEST] ap=%s ambition=%s gravitas=%s",
        tostring(ap), tostring(ambition), tostring(gravitas)))

    local original_ap = ap
    local set_ap_ok = character:SetActionPoints(original_ap + 5)
    assert_true("SetActionPoints returns true", set_ap_ok == true)
    assert_true("SetActionPoints wrote value", character:GetActionPoints() == original_ap + 5)
    character:SetActionPoints(original_ap)
    assert_true("SetActionPoints restored", character:GetActionPoints() == original_ap)

    local original_ambition = ambition
    local set_amb_ok = character:SetAmbition(original_ambition + 3)
    assert_true("SetAmbition returns true", set_amb_ok == true)
    assert_true("SetAmbition wrote value", character:GetAmbition() == original_ambition + 3)
    character:SetAmbition(original_ambition)
    assert_true("SetAmbition restored", character:GetAmbition() == original_ambition)

    local original_gravitas = gravitas
    local set_grav_ok = character:SetGravitas(original_gravitas + 3)
    assert_true("SetGravitas returns true", set_grav_ok == true)
    assert_true("SetGravitas wrote value", character:GetGravitas() == original_gravitas + 3)
    character:SetGravitas(original_gravitas)
    assert_true("SetGravitas restored", character:GetGravitas() == original_gravitas)

    summarize("CHARACTER")
end

local function run_military_force_tests(faction)
    log("[TEST] Rome 2 smoke (UICreated / military_force)")
    if not faction then
        log("[TEST] military_force suite skipped — no faction")
        summarize("MILFORCE")
        return
    end

    local force = nil
    local ok_force, err_force = pcall(function()
        local forces = faction:military_force_list()
        if not forces or forces:num_items() <= 0 then
            error("military_force_list empty")
        end
        force = forces:item_at(0)
    end)
    if not ok_force then
        log("[TEST] military_force resolve error: " .. tostring(err_force))
    end
    assert_true("resolve military_force", ok_force and force ~= nil)
    if not force then
        summarize("MILFORCE")
        return
    end

    local addr = force:GetMemoryAddress()
    assert_true("force GetMemoryAddress string", type(addr) == "string" and addr:match("^0x"))
    log("[TEST] force addr=" .. tostring(addr))

    local queue = force:GetRecruitmentQueueSize()
    assert_true("GetRecruitmentQueueSize number", type(queue) == "number")
    assert_true("GetRecruitmentQueueSize non-negative", queue >= 0)
    log("[TEST] recruitment_queue_size=" .. tostring(queue))

    local original = queue
    local set_ok = force:SetRecruitmentQueueSize(original + 1)
    assert_true("SetRecruitmentQueueSize returns true", set_ok == true)
    assert_true("SetRecruitmentQueueSize wrote value", force:GetRecruitmentQueueSize() == original + 1)
    force:SetRecruitmentQueueSize(original)
    assert_true("SetRecruitmentQueueSize restored", force:GetRecruitmentQueueSize() == original)

    summarize("MILFORCE")
end

local function run_region_tests(faction)
    log("[TEST] Rome 2 smoke (UICreated / region)")
    if not faction then
        log("[TEST] region suite skipped — no faction")
        summarize("REGION")
        return
    end

    local region = nil
    local ok_region, err_region = pcall(function()
        local regions = faction:region_list()
        if not regions or regions:num_items() <= 0 then
            error("region_list empty")
        end
        region = regions:item_at(0)
    end)
    if not ok_region then
        log("[TEST] region resolve error: " .. tostring(err_region))
    end
    assert_true("resolve region", ok_region and region ~= nil)
    if not region then
        summarize("REGION")
        return
    end

    local addr = region:GetMemoryAddress()
    assert_true("region GetMemoryAddress string", type(addr) == "string" and addr:match("^0x"))
    log("[TEST] region addr=" .. tostring(addr))

    pcall(function()
        if type(region.name) == "function" then
            log("[TEST] region name=" .. tostring(region:name()))
        end
    end)
    assert_true("region DumpInts 0x0", region:DumpInts(0x0, 48) == true)
    assert_true("region DumpInts 0xC0", region:DumpInts(0xC0, 48) == true)
    assert_true("region DumpInts 0x140", region:DumpInts(0x140, 48) == true)
    assert_true("region GetIntAtOffset", type(region:GetIntAtOffset(0x0)) == "number")

    summarize("REGION")
end

local function run_slot_tests(faction)
    log("[TEST] Rome 2 smoke (UICreated / slot)")
    if not faction then
        log("[TEST] slot suite skipped — no faction")
        summarize("SLOT")
        return
    end

    local slots = nil
    local ok_slots, err_slots = pcall(function()
        local regions = faction:region_list()
        if not regions or regions:num_items() <= 0 then
            error("region_list empty")
        end
        local region = regions:item_at(0)
        local settlement = region:settlement()
        if not settlement then
            error("settlement nil")
        end
        slots = settlement:slot_list()
        if not slots or slots:num_items() <= 0 then
            error("slot_list empty")
        end
    end)
    if not ok_slots then
        log("[TEST] slot resolve error: " .. tostring(err_slots))
    end
    assert_true("resolve slot_list", ok_slots and slots ~= nil)
    if not slots then
        summarize("SLOT")
        return
    end

    local slot = slots:item_at(0)
    assert_true("resolve slot", slot ~= nil)
    if not slot then
        summarize("SLOT")
        return
    end

    local addr = slot:GetMemoryAddress()
    assert_true("slot GetMemoryAddress string", type(addr) == "string" and addr:match("^0x"))
    log("[TEST] slot addr=" .. tostring(addr))

    pcall(function()
        if type(slot.name) == "function" then
            log("[TEST] slot[0] name=" .. tostring(slot:name()))
        end
    end)
    assert_true("slot DumpInts", slot:DumpInts(0x0, 48) == true)
    assert_true("slot GetIntAtOffset", type(slot:GetIntAtOffset(0x0)) == "number")

    -- Compare a second slot when available (primary vs secondary layout).
    if slots:num_items() > 1 then
        local slot2 = slots:item_at(1)
        assert_true("resolve slot[1]", slot2 ~= nil)
        if slot2 then
            pcall(function()
                if type(slot2.name) == "function" then
                    log("[TEST] slot[1] name=" .. tostring(slot2:name()))
                end
            end)
            log(string.format("[TEST] slot[0] +0xA0=%s +0x4=%s +0x9C=%s",
                tostring(slot:GetIntAtOffset(0xA0)),
                tostring(slot:GetIntAtOffset(0x4)),
                tostring(slot:GetIntAtOffset(0x9C))))
            log(string.format("[TEST] slot[1] +0xA0=%s +0x4=%s +0x9C=%s",
                tostring(slot2:GetIntAtOffset(0xA0)),
                tostring(slot2:GetIntAtOffset(0x4)),
                tostring(slot2:GetIntAtOffset(0x9C))))
            assert_true("slot[1] DumpInts", slot2:DumpInts(0x0, 48) == true)
        end
    else
        log("[TEST] only one slot — skipped slot[1] compare")
    end

    -- Probe +0xA0 as rotation candidate (0..5): write next step, verify, restore.
    local original_a0 = slot:GetIntAtOffset(0xA0)
    assert_true("slot +0xA0 number", type(original_a0) == "number")
    local probe_a0 = (original_a0 + 1) % 6
    slot:SetIntAtOffset(0xA0, probe_a0)
    local after_a0 = slot:GetIntAtOffset(0xA0)
    assert_true("slot SetIntAtOffset(+0xA0) wrote", after_a0 == probe_a0)
    log(string.format("[TEST] slot +0xA0 probe original=%s wrote=%s readback=%s",
        tostring(original_a0), tostring(probe_a0), tostring(after_a0)))
    slot:SetIntAtOffset(0xA0, original_a0)
    assert_true("slot +0xA0 restored", slot:GetIntAtOffset(0xA0) == original_a0)

    summarize("SLOT")
end

local function run_world_tests()
    log("[TEST] Rome 2 smoke (UICreated / world)")
    assert_true("twdll.world table", type(twdll) == "table" and type(twdll.world) == "table")
    assert_true("twdll.world.GetMemoryAddress", type(twdll.world.GetMemoryAddress) == "function")
    assert_true("twdll.world.GetFactionCount", type(twdll.world.GetFactionCount) == "function")
    assert_true("twdll.world.Capture", type(twdll.world.Capture) == "function")

    local addr = twdll.world.GetMemoryAddress()
    if type(addr) ~= "string" then
        log("[TEST] world addr nil after ctor hook — trying Capture from vanilla world()")
        local ok_cap, err_cap = pcall(function()
            local gi = get_game_interface()
            if not gi then error("game_interface nil") end
            local world = gi:model():world()
            local captured = twdll.world.Capture(world)
            if not captured then error("Capture returned false") end
        end)
        if not ok_cap then
            log("[TEST] world Capture error: " .. tostring(err_cap))
        end
        addr = twdll.world.GetMemoryAddress()
    end

    assert_true("world GetMemoryAddress string", type(addr) == "string" and addr:match("^0x"))
    log("[TEST] world addr=" .. tostring(addr))

    local count = twdll.world.GetFactionCount()
    assert_true("GetFactionCount number", type(count) == "number")
    assert_true("GetFactionCount positive", type(count) == "number" and count > 0)
    log("[TEST] faction_count=" .. tostring(count))

    local vanilla_count = nil
    local ok_van, err_van = pcall(function()
        local gi = get_game_interface()
        if not gi then error("game_interface nil") end
        vanilla_count = gi:model():world():faction_list():num_items()
    end)
    if not ok_van then
        log("[TEST] vanilla faction_list error: " .. tostring(err_van))
    end
    if vanilla_count ~= nil then
        log("[TEST] vanilla faction_list:num_items()=" .. tostring(vanilla_count))
        assert_true("GetFactionCount matches vanilla", count == vanilla_count)
    else
        log("[TEST] vanilla faction_list unavailable — skipped equality check")
    end

    summarize("WORLD")
end

local function run_campaign_ui_tests()
    log("[TEST] Rome 2 smoke (UICreated / campaign_ui)")
    assert_true("twdll.campaign_ui table", type(twdll) == "table" and type(twdll.campaign_ui) == "table")
    assert_true("twdll.campaign_ui.GetMemoryAddress", type(twdll.campaign_ui.GetMemoryAddress) == "function")

    local addr = twdll.campaign_ui.GetMemoryAddress()
    if type(addr) ~= "string" then
        log("[TEST] campaign_ui addr nil — ctor likely already ran before hook install")
    end
    assert_true("campaign_ui GetMemoryAddress string", type(addr) == "string" and addr:match("^0x"))
    log("[TEST] campaign_ui addr=" .. tostring(addr))

    summarize("CAMPAIGN_UI")
end

local function run_uicreated_suites()
    local faction = run_faction_tests()
    run_unit_tests(faction)
    run_character_tests(faction)
    run_military_force_tests(faction)
    run_region_tests(faction)
    run_slot_tests(faction)
    run_world_tests()
    run_campaign_ui_tests()
end

local scripting = rawget(_G, "EpisodicScripting")
if type(scripting) ~= "table" then
    local ok_req, mod = pcall(require, "lua_scripts.EpisodicScripting")
    if ok_req then scripting = mod end
end

if type(scripting) == "table" and type(scripting.AddEventCallBack) == "function" then
    scripting.AddEventCallBack("UICreated", function()
        local ok, err = pcall(run_uicreated_suites)
        if not ok then
            log("[TEST] UICreated suite error: " .. tostring(err))
            failed = failed + 1
            summarize("UICREATED")
        end
    end)
    log("[TEST] UICreated suites deferred (faction/unit/character/force/region/slot/world/campaign_ui)")
else
    log("[TEST] AddEventCallBack unavailable — running suites immediately")
    run_uicreated_suites()
end
