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

local function run_faction_tests()
    log("[TEST] Rome 2 smoke (UICreated / faction)")

    local scripting = rawget(_G, "EpisodicScripting")
    if type(scripting) ~= "table" then
        local ok_req, mod = pcall(require, "lua_scripts.EpisodicScripting")
        if ok_req then scripting = mod end
    end

    local faction = nil
    local ok_faction, err_faction = pcall(function()
        local gi = scripting and scripting.game_interface
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

local function run_uicreated_suites()
    local faction = run_faction_tests()
    run_unit_tests(faction)
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
    log("[TEST] faction/unit suites deferred to UICreated")
else
    log("[TEST] AddEventCallBack unavailable — running suites immediately")
    run_uicreated_suites()
end
