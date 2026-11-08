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
end

local scripting = rawget(_G, "EpisodicScripting")
if type(scripting) ~= "table" then
    local ok_req, mod = pcall(require, "lua_scripts.EpisodicScripting")
    if ok_req then scripting = mod end
end

if type(scripting) == "table" and type(scripting.AddEventCallBack) == "function" then
    scripting.AddEventCallBack("UICreated", function()
        local ok, err = pcall(run_faction_tests)
        if not ok then
            log("[TEST] faction suite error: " .. tostring(err))
            failed = failed + 1
            summarize("FACTION")
        end
    end)
    log("[TEST] faction suite deferred to UICreated")
else
    log("[TEST] AddEventCallBack unavailable — running faction suite immediately")
    run_faction_tests()
end
